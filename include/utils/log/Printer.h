#pragma once

#include "core/Order.h"

#include <ostream>
#include "utils/log/Printer.h"
#include "utils/log/ConsoleColors.h"
#include <ostream>
#include <cstdint>

namespace printer
{
    using namespace utils::console_colors;

    // Color-coded strings as inline constants
    inline constexpr const char *BUY_STR = GREEN_CODE "BUY" RESET_CODE;
    inline constexpr const char *SELL_STR = RED_CODE "SELL" RESET_CODE;

    // Print an order directly to an ostream with no intermediate string allocation
    inline void print_order(std::ostream &os, const Order &o)
    {
        os << '[' << CYAN << "Time: " << o.timestamp << RESET << "] | "
           << ((o.side() == Order::Side::Buy) ? BUY_STR : SELL_STR)
           << " | ID: " << o.id
           << " | Price: " << o.price
           << " | Qty: " << o.quantity;
    }

    // Print match details to an ostream with no intermediate string allocations
    inline void print_match(std::ostream &os, const Order &incoming, const Order &matched, uint64_t quantity)
    {
        os << CYAN << "Match Detail:" << RESET << " "
           << (incoming.isBuy() ? BUY_STR : SELL_STR)
           << " order (ID: " << incoming.id << ") matched with "
           << (matched.isBuy() ? BUY_STR : SELL_STR)
           << " order (ID: " << matched.id << ") for "
           << CYAN << quantity << RESET
           << " units at price " << CYAN << matched.price << RESET
           << '\n';
    }
}