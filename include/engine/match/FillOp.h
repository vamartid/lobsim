#pragma once
#include <cstdint>

struct FillOp
{
    uint64_t makerOrderId; // resting order consumed
    uint32_t quantity;     // executed quantity
    double price;          // execution price
};
