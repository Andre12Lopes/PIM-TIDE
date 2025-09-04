#include <assert.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sched.h>
#include <unistd.h>

#include "../common/communication.h"
#include "system.h"

using namespace std::chrono;

System::System(std::uint32_t n_threads, std::uint32_t n_dpus)
    : n_commited_txs_(0),
      n_transactions_(0),
      n_rounds_(0),
      dpu_time_(0),
      copy_time_(0),
      aborts_(n_dpus, std::vector<std::uint32_t>(NR_TASKLETS)),
      n_dpus_(n_dpus),
      n_threads_(n_threads),
      batch_size_(BATCH_SIZE),
      running_(1),
      sub_transactions_(n_dpus),
      batch_(n_dpus, std::vector<sub_transaction_batch_t>(NR_TASKLETS)),
      ts_(0),
      dpu_ts_(n_dpus, 0)
{
    assert(n_dpus_ % (n_threads_ - 1) == 0);

    for (uint32_t dpu = 0; dpu < n_dpus; ++dpu)
    {
        for (uint32_t t = 0; t < NR_TASKLETS; ++t)
        {
            aborts_[dpu][t] = 0;
        }
    }

    launch_threads();
}

void
System::exec(Transaction &transaction)
{
    std::uint32_t timestamp = 0;

    std::unique_lock<std::mutex> unique_lock(mutex_);

    if (transaction.sub_txs.size() < 1)
    {
        return;
    }
    else if (transaction.sub_txs.size() == 1)
    {
        transaction.set_optimistic();

        timestamp = ts_;
    }
    else
    {
        transaction.set_deterministic();

        timestamp = ts_++;
    }

    for (auto &it : transaction.sub_txs)
    {
        if (it.first >= (int)n_dpus_)
        {
            continue;
        }

        n_transactions_++;
        it.second.set_ts(timestamp);

        sub_transactions_[it.first].add(it.second);
    }
}

void
System::wait_initialization()
{
    barrier_.wait(n_threads_ + 1);
}

void
System::stop()
{
    running_ = 0;

    for (std::thread &t : threads_)
    {
        t.join();
    }
}

void
System::launch_threads()
{
    cpu_set_t cpuset;

    threads_.push_back(std::thread(&System::run, this));

    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_setaffinity_np(threads_[0].native_handle(), sizeof(cpu_set_t), &cpuset);

    for (uint32_t i = 0; i < n_threads_ - 1; ++i)
    {
        threads_.push_back(std::thread(&System::run_helper, this, i));

        CPU_ZERO(&cpuset);
        CPU_SET(i + 1, &cpuset);
        pthread_setaffinity_np(threads_[i + 1].native_handle(), sizeof(cpu_set_t), &cpuset);
    }
}

void
System::run()
{
    dpu::DpuSet dpus = dpu::DpuSet::allocate(n_dpus_);

    dpus.load(EXECUTABLE_PATH);

    launch_init_kernel(dpus);

    wait_initialization();

    while (running_)
    {
        barrier_.wait(n_threads_);

        for (std::uint32_t i = 0; i < (n_threads_ - 1); ++i)
        {
            n_commited_txs_ += n_commited_txs_thread_[i].n;
        }
        n_rounds_++;

        launch_kernel(dpus);
    }
}

void
System::run_helper(std::uint32_t t_id)
{
    wait_initialization();

    while (running_)
    {
        build_batch(t_id);

        barrier_.wait(n_threads_);

        barrier_.wait(n_threads_);
    }
}

void
System::build_batch(std::uint32_t t_id)
{
    SubTransaction sub_transaction;
    std::uint64_t ts, prev_round_dpu_ts;
    std::uint32_t idx, thread;

    n_commited_txs_thread_[t_id].n = 0;

    for (std::uint32_t dpu = t_id; dpu < n_dpus_; dpu += (n_threads_ - 1))
    {
        for (int t = 0; t < NR_TASKLETS; ++t)
        {
            batch_[dpu][t].n_det_sub_txs = 0;
            batch_[dpu][t].n_opt_sub_txs = 0;
        }

        prev_round_dpu_ts = dpu_ts_[dpu];
        thread            = 0;
        while (sub_transactions_[dpu].count() > 0 &&
               batch_[dpu][thread].n_det_sub_txs + batch_[dpu][thread].n_opt_sub_txs < batch_size_)
        {
            sub_transaction = sub_transactions_[dpu].remove();

            dpu_ts_[dpu]++;

            if (sub_transaction.is_optimistic())
            {
                ts  = 0;
                idx = batch_size_ - ++batch_[dpu][thread].n_opt_sub_txs;
            }
            else
            {
                ts  = prev_round_dpu_ts + thread + (batch_[dpu][thread].n_det_sub_txs * NR_TASKLETS);
                idx = batch_[dpu][thread].n_det_sub_txs++;
            }

            batch_[dpu][thread].sub_txs[idx]    = sub_transaction.clone();
            batch_[dpu][thread].sub_txs[idx].ts = ts;

            n_commited_txs_thread_[t_id].n++;

            thread = (thread + 1) % NR_TASKLETS;
        }
    }
}

void
System::launch_init_kernel(dpu::DpuSet &dpus)
{
    std::vector<std::vector<std::uint32_t>> dpu_ids(dpus.dpus().size(), std::vector<std::uint32_t>(1));

    assert((std::uint32_t)dpus.dpus().size() == n_dpus_);

    for (std::uint32_t i = 0; i < n_dpus_; ++i)
    {
        dpu_ids[i][0] = i + 1;
    }

    try
    {
        dpus.copy("dpu_id", dpu_ids);
        dpus.exec();

#ifdef DEBUG
        dpus.log(std::cout);
#endif
    }
    catch (const dpu::DpuError &e)
    {
#ifdef DEBUG
        dpus.log(std::cout);
#endif
        std::cout << "[INITIALIZATION ERROR] " << e.what() << std::endl;
    }
}

void
System::launch_kernel(dpu::DpuSet &dpus)
{
    try
    {
        auto start = steady_clock::now();
        dpus.copy("input", batch_);
        copy_time_ += duration_cast<milliseconds>(steady_clock::now() - start).count();
        barrier_.wait(n_threads_);

        // -------------------------------------------

        start = steady_clock::now();
        dpus.exec();
        dpu_time_ += duration_cast<milliseconds>(steady_clock::now() - start).count();

        // -------------------------------------------

        dpus.copy(aborts_, "aborts");

#ifdef DEBUG
        dpus.log(std::cout);
#endif
    }
    catch (const dpu::DpuError &e)
    {
#ifdef DEBUG
        dpus.log(std::cout);
#endif
        std::cout << "[ERROR] " << e.what() << std::endl;

        exit(1);
    }
}
