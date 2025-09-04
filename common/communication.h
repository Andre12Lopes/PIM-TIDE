#pragma once

#include <stdint.h>

#define N_PARAMS 56

enum
{
    DETERMINISTIC = 1,
    OPTIMISTIC    = 2,
};

typedef unsigned char byte;

typedef struct sub_transaction
{
    byte params[N_PARAMS];
    uint32_t ts;
    uint16_t n_params;
    uint8_t type;
    uint8_t mode;
} sub_transaction_t;

typedef struct sub_transaction_batch
{
    sub_transaction_t sub_txs[BATCH_SIZE];
    uint64_t n_det_sub_txs;
    uint64_t n_opt_sub_txs;
} sub_transaction_batch_t;

typedef struct result_batch
{
    uint32_t results[BATCH_SIZE];
    uint32_t n_results;
} result_batch_t;
