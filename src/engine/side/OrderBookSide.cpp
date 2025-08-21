#include "engine/side/OrderBookSide.h"
#include "core/Order.h"

template <typename Compare>
void OrderBookSide<Compare>::add_order(const Order &o)
{
    price_levels_[o.price].push_back(o);
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

// Explicit instantiations
template class OrderBookSide<utils::comparator::Ascending>;
template class OrderBookSide<utils::comparator::Descending>;