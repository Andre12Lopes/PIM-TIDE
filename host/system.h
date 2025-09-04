#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <dpu>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "barrier.h"
#include "buffer.h"
#include "sub_transaction.h"
#include "transaction.h"

#define EXECUTABLE_PATH "bin/dpu_main"

typedef struct cache_alligned_counter
{
    std::uint64_t n;
    std::uint64_t p[8];
} cache_alligned_counter_t;

class System
{
  public:
    System(std::uint32_t n_threads, std::uint32_t n_dpus);

    void exec(Transaction &transaction);
    void wait_initialization();
    void stop();

    std::atomic_uint64_t n_commited_txs_;
    std::atomic_uint64_t n_transactions_;
    std::atomic_uint64_t n_rounds_;
    volatile double dpu_time_;
    volatile double copy_time_;
    std::vector<std::vector<std::uint32_t>> aborts_;

  private:
    void launch_threads();
    void run();
    void run_helper(std::uint32_t t_id);
    void build_batch(std::uint32_t t_id);
    void launch_init_kernel(dpu::DpuSet &dpus);
    void launch_kernel(dpu::DpuSet &dpus);

    std::uint32_t n_dpus_;
    std::uint32_t n_threads_;
    std::uint32_t batch_size_;
    std::atomic_uint8_t running_;

    std::vector<std::thread> threads_;
    std::mutex mutex_;
    std::shared_mutex shared_mutex_;
    Barrier barrier_;

    std::vector<Buffer<SubTransaction>> sub_transactions_;
    std::vector<std::vector<sub_transaction_batch_t>> batch_;

    std::uint64_t ts_;
    std::vector<std::uint64_t> dpu_ts_;
    cache_alligned_counter_t n_commited_txs_thread_[64];
};
