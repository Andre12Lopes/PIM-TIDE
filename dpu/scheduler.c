#include <barrier.h>

#include "application.h"
#include "scheduler.h"

BARRIER_INIT(barrier, NR_TASKLETS);

__mram sub_transaction_batch_t input[NR_TASKLETS];

cache_t cache[NR_TASKLETS];

void
scheduler_init(uint32_t dpu_id)
{
    application_init(dpu_id);
}

void
scheduler_process_sub_transactions(int tid)
{
    __dma_aligned uint64_t n_det_sub_txs;
    __dma_aligned uint64_t n_opt_sub_txs;

    sub_transaction_t *sub_tx;
    uint32_t size;

    mram_read(&input[tid].n_det_sub_txs, &n_det_sub_txs, sizeof(n_det_sub_txs));
    mram_read(&input[tid].n_opt_sub_txs, &n_opt_sub_txs, sizeof(n_opt_sub_txs));

    // Processing deterministic sub transactions
    for (uint32_t i = 0; i < (uint32_t)n_det_sub_txs; i += CACHE_SIZE)
    {
        mram_read(&input[tid].sub_txs[i], cache[tid].sub_txs, CACHE_SIZE * sizeof(sub_transaction_t));

        size = (i + CACHE_SIZE) > n_det_sub_txs ? n_det_sub_txs - i : CACHE_SIZE;

        for (uint32_t j = 0; j < size; ++j)
        {
            sub_tx = &cache[tid].sub_txs[j];

            exec(sub_tx->params, sub_tx->type, sub_tx->mode, sub_tx->ts);
        }
    }

    barrier_wait(&barrier);

    // Processing optimistic sub transactions
    for (uint32_t i = BATCH_SIZE - n_opt_sub_txs; i < (uint32_t)BATCH_SIZE; i += CACHE_SIZE)
    {
        mram_read(&input[tid].sub_txs[i], cache[tid].sub_txs, CACHE_SIZE * sizeof(sub_transaction_t));

        size = (i + CACHE_SIZE) > BATCH_SIZE ? BATCH_SIZE - i : CACHE_SIZE;

        for (uint32_t j = 0; j < size; ++j)
        {
            sub_tx = &cache[tid].sub_txs[j];

            exec(sub_tx->params, sub_tx->type, sub_tx->mode, sub_tx->ts);
        }
    }
}
