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
#include <span>

using WallTime = uint32_t; // ms since epoch

class OrderBookEngine
{
public:
    static constexpr uint32_t MAX_TICKS = 1000000; // or whatever max you expect

    OrderBookEngine(EventBus &bus, std::unique_ptr<IMatchingStrategy> strategy = std::make_unique<PriceTimePriorityStrategy>());

    // Add a new order to the book and run matching
    void add_order(Order &order);

    // Cancel an existing order by ID
    void cancel_order(uint64_t order_id);

    // Accessors for read-only views
    const BidBookSide &bids() const { return bids_; }
    const AskBookSide &asks() const { return asks_; }

    std::span<const WallTime> tick_wall_times() const;

private:
    BidBookSide bids_;
    AskBookSide asks_;
    IOrderBookSideView &bidsView_;
    IOrderBookSideView &asksView_;

    EventBus &bus_;
    Ticks current_tick_ = 0;
    Seq next_seq_ = 0;

    std::unique_ptr<WallTime[]> tick_times_;
    // std::array<WallTime, MAX_TICKS> tick_times_{0};

    std::unique_ptr<IMatchingStrategy> matching_strategy_;

    // Fast lookup for cancellations: order_id -> (side, price, iterator)
    using OrderIterator = std::list<Order>::iterator;
    std::unordered_map<uint64_t, std::tuple<Order::Side, double, OrderIterator>> id_lookup_;

    void advance_tick();
    WallTime get_current_wall_time() const;

    // Internal helpers
    template <typename SideType>
    void add_order_to_side(SideType &book_side, Order &order);
    template <typename SideType>
    void cancel_order_on_side(SideType &book_side, Order::Side side, double price, OrderIterator order_it);

    // Apply FillOps from the strategy
    void apply_fill_ops(const std::vector<FillOp> &fills);
};
