#pragma once
#include <cstdint>

/**
 * @brief View into a single side of the order book.
 *        Exposes read-only iteration for strategies without revealing container internals.
 */
struct PriceLevelView
{
    double price;
    size_t order_count;
    uint32_t aggregate_qty; // new: total qty at this level
    // // optional optimization
    // template <typename Fn>
    // void for_each_level(Fn &&fn) const;
};
