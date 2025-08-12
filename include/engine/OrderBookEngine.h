#pragma once

#include "engine/match/IMatchingStrategy.h"
#include "engine/OrderBookSide.h"
#include "utils/OrderTracker.h"

#include <unordered_map>
#include <tuple>
#include <memory>

// Support for injecting different matching strategies.
// Efficient lookup for cancellations.
// Separated price levels using std::map<double, OrderQueue>.
// Hooks for printing the order book and simple matching.

using DescendingSide = OrderBookSide<utils::comparator::Descending>;
using AscendingSide = OrderBookSide<utils::comparator::Ascending>;
using OrderIterator = std::list<Order>::iterator;

class OrderBookEngine
{
public:
    OrderBookEngine();

    void add_order(const Order &order);
    void cancel_order(uint64_t order_id);

    void match_order(const Order &incoming_order);
    void print() const;

    const DescendingSide &bids() const { return bids_; }
    const AscendingSide &asks() const { return asks_; }
    void set_order_tracker(std::shared_ptr<OrderTracker> tracker);

private:
    DescendingSide bids_;
    AscendingSide asks_;
    std::unique_ptr<IMatchingStrategy> matching_strategy_;
    std::shared_ptr<OrderTracker> order_tracker_;

    // Your OrderBookSide::add_order/ OrderBookSide::remove_order do a linear search, which can be slow for large order books
    // Fast lookup: id -> (side, price, iterator to order)
    // Tracks: side (Buy or Sell) ,The price level, iterator to the exact order in the std::list<Order> inside the price level
    std::unordered_map<uint64_t, std::tuple<Side, double, OrderIterator>> id_lookup_;

    template <typename SideType>
    void add_order_to_side(SideType &book_side, const Order &order);
    template <typename SideType>
    void cancel_order_on_side(SideType &book_side, Side side, double price, OrderIterator order_it);
};
