// OrderBookEngine.cpp
#include "engine/OrderBookEngine.h"
#include "engine/match/DefaultMatchingStrategy.h"

#include <iostream>

OrderBookEngine::OrderBookEngine() : bids_(Side::Buy),
                                     asks_(Side::Sell),
                                     matching_strategy_(std::make_unique<DefaultMatchingStrategy>()) {

                                     };

template <typename SideType>
void OrderBookEngine::add_order_to_side(SideType &book_side, const Order &order)
{
    auto &orders_at_price = book_side.price_levels()[order.price];
    orders_at_price.push_back(order);

    auto it = std::prev(orders_at_price.end()); // iterator to newly inserted order
    id_lookup_[order.id] = std::make_tuple(order.side, order.price, it);
}

void OrderBookEngine::add_order(const Order &order)
{
    if (order.side == Side::Buy)
    {
        add_order_to_side(bids_, order);
    }
    else
    {
        add_order_to_side(asks_, order);
    }
    if (order_tracker_)
        order_tracker_->add_order(order);
}

template <typename SideType>
void OrderBookEngine::cancel_order_on_side(SideType &book_side, Side side, double price, OrderIterator order_it)
{
    auto &orders_at_price = book_side.price_levels()[price];
    orders_at_price.erase(order_it);

    if (orders_at_price.empty())
        book_side.price_levels().erase(price);

    // Remove from lookup handled outside this function
}

void OrderBookEngine::cancel_order(uint64_t order_id)
{
    auto found = id_lookup_.find(order_id);
    if (found == id_lookup_.end())
        return;

    const auto &[side, price, order_it] = found->second;

    if (side == Side::Buy)
    {
        cancel_order_on_side(bids_, side, price, order_it);
    }
    else
    {
        cancel_order_on_side(asks_, side, price, order_it);
    }
    if (order_tracker_)
        order_tracker_->remove_order(order_id);
    id_lookup_.erase(found);
}

void OrderBookEngine::match_order(const Order &incoming_order)
{
    // Insert order first
    add_order(incoming_order);

    // Pass the whole lookup to the strategy so it can remove matched orders quickly
    matching_strategy_->match(
        bids_,
        asks_,
        id_lookup_);
}

void OrderBookEngine::print() const
{
    bids_.print("BUY");
    asks_.print("SELL");
}

void OrderBookEngine::set_order_tracker(std::shared_ptr<OrderTracker> tracker)
{
    order_tracker_ = std::move(tracker);
}

template void OrderBookEngine::add_order_to_side<DescendingSide>(DescendingSide &book_side, const Order &order);
template void OrderBookEngine::add_order_to_side<AscendingSide>(AscendingSide &book_side, const Order &order);

template void OrderBookEngine::cancel_order_on_side<DescendingSide>(DescendingSide &, Side, double, OrderIterator);
template void OrderBookEngine::cancel_order_on_side<AscendingSide>(AscendingSide &, Side, double, OrderIterator);