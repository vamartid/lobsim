#pragma once

#include "utils/random/MockRNG.h"
#include "core/Order.h"

#include <vector>
#include <tuple>

// Convert an Order into the corresponding sequence of doubles expected by MockRNG
std::vector<double> order_to_mock_rng(const Order &order)
{
    return std::vector<double>{
        static_cast<double>(order.price),
        static_cast<double>(order.quantity),
        static_cast<double>(order.side == Side::Buy ? 0.0 : 1.0)};
}

// Aggregate multiple orders into one vector for MockRNG:
inline std::vector<double> orders_to_mock_values(const std::vector<Order> &orders)
{
    std::vector<double> all_values;
    for (const auto &order : orders)
    {
        std::vector<double> vals = order_to_mock_rng(order);
        all_values.insert(all_values.end(), vals.begin(), vals.end());
    }
    return all_values;
}

// Create a MockRNG shared_ptr from orders:
inline std::shared_ptr<MockRNG> make_mock_rng_from_orders(const std::vector<Order> &orders)
{
    return std::make_shared<MockRNG>(orders_to_mock_values(orders));
}
