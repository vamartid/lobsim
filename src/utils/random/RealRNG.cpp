#include "utils/random/RealRNG.h"

#include <chrono>

RealRNG::RealRNG(std::optional<unsigned> seed)
{
    if (seed)
        engine_.seed(*seed);
    else
    {
        std::random_device rd;
        auto time_seed = static_cast<unsigned>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::seed_seq seq{rd(), rd(), rd(), time_seed};
        engine_.seed(seq);
    }
}

double RealRNG::uniform_real(double min, double max)
{
    std::uniform_real_distribution<double> dist(min, max);
    return dist(engine_);
}

int RealRNG::uniform_int(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(engine_);
}
