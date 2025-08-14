#include "utils/random/MockRNG.h"
#include <iostream>
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

// In utils/random/MockRNG.cpp
double MockRNG::next_value()
{
    if (values_.empty())
    {
        // std::cerr << "MockRNG::next_value() called with an empty vector. Returning 0.0" << std::endl;
        return 0.0;
    }
    double val = values_[index_];
    // std::cout << "MockRNG::next_value() called. Returning " << val << " from index " << index_ << std::endl;
    index_ = (index_ + 1) % values_.size();
    return val;
}