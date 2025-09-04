#include <defs.h>
#include <string.h>

#include "alloc.h"
#include "tm.h"

#define LOCK_TABLE_SIZE (1 << 13)
#define LOCK_MASK       (LOCK_TABLE_SIZE - 1)
#define LOCK_SHIFT      (2)
#define LOCK_BIT        (1)

#define LOCK_ADDR(a)    (lock_table + (((lock_t)(a) >> LOCK_SHIFT) & LOCK_MASK))
#define LOCK_LOAD(l)    (*(l))
#define LOCK_VERSION(l) (l >> LOCK_BITS)

#define INIT_SEED       (123456789UL)
#define MIN_BACKOFF     (1UL << 1)
#define MAX_BACKOFF     (1UL << 20)

typedef struct
{
    uint64_t padding;
    r_entry_t mram_r_set[R_SET_SIZE];
    w_entry_t mram_w_set[W_SET_SIZE];
} r_w_set_t;

volatile clock_t clock;
volatile lock_t lock_table[LOCK_TABLE_SIZE];

tx_t tx_array[NR_TASKLETS];
__mram r_w_set_t r_w_set[NR_TASKLETS];

__host uint32_t aborts[NR_TASKLETS];

tx_t *
tm_get_descriptor()
{
    return &tx_array[me()];
}

/* #############################################################################
 * ATOMIC FUNCTIONS
 * #############################################################################
 */

clock_t
atomic_clock_read()
{
    clock_t result;

    __asm__ __volatile__("acquire %[p], 0, nz, ." : : [p] "r"(&clock) :);

    result = clock;

    __asm__ __volatile__("release %[p], 0, nz, .+1" : : [p] "r"(&clock) :);

    return result;
}

clock_t
atomic_clock_increment()
{
    clock_t result;

    __asm__ __volatile__("acquire %[p], 0, nz, ." : : [p] "r"(&clock) :);

    clock += 2;
    result = clock;

    __asm__ __volatile__("release %[p], 0, nz, .+1" : : [p] "r"(&clock) :);

    return result;
}

lock_t
cas(volatile lock_t *ptr, lock_t old_val, lock_t new_val)
{
    lock_t val;

    __asm__ __volatile__("acquire %[p], 0, nz, ." : : [p] "r"(ptr) :);

    val = *ptr;

    if (val == old_val)
    {
        *ptr = new_val;
    }

    __asm__ __volatile__("release %[p], 0, nz, .+1" : : [p] "r"(ptr) :);

    return val;
}

/* #############################################################################
 * HELPER FUNCTIONS
 * #############################################################################
 */

int
read_set_coherent(tx_t *tx)
{
    lock_t lock_v, ts;
    MRAM r_entry_t *r;
    MRAM w_entry_t *w;

    ts = tx->ts;

    ASSERT((ts & LOCK_BIT) == 0);

    r = tx->r_set.entries;
    for (uint16_t i = tx->r_set.n_entries; i > 0; i--, r++)
    {
        ASSERT(r->lock);

        lock_v = LOCK_LOAD(r->lock);

        if (lock_v & LOCK_BIT)
        {
            w = (MRAM w_entry_t *)(lock_v & ~LOCK_BIT);

            if (!(w >= tx->w_set.entries && w < tx->w_set.entries + tx->w_set.n_entries))
            {
                return 0;
            }
            else
            {
                if (w->version > ts)
                {
                    return 0;
                }
            }
        }
        else
        {
            if (lock_v > ts)
            {
                return 0;
            }
        }
    }

    return 1;
}

void
drop_locks_after_commit(tx_t *tx, clock_t ts)
{
    MRAM w_entry_t *w;

    w = tx->w_set.entries;
    for (uint16_t i = tx->w_set.n_entries; i > 0; i--, w++)
    {
        if (w->lock == NULL)
        {
            continue;
        }

        ASSERT(w->addr != NULL);
        ASSERT(w->lock != NULL);
        ASSERT(*(w->lock) & LOCK_BIT);
        ASSERT(
            ((MRAM w_entry_t *)(*(w->lock) & ~LOCK_BIT)) >= tx->w_set.entries &&
            ((MRAM w_entry_t *)(*(w->lock) & ~LOCK_BIT)) < tx->w_set.entries + tx->w_set.n_entries);
        ASSERT((ts & LOCK_BIT) == 0);

        *(w->lock) = ts;
    }
}

void
reverse_undo_writes(tx_t *tx)
{
    MRAM w_entry_t *w;

    w = &tx->w_set.entries[tx->w_set.n_entries - 1];
    for (uint16_t i = tx->w_set.n_entries; i > 0; i--, w--)
    {
        ASSERT(w->addr != NULL);

        *(w->addr) = w->value;
    }
}

void
drop_locks_after_abort(tx_t *tx)
{
    MRAM w_entry_t *w;

    w = tx->w_set.entries;
    for (uint16_t i = tx->w_set.n_entries; i > 0; i--, w++)
    {
        if (w->lock == NULL)
        {
            continue;
        }

        ASSERT(w->addr != NULL);
        ASSERT(w->lock != NULL);
        ASSERT((w->version & LOCK_BIT) == 0);
        ASSERT(*(w->lock) & LOCK_BIT);
        ASSERT(
            ((MRAM w_entry_t *)(*(w->lock) & ~LOCK_BIT)) >= tx->w_set.entries &&
            ((MRAM w_entry_t *)(*(w->lock) & ~LOCK_BIT)) < tx->w_set.entries + tx->w_set.n_entries);
        ASSERT((w->version & LOCK_BIT) == 0);

        *(w->lock) = w->version;
    }
}

int
cm_abort(tx_t *self, tx_t *other)
{
    if (self->mode == OPTIMISTIC)
    {
        return 1;
    }

    if (self->c_ts > other->c_ts)
    {
        return 1;
    }

    other->aborted = 1;

    return 0;
}

void
backoff(tx_t *tx, uint32_t retries)
{
    (void)retries;

    volatile uint64_t i = 0;
    uint64_t stall;

    // stall = tx->ts & 0xF;
    // stall += retries >> 2;
    // stall *= 300;

    tx->seed ^= (tx->seed << 17);
    tx->seed ^= (tx->seed >> 13);
    tx->seed ^= (tx->seed << 5);

    stall = tx->seed % tx->backoff;

    while (i++ < stall)
    {
    }

    if (tx->backoff < MAX_BACKOFF)
    {
        tx->backoff <<= 1;
    }
}

/* #############################################################################
 * COMMON TM FUNCTIONS
 * #############################################################################
 */

void
tm_init()
{
    clock = 0;

    memset((void *)lock_table, 0, sizeof(lock_table));
    memset((void *)aborts, 0, sizeof(aborts));

    for (int i = 0; i < NR_TASKLETS; ++i)
    {
        tx_array[i].r_set.entries = r_w_set[i].mram_r_set;
        ASSERT(tx_array[i].r_set.entries != NULL);

        tx_array[i].w_set.entries = r_w_set[i].mram_w_set;
        ASSERT(tx_array[i].w_set.entries != NULL);
    }

    for (int i = 0; i < NR_TASKLETS; ++i)
    {
        tx_array[i].seed    = INIT_SEED * (i + 1);
        tx_array[i].backoff = MIN_BACKOFF;
    }
}

void
tm_start(tx_t *tx, clock_t c_ts, uint8_t type, uint8_t mode)
{
    tx->r_set.n_entries = 0;
    tx->w_set.n_entries = 0;

    tx->ts = atomic_clock_read();
    ASSERT((tx->ts & LOCK_BIT) == 0);

    tx->c_ts = c_ts * 2; // In order for lowest bit to be 0
    ASSERT((tx->c_ts & LOCK_BIT) == 0);

    tx->status  = TX_ACTIVE;
    tx->type    = type;
    tx->mode    = mode;
    tx->aborted = 0;
}

val_t
tm_load(tx_t *tx, volatile MRAM val_t *addr)
{
    volatile lock_t *lock;
    lock_t lock_v, version;
    val_t value;
    MRAM r_entry_t *r;
    MRAM w_entry_t *w;

    ASSERT(tx->mode == DETERMINISTIC || tx->status == TX_ACTIVE);

    lock = LOCK_ADDR(addr);

    do
    {
        lock_v  = LOCK_LOAD(lock);
        version = lock_v & ~LOCK_BIT;
        value   = *addr;

        if (tx->aborted)
        {
            ASSERT(tx->mode == DETERMINISTIC);

            tm_abort(tx);
            return 0;
        }

        if (LOCK_LOAD(lock) == version && tx->ts >= version)
        {
            // Not locked
            assert(tx->r_set.n_entries < R_SET_SIZE);

            ASSERT(addr != NULL);
            ASSERT(lock != NULL);

            r       = &tx->r_set.entries[tx->r_set.n_entries++];
            r->lock = lock;

            return value;
        }

        w = (MRAM w_entry_t *)version;

        if (lock_v & LOCK_BIT && w >= tx->w_set.entries && w < tx->w_set.entries + tx->w_set.n_entries)
        {
            // We onw the lock for addr
            return value;
        }

    } while (lock_v & LOCK_BIT && !cm_abort(tx, w->tx));

    tm_abort(tx);
    return 0;
}

void
tm_store(tx_t *tx, volatile MRAM val_t *addr, val_t value)
{

    volatile lock_t *lock;
    lock_t lock_v, version;
    MRAM w_entry_t *w;
    MRAM w_entry_t *prev;

    lock   = LOCK_ADDR(addr);
    lock_v = LOCK_LOAD(lock);

    w = (MRAM w_entry_t *)(lock_v & ~LOCK_BIT);

    if (lock_v & LOCK_BIT && w >= tx->w_set.entries && w < tx->w_set.entries + tx->w_set.n_entries)
    {
        // We onw the lock for addr
        version = w->version;

        assert(tx->w_set.n_entries < W_SET_SIZE);
        w = &tx->w_set.entries[tx->w_set.n_entries];

        // We already have this lock. Don't drop it twice
        lock = NULL;
    }
    else
    {
        assert(tx->w_set.n_entries < W_SET_SIZE);
        w     = &tx->w_set.entries[tx->w_set.n_entries];
        w->tx = tx;

        // We do not own this lock. Try to acquire it.
        do
        {
            lock_v = LOCK_LOAD(lock);
            prev   = (MRAM w_entry_t *)(lock_v & ~LOCK_BIT);

            if (tx->aborted)
            {
                ASSERT(tx->mode == DETERMINISTIC);

                tm_abort(tx);
                return;
            }

            if (lock_v & LOCK_BIT)
            {
                if (cm_abort(tx, prev->tx))
                {
                    tm_abort(tx);
                    return;
                }
                else
                {
                    continue;
                }
            }

            if (cas(lock, lock_v, ((lock_t)w | LOCK_BIT)) == lock_v)
            {
                version = lock_v & ~LOCK_BIT; // Not needed. For redability
                break;
            }
        } while (1);

        ASSERT(lock != NULL);
    }

    ASSERT(addr != NULL);
    ASSERT(tx != NULL);
    ASSERT((version & LOCK_BIT) == 0);

    w->addr    = addr;
    w->lock    = lock;
    w->version = version;
    w->value   = *addr;

    *addr = value;

    tx->w_set.n_entries++;

    if (version > tx->ts)
    {
        tm_abort(tx);
    }
}

void
tm_commit(tx_t *tx)
{
    clock_t ts;

    if (tx->mode == DETERMINISTIC)
    {
        while (atomic_clock_read() < tx->c_ts)
        {
            if (tx->aborted)
            {
                tm_abort(tx);
                return;
            }
        }
    }

    ASSERT(tx->status == TX_ACTIVE);

    ts = atomic_clock_increment();

    if (tx->w_set.n_entries == 0)
    {
        tx->status = TX_COMMITTED;
        return;
    }

    if (tx->ts != ts - 2 && !read_set_coherent(tx))
    {
        tm_abort(tx);
        return;
    }

    drop_locks_after_commit(tx, ts);

    mram_malloc_commit();

    aborts[me()] += (uint32_t)tx->retries;

    tx->retries = 0;
    tx->backoff = MIN_BACKOFF;
    tx->status  = TX_COMMITTED;

    MEMBARSTLD();
}

void
tm_abort(tx_t *tx)
{
    reverse_undo_writes(tx);

    drop_locks_after_abort(tx);

    mram_malloc_revert();

    tx->status = TX_ABORTED;
    tx->retries++;

    backoff(tx, tx->retries);
}
