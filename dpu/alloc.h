#pragma once

#include <mram.h>
#include <stddef.h>
#include <stdint.h>

void mram_malloc_init();
void mram_malloc_rebalance();

__mram_ptr void *mram_malloc(size_t size);
void mram_malloc_commit();
void mram_malloc_revert();

void print_mram_heap_pointer();
