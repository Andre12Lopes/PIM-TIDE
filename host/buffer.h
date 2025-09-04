#pragma once

#include <assert.h>
#include <condition_variable>
#include <cstdint>
#include <mutex>

#define BUFFER_SIZE 2100000

template <class T> class Buffer
{
  public:
    Buffer()
        : add_idx_(0),
          remove_idx_(0),
          count_(0)
    {
    }

    void
    add(T item)
    {
        // RAII acquires mutex on constriction and releases
        // the mutex once the variable is destructed (falls out of scope)
        std::unique_lock<std::mutex> unique_lock(m);

        while (count_ == BUFFER_SIZE)
        {
            add_cond.wait(unique_lock);
        }

        buffer[add_idx_] = item;

        add_idx_ = (add_idx_ + 1) % BUFFER_SIZE;
        count_++;

        unique_lock.unlock();
    }

    T
    remove()
    {
        T item;

        // RAII acquires mutex on constriction and releases
        // the mutex once the variable is destructed (falls out of scope)
        std::unique_lock<std::mutex> unique_lock(m);

        if (count_ == 0)
        {
            assert(0);
            return item;
        }

        item        = buffer[remove_idx_];
        remove_idx_ = (remove_idx_ + 1) % BUFFER_SIZE;
        count_--;

        unique_lock.unlock();
        add_cond.notify_one();

        return item;
    }

    int
    count()
    {
        // RAII acquires mutex on constriction and releases
        // the mutex once the variable is destructed (falls out of scope)
        // TODO: Maybe mutex not needed here
        // std::unique_lock<std::mutex> unique_lock(m);
        return count_;
    }

  private:
    T buffer[BUFFER_SIZE];
    std::uint_fast32_t add_idx_;
    std::uint_fast32_t remove_idx_;
    std::uint_fast32_t count_;

    std::mutex m;
    std::condition_variable add_cond;
};
