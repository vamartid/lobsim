#pragma once
#include <cstdint>
#include "core/Order.h" // for Order::Side

/**
 * @brief Public-facing representation of an executed trade.
 *        Derived from engine FillOps but enriched with context
 *        useful for views, renderers, and logging.
 */
using Ticks = uint32_t;
struct TradeInfo
{
    uint64_t makerId;
    uint64_t takerId;
    double price; // price, keep double to match E_Fill
    int64_t qty;  // executed quantity
    Ticks ts;     // timestamp (ticks)
    uint32_t seq;
};

template <>
struct std::formatter<TradeInfo> : std::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const TradeInfo &x, FormatContext &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("Time:{} Maker:{} Taker:{} Price:{:.2f} Qty:{}",
                        x.ts,      // timestamp
                        x.makerId, // then maker
                        x.takerId, // then taker
                        x.price,   // price
                        x.qty),    // quantity
            ctx);
    }
};
