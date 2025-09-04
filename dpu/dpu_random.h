#pragma once

#define RAND_R_FNC(seed)                                                                                \
    ({                                                                                                  \
        unsigned long next = seed;                                                                      \
        unsigned long result;                                                                           \
        next *= 1103515245;                                                                             \
        next += 12345;                                                                                  \
        result = (unsigned long)(next / 65536) % 2048;                                                  \
        next *= 1103515245;                                                                             \
        next += 12345;                                                                                  \
        result <<= 10;                                                                                  \
        result ^= (unsigned long)(next / 65536) % 1024;                                                 \
        next *= 1103515245;                                                                             \
        next += 12345;                                                                                  \
        result <<= 10;                                                                                  \
        result ^= (unsigned long)(next / 65536) % 1024;                                                 \
        seed = next;                                                                                    \
        result;                                                                                         \
    })

unsigned long seed_ = 123;

// [min, max]
int
generate_random_int(int min, int max)
{
    unsigned long rand_int;

    rand_int = RAND_R_FNC(seed_);

    return (int)((rand_int % (max - min + 1)) + min);
}

// [min, max[
float
generate_random_float(int digits, float min, float max)
{
    int multiplier = 1;
    int int_lower, int_upper;

    for (int i = 0; i < digits; ++i)
    {
        multiplier *= 10;
    }

    int_lower = (int)((min * (double)multiplier) + 0.5);
    int_upper = (int)((max * (double)multiplier) + 0.5);

    return (float)generate_random_int(int_lower, int_upper) / (float)multiplier;
}

void
generate_random_string(__mram_ptr char *str, int min_size, int max_size)
{
	int size;

	size = generate_random_int(min_size, max_size - 1);

	for (int i = 0; i < size; ++i)
	{
		str[i] = (char)generate_random_int(97, 122);
	}

	str[size] = '\0';
}
