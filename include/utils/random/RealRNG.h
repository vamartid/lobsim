#pragma once
#include "utils/random/IRNG.h"

#include <random>
#include <optional>

class RealRNG : public IRNG
{
public:
    explicit RealRNG(std::optional<unsigned> seed = std::nullopt);
    double uniform_real(double min, double max) override;
    int uniform_int(int min, int max) override;

private:
    std::default_random_engine engine_;
};
