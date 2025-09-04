#include "transaction.h"

Transaction::Transaction()
{
}

// Transaction::Transaction(const Transaction &tx)
// {
// }

void
Transaction::add(int dpu_id, SubTransaction &sub_transaction)
{
    sub_txs.insert({dpu_id, sub_transaction});
}

void
Transaction::set_deterministic()
{
    for (auto &it : sub_txs)
    {
        it.second.set_deterministic();
    }
}

void
Transaction::set_optimistic()
{
    for (auto &it : sub_txs)
    {
        it.second.set_optimistic();
    }
}
