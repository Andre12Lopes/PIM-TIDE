#include <chrono>
#include <cstdint>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>

#include "system.h"
#include "tpcc_client.h"

using namespace std::chrono;

#define N_TXS 32000000

enum param_types
{
    PARAM_N_THREADS = (unsigned char)'t',
    PARAM_N_DPUS    = (unsigned char)'d',
};

enum param_defaults
{
    PARAM_DEFAULT_N_THREADS = 2,
    PARAM_DEFAULT_N_DPUS    = 1,
};

long params[256]; // 256 = ascii limit

void
display_usage(const char *app_name)
{
    printf("Usage: %s [options]\n", app_name);
    puts("\nOptions:                                 (defaults)");
    printf("  -t <UINT>   Number of [t]hreads [>= 2] (%d)\n", PARAM_DEFAULT_N_THREADS);
    printf("  -d <UINT>   Number of [d]pus           (%d)\n", PARAM_DEFAULT_N_DPUS);

    exit(1);
}

void
set_default_params()
{
    params[PARAM_N_THREADS] = PARAM_DEFAULT_N_THREADS;
    params[PARAM_N_DPUS]    = PARAM_DEFAULT_N_DPUS;
}

void
parse_args(long argc, char *const argv[])
{
    long opt;

    opterr = 0;

    set_default_params();

    while ((opt = getopt(argc, argv, "t:d:l:h")) != -1)
    {
        switch (opt)
        {
        case 't':
        case 'd':
        case 'l':
            params[(unsigned char)opt] = atol(optarg);
            break;
        case 'h':
            display_usage(argv[0]);
            break;
        default:
            opterr++;
            break;
        }
    }

    if (params[PARAM_N_THREADS] < 2 || params[PARAM_N_DPUS] < 1)
    {
        display_usage(argv[0]);
    }

    if (opterr)
    {
        display_usage(argv[0]);
    }
}

int
main(int argc, char *const argv[])
{
    parse_args(argc, argv);

    System system(params[PARAM_N_THREADS], params[PARAM_N_DPUS]);

    // ########################################################################

    TPCCClient tpcc_client(params[PARAM_N_DPUS]);

    // for (int i = 0; i < N_TXS; ++i)
    // {
    //     tpcc_client.payment(system);
    //     tpcc_client.new_order(system);
    //     tpcc_client.order_status(system);
    // }

    for (int i = 0; i < N_TXS; ++i)
    {
        int x = (rand() % 100) + 1;

        if (x <= 25)
        {
            tpcc_client.payment(system);
        }
        else if (x >= 26 && x <= 50)
        {
            tpcc_client.new_order(system);
        }
        else
        {
            tpcc_client.order_status(system);
        }
    }

    // ########################################################################

    system.wait_initialization();

    auto start = steady_clock::now();

    while (system.n_commited_txs_ < system.n_transactions_)
    {
    }

    system.stop();
    volatile double exec_time = duration_cast<milliseconds>(steady_clock::now() - start).count();

    std::uint64_t n_aborts = 0;
    for (int i = 0; i < params[PARAM_N_DPUS]; ++i)
    {
        for (int j = 0; j < NR_TASKLETS; ++j)
        {
            n_aborts += system.aborts_[i][j];
        }
    }

    double dpu_time  = system.dpu_time_;
    double copy_time = system.copy_time_;
    double cpu_time  = exec_time - (dpu_time + copy_time);

    std::uint64_t n_rounds           = system.n_rounds_;
    std::uint64_t n_commited_sub_txs = system.n_commited_txs_;

    std::cout << N_TXS << "\t"                   //
              << n_commited_sub_txs << "\t"      //
              << DIST_PAYMENT << "\t"            //
              << n_aborts << "\t"                //
              << params[PARAM_N_DPUS] << "\t"    //
              << params[PARAM_N_THREADS] << "\t" //
              << NR_TASKLETS << "\t"             //
              << BATCH_SIZE << "\t"              //
              << exec_time << "\t"               //
              << cpu_time << "\t"                //
              << dpu_time << "\t"                //
              << copy_time << "\t"               //
              << n_rounds << "\t"                //
              << std::endl;                      //

    return 0;
}
