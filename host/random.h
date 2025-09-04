#pragma once

#include <ctime>
#include <random>

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

class Random
{
  public:
    Random()
    {
        seed_ = (unsigned long)time(NULL);
    }

    // [min, max]
    int
    generate(int min, int max)
    {
        unsigned long rand_int;

        rand_int = RAND_R_FNC(seed_);

        return (int)((rand_int % (max - min + 1)) + min);
    }

    // [min, max[
    float
    generate(int digits, float min, float max)
    {
        int multiplier = 1;
        int int_lower, int_upper;

        for (int i = 0; i < digits; ++i)
        {
            multiplier *= 10;
        }

        int_lower = static_cast<int>(min * static_cast<double>(multiplier) + 0.5);
        int_upper = static_cast<int>(max * static_cast<double>(multiplier) + 0.5);

        return (float)generate(int_lower, int_upper) / (float)multiplier;
    }

  private:
    unsigned long seed_;
};
