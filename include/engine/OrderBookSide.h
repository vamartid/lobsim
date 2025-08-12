// OrderBookSide.h
#pragma once

#include "core/Order.h"

#include <map>
#include <list>

// OrderBookSide is parametrized on comparator
template <typename Compare>
class OrderBookSide
{
public:
    explicit OrderBookSide(Side side);

    void add_order(const Order &order);
    void remove_order(uint64_t order_id);

    // Return best price level or nullopt if empty
    std::optional<double> best_price() const;

    // Return list of orders at best price level
    std::list<Order> &best_orders();
    const std::list<Order> &best_orders() const;

    void print(const std::string &side_name) const;

    std::map<double, std::list<Order>, Compare> &price_levels() { return price_levels_; }

private:
    Side side_;
    // Map price -> list of orders at that price
    // For Buy: sorted descending; for Sell: ascending
    // To avoid big overhead due to std::function since it's a type-erased wrapper
    // which means every call goes through a small virtual dispatch
    // also when setting the std::function to comparator on constructor
    // we used a ternary creating temporaries and potential runtime overhead.
    std::map<double, std::list<Order>, Compare> price_levels_;
};
