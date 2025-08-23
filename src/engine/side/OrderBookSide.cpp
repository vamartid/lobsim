#include "engine/side/OrderBookSide.h"
#include "core/Order.h"
#include "utils/log/DebugLog.h"

template <typename Compare>
void OrderBookSide<Compare>::add_order(const Order &o)
{
    price_levels_[o.price].push_back(o);
}

template <typename Compare>
typename OrderBookSide<Compare>::OrderList::iterator
OrderBookSide<Compare>::add_order_and_get_iterator(const Order &o)
{
    auto &orders = price_levels_[o.price]; // inserts if not exists
    orders.push_back(o);
    return std::prev(orders.end());
}

template <typename Compare>
typename OrderBookSide<Compare>::OrderList &
OrderBookSide<Compare>::get_orders_at_price(double price)
{
    return price_levels_[price]; // inserts if not exists
}

template <typename Compare>
const typename OrderBookSide<Compare>::OrderList &
OrderBookSide<Compare>::get_orders_at_price(double price) const
{
    static const OrderList empty_list{}; // safe empty reference

    auto it = price_levels_.find(price);
    if (it != price_levels_.end())
        return it->second;
    return empty_list;
}

template <typename Compare>
std::optional<double> OrderBookSide<Compare>::best_price() const
{
    if (price_levels_.empty())
        return std::nullopt;
    return price_levels_.begin()->first;
}

template <typename Compare>
size_t OrderBookSide<Compare>::num_levels() const
{
    return price_levels_.size();
}

template <typename Compare>
void OrderBookSide<Compare>::for_each_level(const std::function<void(const PriceLevelView &)> &fn) const
{
    for (const auto &[price, orders] : price_levels_)
    {
        uint64_t agg_qty = 0;
        for (const auto &o : orders)
        {
            agg_qty += o.quantity; // consider effective_qty(o) later
        }
        fn(PriceLevelView{price, agg_qty});
    }
}

template <typename Compare>
void OrderBookSide<Compare>::for_each_order_at_price(
    double price, const std::function<void(const Order &)> &fn) const
{
    auto it = price_levels_.find(price);
    if (it == price_levels_.end())
        return;
    for (const auto &o : it->second)
    {
        fn(o);
    }
}

template <typename Compare>
void OrderBookSide<Compare>::remove_price_level(double price)
{
    price_levels_.erase(price);
}

template <typename Compare>
bool OrderBookSide<Compare>::empty_at_price(double price) const
{
    auto it = price_levels_.find(price);
    return it == price_levels_.end() || it->second.empty();
}

// ---- side_tag specializations ----

template <>
constexpr Order::Side OrderBookSide<utils::comparator::Descending>::side_tag()
{
    return Order::Side::Buy;
}

template <>
constexpr Order::Side OrderBookSide<utils::comparator::Ascending>::side_tag()
{
    return Order::Side::Sell;
}

// ---- Event Handling ----

template <typename Compare>
inline void OrderBookSide<Compare>::on_event(const Event &e)
{
    switch (e.type)
    {
    case EventType::OrderAdded:
        handle_order_added(e.d.added);
        break;
    case EventType::OrderUpdated:
        handle_order_updated(e.d.updated);
        break;
    case EventType::OrderRemoved:
        handle_order_removed(e.d.removed.id);
        break;
    case EventType::Fill:
        handle_fill(e.d.fill);
        break;
    default:
        break; // LevelAgg ignored
    }
}

// ---- Handlers (read-only in Option A) ----

template <typename Compare>
inline void OrderBookSide<Compare>::handle_order_added(const E_OrderAdded &e)
{
    if (e.side != side_tag())
        return;
    // Instead of inserting into the book, just log or update metrics
    DEBUG_ORDERBOOKSIDE("Listener saw OrderAdded: {}", e);
    // Example: track total notional or order count for this side
    // total_qty_ += e.qty;
}

template <typename Compare>
inline void OrderBookSide<Compare>::handle_order_updated(const E_OrderUpdated &e)
{
    DEBUG_ORDERBOOKSIDE("Listener saw OrderUpdated: {}", e);
    // Example: could update metrics like "avg price" or "total notional"
}

template <typename Compare>
inline void OrderBookSide<Compare>::handle_order_removed(uint64_t order_id)
{
    DEBUG_ORDERBOOKSIDE("Listener saw OrderRemoved: id={}", order_id);
    // Example: adjust counters, metrics, or visualization
}

template <typename Compare>
inline void OrderBookSide<Compare>::handle_fill(const E_Fill &f)
{
    DEBUG_ORDERBOOKSIDE("Listener saw Fill: {}", f);
    // // Example: update traded volume / notional
    // traded_volume_ += f.qty;
    // traded_notional_ += f.qty * f.price;
}

// Explicit instantiations
template class OrderBookSide<utils::comparator::Ascending>;
template class OrderBookSide<utils::comparator::Descending>;