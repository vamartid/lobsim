#include "engine/match/MatchPrinter.h"

#include <iostream>
#include <format>

namespace match_printer
{

    void print_match(const Order &incoming, const Order &matched, uint64_t quantity)
    {
        using namespace utils::console_colors;
        using utils::string::color_wrap;

        const auto incoming_side = color_wrap(
            (incoming.side == Side::Buy) ? "BUY" : "SELL",
            (incoming.side == Side::Buy) ? GREEN : RED);

        const auto matched_side = color_wrap(
            (matched.side == Side::Buy) ? "BUY" : "SELL",
            (matched.side == Side::Buy) ? GREEN : RED);

        std::cout << std::format(
            "{} {} order (ID: {}) matched with {} order (ID: {}) for {} units at price {}\n",
            color_wrap("Match Detail:", CYAN),
            incoming_side, incoming.id,
            matched_side, matched.id,
            color_wrap(std::to_string(quantity), CYAN),
            color_wrap(std::format("{:.2f}", matched.price), CYAN));
    }

} // namespace match_printer
