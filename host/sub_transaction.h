#pragma once

#include <cstdint>
#include <cstdlib>

#include "../common/communication.h"

class SubTransaction
{
  public:
    SubTransaction();
    SubTransaction(const SubTransaction &that);

    SubTransaction &operator=(const SubTransaction &that);
    sub_transaction_t clone();

    void add_param(std::uint16_t p);
    void add_param(std::uint32_t p);
    void add_param(float p);
    void add_param(char *p, size_t size);
    void set_ts(std::uint32_t ts);
    void set_type(std::uint8_t type);
    int is_deterministic();
    void set_deterministic();
    int is_optimistic();
    void set_optimistic();

  private:
    byte params_[N_PARAMS];
    std::uint32_t ts_;
    std::uint16_t n_params_;
    std::uint8_t type_; // PAYMENT/NEW_ORDER
    std::uint8_t mode_; // DETERMINISTIC/OPTIMISTIC
};
