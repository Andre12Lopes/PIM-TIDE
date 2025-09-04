#pragma once

#include <map>

#include "sub_transaction.h"

class Transaction
{
  public:
    Transaction();
    Transaction(const Transaction &transaction);

    void add(int dpu, SubTransaction &sub_transaction);
    void set_deterministic();
    void set_optimistic();

    std::map<int, SubTransaction> sub_txs;

  private:
};
