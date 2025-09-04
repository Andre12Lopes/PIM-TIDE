#pragma once

#include <stdint.h>

#include "../common/communication.h"
#include "util.h"

typedef uintptr_t clock_t;
typedef uintptr_t lock_t;
typedef uintptr_t word_t;
typedef uintptr_t val_t;

typedef struct r_entry
{
    volatile lock_t *lock;
} r_entry_t;

typedef struct r_set
{
    MRAM r_entry_t *entries;
    uint16_t n_entries;
} r_set_t;

typedef struct w_entry
{
    volatile MRAM val_t *addr;
    volatile lock_t *lock;
    struct tx *tx;
    lock_t version;
    val_t value;
} w_entry_t;

typedef struct w_set
{
    MRAM w_entry_t *entries;
    uint16_t n_entries;
} w_set_t;

typedef struct tx
{
    r_set_t r_set;
    w_set_t w_set;
    lock_t ts;
    lock_t c_ts;
    uint64_t seed;
    uint32_t backoff;
    uint32_t retries;
    uint8_t status;   // TX_ACTIVE or TX_COMMITTED or TX_ABORTED
    uint8_t type;   // PAYMENT or NEW_ORDER or ...
    uint8_t mode;   // DET or OPT
    uint8_t aborted;
} tx_t;

enum
{
    TX_ACTIVE    = 1,
    TX_COMMITTED = 2,
    TX_ABORTED   = 4,
};

tx_t *tm_get_descriptor();

clock_t atomic_clock_read();
clock_t atomic_clock_increment();
lock_t cas(volatile lock_t *ptr, lock_t old_val, lock_t new_val);

int read_set_coherent(tx_t *tx);
void drop_locks_after_commit(tx_t *tx, clock_t ts);
void reverse_undo_writes(tx_t *tx);
void drop_locks_after_abort(tx_t *tx);
int cm_abort(tx_t *self, tx_t *other);
void backoff(tx_t *tx, uint32_t retries);

void tm_init();
void tm_start(tx_t *tx, clock_t c_ts, uint8_t type, uint8_t mode);
val_t tm_load(tx_t *tx, volatile MRAM val_t *addr);
void tm_store(tx_t *tx, volatile MRAM val_t *addr, val_t value);
void tm_commit(tx_t *tx);
void tm_abort(tx_t *tx);

void opt_tm_store(tx_t *tx, volatile MRAM val_t *addr, val_t value);
void det_tm_store(tx_t *tx, volatile MRAM val_t *addr, val_t value);

// ############################################# MACROS #############################################

#define TM_INIT() tm_init();

#define TM_START(c_ts, t, m)                                                                            \
    do                                                                                                  \
    {                                                                                                   \
        tm_start(tm_get_descriptor(), c_ts, t, m);

#define TM_LOAD(addr)                                                                                   \
    tm_load(tm_get_descriptor(), (volatile MRAM val_t *)addr);                                          \
    if (tm_get_descriptor()->status & TX_ABORTED)                                                       \
    {                                                                                                   \
        continue;                                                                                       \
    }

#define TM_LOAD_F(addr)                                                                                 \
    tm_load(tm_get_descriptor(), (volatile MRAM val_t *)addr);                                          \
    if (tm_get_descriptor()->status & TX_ABORTED)                                                       \
    {                                                                                                   \
        return;                                                                                         \
    }

#define TM_STORE(addr, value)                                                                           \
    tm_store(tm_get_descriptor(), (volatile MRAM val_t *)addr, value);                                  \
    if (tm_get_descriptor()->status & TX_ABORTED)                                                       \
    {                                                                                                   \
        continue;                                                                                       \
    }

#define TM_STORE_F(addr, value)                                                                         \
    tm_store(tm_get_descriptor(), (volatile MRAM val_t *)addr, value);                                  \
    if (tm_get_descriptor()->status & TX_ABORTED)                                                       \
    {                                                                                                   \
        return;                                                                                         \
    }

#define AFTER_TM_FUNCTION()                                                                             \
    if (tm_get_descriptor()->status & TX_ABORTED)                                                       \
    {                                                                                                   \
        continue;                                                                                       \
    }

#define AFTER_TM_FUNCTION_LOOP()                                                                        \
    if (tm_get_descriptor()->status & TX_ABORTED)                                                       \
    {                                                                                                   \
        break;                                                                                          \
    }

#define AFTER_TM_FUNCTION_F()                                                                           \
    if (tm_get_descriptor()->status & TX_ABORTED)                                                       \
    {                                                                                                   \
        return;                                                                                         \
    }

#define TM_ABORT()                                                                                      \
    tm_abort(tm_get_descriptor());                                                                      \
    return;

#define TM_COMMIT()                                                                                     \
    tm_commit(tm_get_descriptor());                                                                     \
    if (tm_get_descriptor()->status & TX_COMMITTED)                                                     \
    {                                                                                                   \
        break;                                                                                          \
    }                                                                                                   \
    }                                                                                                   \
    while (1)
