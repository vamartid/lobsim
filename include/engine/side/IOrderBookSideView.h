#pragma once

#include "core/Order.h"
#include "engine/side/PriceLevelView.h"

#include <cstdint>
#include <optional>
#include <functional>
class IOrderBookSideView
{
public:
    virtual ~IOrderBookSideView() = default;

    // return best price (nullopt if empty)
    virtual std::optional<double> best_price() const = 0;

    // total number of levels
    virtual size_t num_levels() const = 0;

    // iterate over price levels
    virtual void for_each_level(const std::function<void(const PriceLevelView &)> &fn) const = 0;

    // iterate over orders at a specific price
    virtual void for_each_order_at_price(double price, const std::function<void(const Order &)> &fn) const = 0;
};