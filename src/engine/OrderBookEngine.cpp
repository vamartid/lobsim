#include "engine/OrderBookEngine.h"
#include "utils/log/DebugLog.h"

#include <numeric>
#include <algorithm>
#include <iostream>

OrderBookEngine::OrderBookEngine(EventBus &bus, std::unique_ptr<IMatchingStrategy> strategy)
    : bidsView_(bids_),
      asksView_(asks_),
      bus_(bus),
      matching_strategy_(std::move(strategy))
{
    // Subscribe book sides to the bus
    bus_.add_listener([this](const Event &e)
                      { bids_.on_event(e); });
    bus_.add_listener([this](const Event &e)
                      { asks_.on_event(e); });
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
    DEBUG_ENGINE("Adding {}", incoming);

    // 1️⃣ Match against opposite side
    const IOrderBookSideView &oppositeView = incoming.isBuy() ? asksView_ : bidsView_;
    std::vector<FillOp> fills;

    MatchResult result = matching_strategy_->match(incoming, oppositeView, fills);
    DEBUG_ENGINE("{}", result);

    // 2️⃣ Publish fills
    for (const auto &fill : fills)
    {
        bus_(current_tick_, next_seq_++, E_Fill{fill.makerOrderId, fill.makerOrderId, fill.price, fill.quantity});
    }

    // 3️⃣ Apply fills to resting orders
    apply_fill_ops(fills);

    // 4️⃣ Reduce incoming quantity set zero if below zero
    DEBUG_SUBTRACT_INT64(DEBUG_ENGINE, "remaining_qty (add_order_to_side) = ", incoming.quantity, result.filledQty);
    incoming.quantity = result.filledQty >= incoming.quantity ? 0 : incoming.quantity - result.filledQty;
    DEBUG_ENGINE("After applying fills, incoming qty={}", incoming.quantity);

    // 5️⃣ If any quantity remains and not IOC/FOK, insert into book
    if (incoming.quantity > 0 && !(incoming.isIOC() || incoming.isFOK()))
    {
        auto it = book_side.add_order_and_get_iterator(incoming);
        id_lookup_[incoming.id] = std::make_tuple(incoming.side(), incoming.price, it);
        DEBUG_ENGINE("Added to book side {}", incoming);
        // Publish both OrderAdded and LevelAgg
        bus_(current_tick_, next_seq_++, E_OrderAdded{incoming.id, incoming.side(), incoming.price, incoming.quantity});
        // Publish LevelAgg for OrderBookView
        auto &orders_at_price = book_side.get_orders_at_price(incoming.price);
        bus_(current_tick_, next_seq_++, E_LevelAgg{incoming.side(), incoming.price, std::accumulate(orders_at_price.begin(), orders_at_price.end(), int64_t(0), [](int64_t sum, const Order &o)
                                                                                                     { return sum + o.quantity; })});
    }
    else if (incoming.quantity > 0)
    {
        DEBUG_ENGINE("Canceled (IOC/FOK) {}", incoming);
    }
}

void OrderBookEngine::apply_fill_ops(const std::vector<FillOp> &fills)
{
    for (const auto &fill : fills)
    {
        DEBUG_ENGINE("Applying {}", fill);

        auto it = id_lookup_.find(fill.makerOrderId);
        if (it == id_lookup_.end())
        {
            DEBUG_ENGINE("Maker order not found in id_lookup_ (id={})", fill.makerOrderId);
            continue;
        }

        auto &[side, price, order_it] = it->second;

        // Reduce quantity set zero if below zero
        DEBUG_SUBTRACT_INT64(DEBUG_ENGINE, "remaining_qty (apply_fill_ops) = ", order_it->quantity, fill.quantity);
        order_it->quantity = fill.quantity >= order_it->quantity ? 0 : order_it->quantity - fill.quantity;
        DEBUG_ENGINE("Order ID={} new qty={}", fill.makerOrderId, order_it->quantity);

        if (order_it->quantity == 0)
        {
            DEBUG_ENGINE("Order ID={} fully filled, removing from book", fill.makerOrderId);

            if (side == Order::Side::Buy)
                cancel_order_on_side(bids_, side, price, order_it);
            else
                cancel_order_on_side(asks_, side, price, order_it);

            id_lookup_.erase(it);

            // Level is now empty, publish LevelAgg with 0
            bus_(current_tick_, next_seq_++, E_LevelAgg{side, price, 0});
        }
        else
        {

            // Level still exists, publish new quantity
            if (side == Order::Side::Buy)
            {
                auto &orders_at_price = bids_.get_orders_at_price(price);
                bus_(current_tick_, next_seq_++, E_LevelAgg{side, price, std::accumulate(orders_at_price.begin(), orders_at_price.end(), int64_t(0), [](int64_t sum, const Order &o)
                                                                                         { return sum + o.quantity; })});
            }
            else
            {
                auto &orders_at_price = asks_.get_orders_at_price(price);
                bus_(current_tick_, next_seq_++, E_LevelAgg{side, price, std::accumulate(orders_at_price.begin(), orders_at_price.end(), int64_t(0), [](int64_t sum, const Order &o)
                                                                                         { return sum + o.quantity; })});
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
    if (orders.empty())
        return;             // nothing to cancel
    auto id = order_it->id; // capture ID before erasing
    orders.erase(order_it);
    if (book_side.empty_at_price(price))
        book_side.remove_price_level(price);
    bus_(current_tick_, next_seq_++, E_OrderRemoved{id});
}
