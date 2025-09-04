#pragma once

#include <assert.h>
#include <mram.h>

#define MRAM __mram_ptr

#ifdef DEBUG
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

#define MEMBARLDLD() /* nothing */
#define MEMBARSTST() /* nothing */
#define MEMBARSTLD() __asm__ __volatile__("" : : : "memory")
