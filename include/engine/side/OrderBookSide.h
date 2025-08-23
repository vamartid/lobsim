#pragma once
#include <map>
#include <list>
#include <optional>
#include <functional>
#include "IOrderBookSideView.h"
#include "utils/Comparator.h"
#include "engine/events/IEventListener.h" // for Event + EventType

template <typename Compare>
class OrderBookSide : public IOrderBookSideView, public IEventListener
{
public:
    using OrderList = std::list<Order>;
    using PriceLevel = OrderList;
    using PriceMap = std::map<double, PriceLevel, Compare>;

    // ---- IOrderBookSideView ----
    std::optional<double> best_price() const override;
    size_t num_levels() const override;
    void for_each_level(const std::function<void(const PriceLevelView &)> &fn) const override;
    void for_each_order_at_price(double price, const std::function<void(const Order &)> &fn) const override;

    // ---- engine-facing mutators ----

    // add an order
    void add_order(const Order &o);
    OrderList::iterator add_order_and_get_iterator(const Order &o);

    // expose orders at price (non-const for engine use)
    OrderList &get_orders_at_price(double price);
    const OrderList &get_orders_at_price(double price) const;

    void remove_price_level(double price);
    bool empty_at_price(double price) const;

    // ---- IEventListener ----
    void on_event(const Event &e) override; // <-- exact signature

private:
    static constexpr Order::Side side_tag(); // <-- static constexpr
    PriceMap price_levels_;

    // Event handles
    inline void handle_order_added(const E_OrderAdded &e);
    inline void handle_order_updated(const E_OrderUpdated &e);
    inline void handle_order_removed(uint64_t order_id);
    inline void handle_fill(const E_Fill &f);
};

using BidBookSide = OrderBookSide<utils::comparator::Descending>;
using AskBookSide = OrderBookSide<utils::comparator::Ascending>;
