#include "sub_transaction.h"

#include <cassert>
#include <cstring>
#include <string>

SubTransaction::SubTransaction()
    : ts_(0),
      n_params_(0),
      type_(0),
      mode_(0)
{
}

SubTransaction::SubTransaction(const SubTransaction &that)
{
    std::memcpy(params_, that.params_, N_PARAMS);
    ts_       = that.ts_;
    n_params_ = that.n_params_;
    type_     = that.type_;
    mode_     = that.mode_;
}

SubTransaction &
SubTransaction::operator=(const SubTransaction &that)
{
    std::memcpy(params_, that.params_, N_PARAMS);
    ts_       = that.ts_;
    n_params_ = that.n_params_;
    type_     = that.type_;
    mode_     = that.mode_;

    return *this;
}

sub_transaction_t
SubTransaction::clone()
{
    sub_transaction_t s;

    std::memcpy(s.params, params_, sizeof(byte) * n_params_);
    s.ts       = ts_;
    s.n_params = n_params_;
    s.type     = type_;
    s.mode     = mode_;

    return s;
}

void
SubTransaction::add_param(std::uint16_t p)
{
    if (n_params_ + sizeof(p) >= N_PARAMS)
    {
        assert(0);
        return;
    }

    std::copy((byte *)&p, ((byte *)&p) + sizeof(p), &params_[n_params_]);
    n_params_ += sizeof(p);
}

void
SubTransaction::add_param(std::uint32_t p)
{
    if (n_params_ + sizeof(p) >= N_PARAMS)
    {
        assert(0);
        return;
    }

    std::copy((byte *)&p, ((byte *)&p) + sizeof(p), &params_[n_params_]);
    n_params_ += sizeof(p);
}

void
SubTransaction::add_param(float p)
{
    if (n_params_ + sizeof(p) >= N_PARAMS)
    {
        assert(0);
        return;
    }

    std::copy((byte *)&p, ((byte *)&p) + sizeof(p), &params_[n_params_]);
    n_params_ += sizeof(p);
}

void
SubTransaction::add_param(char *p, size_t size)
{
    if (n_params_ + size >= N_PARAMS)
    {
        assert(0);
        return;
    }

    std::copy((unsigned char *)p, ((unsigned char *)p) + size, &params_[n_params_]);
    n_params_ += size;
}

void
SubTransaction::set_ts(std::uint32_t ts)
{
    ts_ = ts;
}

void
SubTransaction::set_type(std::uint8_t type)
{
    type_ = type;
}

int
SubTransaction::is_deterministic()
{
    return (mode_ == DETERMINISTIC ? 1 : 0);
}

void
SubTransaction::set_deterministic()
{
    mode_ = DETERMINISTIC;
}

int
SubTransaction::is_optimistic()
{
    return (mode_ == OPTIMISTIC ? 1 : 0);
}

void
SubTransaction::set_optimistic()
{
    mode_ = OPTIMISTIC;
}
