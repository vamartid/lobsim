#pragma once

#include <cstdint>
#include <string>
#include <format>

#include "utils/StringUtils.h"

enum class Side
{
    Buy,
    Sell
};

struct Order
{
    uint64_t id;           // 8 bytes
    uint64_t timestamp_ns; // 8 bytes
    double price;          // 8 bytes
    uint32_t quantity;     // 4 bytes
    Side side;             // 4 bytes
    // uint16_t feeder_id = UINT16_MAX; // UINT16_MAX means no feeder assigned

    std::string to_string() const
    {
        using namespace utils::console_colors;
        using utils::string::color_wrap;

        auto side_colored = color_wrap((side == Side::Buy) ? "BUY" : "SELL",
                                       (side == Side::Buy) ? GREEN : RED);

        return std::format(
            "{} | {} | ID: {} | Price: {:.2f} | Qty: {}",
            color_wrap(std::format("Time: {}", timestamp_ns), CYAN),
            side_colored,
            id,
            price,
            quantity);
    }
};
