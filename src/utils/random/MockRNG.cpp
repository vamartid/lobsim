#include "utils/random/MockRNG.h"

MockRNG::MockRNG(const std::vector<double> &values)
    : values_(values), index_(0) {}

double MockRNG::uniform_real(double /*min*/, double /*max*/)
{
    return next_value();
}

int MockRNG::uniform_int(int /*min*/, int /*max*/)
{
    return static_cast<int>(next_value());
}

double MockRNG::next_value()
{
    if (values_.empty())
        return 0.0;
    double val = values_[index_];
    index_ = (index_ + 1) % values_.size();
    return val;
}
