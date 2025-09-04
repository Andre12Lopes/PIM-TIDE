#pragma once

#include <stdint.h>

#include "clock.h"
#include "random.h"
#include "system.h"

class TPCCClient
{
  public:
    TPCCClient(std::uint32_t n_warehouses);

    void payment(System &system);
    void new_order(System &system);
    void order_status(System &system);

  private:
    std::uint32_t n_warehouses_;

    Clock clock_;
    Random random_;
};
