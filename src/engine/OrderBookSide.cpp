// OrderBookSide.cpp
#include "engine/OrderBookSide.h"
#include "utils/Comparator.h"

#include <iostream>

template <typename Compare>
OrderBookSide<Compare>::OrderBookSide(Side side)
    : side_(side), price_levels_(Compare{}) // Default construct comparator
{
}

// Engine does not use add_order methods since they do linear search
template <typename Compare>
void OrderBookSide<Compare>::add_order(const Order &order)
{
    price_levels_[order.price].push_back(order);
}

// Engine does not use add_order methods since they do linear search
template <typename Compare>
void OrderBookSide<Compare>::remove_order(uint64_t order_id)
{
    for (auto it = price_levels_.begin(); it != price_levels_.end();)
    {
        auto &orders = it->second;
        for (auto order_it = orders.begin(); order_it != orders.end(); ++order_it)
        {
            if (order_it->id == order_id)
            {
                orders.erase(order_it);
                if (orders.empty())
                    it = price_levels_.erase(it);
                else
                    ++it;
                return;
            }
        }
        ++it;
    }
}

template <typename Compare>
std::optional<double> OrderBookSide<Compare>::best_price() const
{
    if (price_levels_.empty())
        return std::nullopt;
    return price_levels_.begin()->first;
}

template <typename Compare>
std::list<Order> &OrderBookSide<Compare>::best_orders()
{
    static std::list<Order> empty;
    if (price_levels_.empty())
        return empty;
    return price_levels_.begin()->second;
}

template <typename Compare>
const std::list<Order> &OrderBookSide<Compare>::best_orders() const
{
    static const std::list<Order> empty;
    if (price_levels_.empty())
        return empty;
    return price_levels_.begin()->second;
}

template <typename Compare>
void OrderBookSide<Compare>::print(const std::string &side_name) const
{
    std::cout << "=== " << side_name << " Side ===\n";
    for (const auto &[price, queue] : price_levels_)
    {
        for (const auto &order : queue)
            std::cout << order.to_string() << "\n";
    }
}

// Explicit instantiation of template class
template class OrderBookSide<utils::comparator::Ascending>;
template class OrderBookSide<utils::comparator::Descending>;