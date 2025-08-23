#pragma once
#include <cstdint>
#include <format>

struct FillOp
{
    uint64_t makerOrderId; // resting order consumed
    uint32_t quantity;     // executed quantity
    double price;          // execution price
};

// --- FillOp ---
template <>
struct std::formatter<FillOp> : std::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const FillOp &f, FormatContext &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("makerOrderId:{} Qty:{} Price:{:.2f}",
                        f.makerOrderId,
                        f.quantity,
                        f.price),
            ctx);
    }
};