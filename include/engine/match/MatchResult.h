#pragma once
#include <cstdint>

struct MatchResult
{
    uint32_t filledQty = 0;       // total filled qty for incoming
    bool allOrNoneFailed = false; // true if FOK/AON failed
};
