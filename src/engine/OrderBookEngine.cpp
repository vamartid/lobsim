#include "engine/OrderBookEngine.h"

#include <algorithm>
#include <iostream>

OrderBookEngine::OrderBookEngine(std::unique_ptr<IMatchingStrategy> strategy)
    : bidsView_(bids_),
      asksView_(asks_),
      matching_strategy_(std::move(strategy))
{
}

void OrderBookEngine::add_order(Order &order)
{
    // Determine the side
    if (order.isBuy())
        add_order_to_side(bids_, order);
    else
        add_order_to_side(asks_, order);
}

void OrderBookEngine::cancel_order(uint64_t order_id)
{
    auto it = id_lookup_.find(order_id);
    if (it == id_lookup_.end())
        return;

    auto &[side, price, order_it] = it->second;

    if (side == Order::Side::Buy)
        cancel_order_on_side(bids_, side, price, order_it);
    else
        cancel_order_on_side(asks_, side, price, order_it);

    id_lookup_.erase(it);
}

template <typename SideType>
void OrderBookEngine::add_order_to_side(SideType &book_side, Order &incoming)
{
    // Insert incoming order into the book
    book_side.add_order(incoming);
    // auto it = std::prev(book_side.price_levels_[incoming.price].end());
    auto &orders = book_side.get_orders_at_price(incoming.price);
    auto it = std::prev(orders.end());
    id_lookup_[incoming.id] = std::make_tuple(incoming.side(), incoming.price, it);

    // Match against opposite side (read-only view)
    const IOrderBookSideView &oppositeView = incoming.isBuy() ? asksView_ : bidsView_;
    std::vector<FillOp> fills;

    // Strategy computes fills but does NOT mutate resting orders
    MatchResult result = matching_strategy_->match(incoming, oppositeView, fills);

    // Apply fills to resting orders and update id_lookup_
    apply_fill_ops(fills);

    // Reduce incoming quantity by filled amount
    incoming.quantity -= result.filledQty;

    // Optional: IOC or FOK handling
    if (incoming.isIOC() && incoming.quantity > 0)
    {
        cancel_order_on_side(book_side, incoming.side(), incoming.price, it);
        id_lookup_.erase(incoming.id);
    }
}

void OrderBookEngine::apply_fill_ops(const std::vector<FillOp> &fills)
{
    for (const auto &fill : fills)
    {

        // // Update trackers if needed
        // if (order_tracker_)
        //     order_tracker_->record_fill(fill.makerOrderId, fill.qty, fill.price);

        // Remove or reduce resting order
        auto it = id_lookup_.find(fill.makerOrderId);
        if (it != id_lookup_.end())
        {
            auto &[side, price, order_it] = it->second;
            order_it->quantity -= fill.quantity;

            if (order_it->quantity == 0)
            {
                if (side == Order::Side::Buy)
                    cancel_order_on_side(bids_, side, price, order_it);
                else
                    cancel_order_on_side(asks_, side, price, order_it);

                id_lookup_.erase(it);
            }
        }
    }
}

template <typename SideType>
void OrderBookEngine::cancel_order_on_side(
    SideType &book_side,
    Order::Side,
    double price,
    typename std::list<Order>::iterator order_it)
{
    auto &orders = book_side.get_orders_at_price(price);
    orders.erase(order_it);

    if (book_side.empty_at_price(price))
    {
        book_side.remove_price_level(price);
    }
}

void OrderBookEngine::set_order_tracker(std::shared_ptr<OrderTracker> tracker)
{
    order_tracker_ = tracker;
}
