#pragma once

#include "core/Order.h"
#include "engine/side/OrderBookSide.h"
#include "engine/match/IMatchingStrategy.h"

#include "engine/match/PriceTimePriorityStrategy.h"

#include "engine/match/FillOp.h"
#include "engine/match/MatchResult.h"

#include "engine/events/EventBus.h"

#include <memory>
#include <vector>
#include <cstdint>

class OrderBookEngine
{
public:
    OrderBookEngine(EventBus &bus, std::unique_ptr<IMatchingStrategy> strategy = std::make_unique<PriceTimePriorityStrategy>());

    // Add a new order to the book and run matching
    void add_order(Order &order);

    // Cancel an existing order by ID
    void cancel_order(uint64_t order_id);

    // Accessors for read-only views
    const BidBookSide &bids() const { return bids_; }
    const AskBookSide &asks() const { return asks_; }

private:
    BidBookSide bids_;
    AskBookSide asks_;
    IOrderBookSideView &bidsView_;
    IOrderBookSideView &asksView_;

    EventBus &bus_;
    Ticks current_tick_ = 0;
    Seq next_seq_ = 0;

    std::unique_ptr<IMatchingStrategy> matching_strategy_;

    // Fast lookup for cancellations: order_id -> (side, price, iterator)
    using OrderIterator = std::list<Order>::iterator;
    std::unordered_map<uint64_t, std::tuple<Order::Side, double, OrderIterator>> id_lookup_;

    // Internal helpers
    template <typename SideType>
    void add_order_to_side(SideType &book_side, Order &order);
    template <typename SideType>
    void cancel_order_on_side(SideType &book_side, Order::Side side, double price, OrderIterator order_it);

    // Apply FillOps from the strategy
    void apply_fill_ops(const std::vector<FillOp> &fills);
};
