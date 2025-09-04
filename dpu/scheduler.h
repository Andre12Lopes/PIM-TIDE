#pragma once

#include <stdint.h>

#include "../common/communication.h"
#include "util.h"

#define CACHE_SIZE 16

typedef struct cache
{
    __dma_aligned sub_transaction_t sub_txs[CACHE_SIZE];
} cache_t;

void scheduler_init(uint32_t dpu_id);
void scheduler_process_sub_transactions(int tid);
