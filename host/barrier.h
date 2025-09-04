#pragma once

#include <condition_variable>
#include <mutex>

class Barrier
{
  private:
    int waiting;

    std::mutex mutex;
    std::condition_variable cond;

  public:
    Barrier()
        : waiting(0)
    {
    }

    void
    wait(int n_threads)
    {
        std::unique_lock<std::mutex> unique_lock(mutex);

        if (++waiting == n_threads)
        {
            waiting = 0;

            unique_lock.unlock();
            cond.notify_all();

            return;
        }
        else
        {
            cond.wait(unique_lock);
        }

        unique_lock.unlock();
    }
};
