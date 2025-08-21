#pragma once
#include <map>
#include <list>
#include <optional>
#include <functional>
#include "IOrderBookSideView.h"
#include "utils/Comparator.h"

template <typename Compare>
class OrderBookSide : public IOrderBookSideView
{
public:
    using OrderList = std::list<Order>;
    using PriceLevel = OrderList;
    using PriceMap = std::map<double, PriceLevel, Compare>;

    // add an order
    void add_order(const Order &o);

    // expose orders at price (non-const for engine use)
    OrderList &get_orders_at_price(double price);
    const OrderList &get_orders_at_price(double price) const;

    // IOrderBookSideView overrides
    std::optional<double> best_price() const override;
    size_t num_levels() const override;
    void for_each_level(const std::function<void(const PriceLevelView &)> &fn) const override;
    void for_each_order_at_price(double price, const std::function<void(const Order &)> &fn) const override;
    void remove_price_level(double price);
    bool empty_at_price(double price) const;

private:
    PriceMap price_levels_;
};

using BidBookSide = OrderBookSide<utils::comparator::Descending>;
using AskBookSide = OrderBookSide<utils::comparator::Ascending>;
