#include <assert.h>
#include <defs.h>
#include <stdio.h>

#include "alloc.h"

typedef struct mem_block
{
    uintptr_t addr;   // First free address in the block
    uintptr_t t_addr; // Tentative first free address in the block
    uintptr_t used;   // Number of bytes used in the block
    uintptr_t t_used; // Tentative number of bytes used in the block
    uintptr_t size;   // Total size of the block
} mem_block_t;

mem_block_t blocks[NR_TASKLETS];

void
mram_malloc_init()
{
    assert(DPU_MRAM_HEAP_POINTER < (__mram_ptr void *)0x8FFFFF);

    blocks[0].addr   = (uintptr_t)DPU_MRAM_HEAP_POINTER;
    blocks[0].t_addr = (uintptr_t)DPU_MRAM_HEAP_POINTER;
    blocks[0].used   = 0;
    blocks[0].t_used = 0;
    blocks[0].size   = 60000000;
}

void
mram_malloc_rebalance()
{
    uintptr_t size;

    size = (62000000 - blocks[0].addr) / NR_TASKLETS;

    blocks[0].used   = 0;
    blocks[0].t_used = 0;
    blocks[0].size   = size;

    for (int tid = 1; tid < NR_TASKLETS; ++tid)
    {
        blocks[tid].addr   = blocks[tid - 1].addr + blocks[tid - 1].size;
        blocks[tid].t_addr = blocks[tid - 1].addr + blocks[tid - 1].size;
        blocks[tid].used   = 0;
        blocks[tid].t_used = 0;
        blocks[tid].size   = size;
    }
}

__mram_ptr void *
mram_malloc(size_t size)
{
    int tid;
    uintptr_t addr;

    tid = me();

    if (size <= 0)
    {
        assert(0);
        return NULL;
    }

    if (blocks[tid].t_used + size >= blocks[tid].size)
    {
        assert(0);
        return NULL;
    }

    addr = blocks[tid].t_addr;

    blocks[tid].t_addr += size;
    blocks[tid].t_used += size;

    return (__mram_ptr void *)addr;
}

void
mram_malloc_commit()
{
    int tid;

    tid = me();

    blocks[tid].addr = blocks[tid].t_addr;
    blocks[tid].used = blocks[tid].t_used;
}

void
mram_malloc_revert()
{
    int tid;

    tid = me();

    blocks[tid].t_addr = blocks[tid].addr;
    blocks[tid].t_used = blocks[tid].used;
}

void
print_mram_heap_pointer()
{
    int tid;

    tid = me();

    printf(
        "t%d -> ADDR = %u | USED = %u | SIZE = %u\n", tid, blocks[tid].addr, blocks[tid].used,
        blocks[tid].size);
}
