#pragma once
#include "utils/random/IRNG.h"

#include <vector>

class MockRNG : public IRNG
{
public:
    explicit MockRNG(const std::vector<double> &values);
    double uniform_real(double /*min*/, double /*max*/) override;
    int uniform_int(int /*min*/, int /*max*/) override;

private:
    double next_value();

    std::vector<double> values_;
    size_t index_;
};
