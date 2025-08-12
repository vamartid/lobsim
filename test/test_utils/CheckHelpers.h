#pragma once

#include <vector>

template <typename T>
bool all_in_range(const std::vector<T> &values, T min, T max)
{
    for (const auto &v : values)
    {
        if (v < min || v > max)
            return false;
    }
    return true;
}