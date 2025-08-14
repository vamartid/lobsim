#pragma once
#include "utils/random/MockRNG.h"
#include "core/Order.h"
#include "test_utils/OrderFactory.h"

#include <vector>
#include <memory>

// Convert a TestOrderFactory parameter set into MockRNG doubles
inline std::vector<double> factory_params_to_mock_rng(
    double price,
    uint32_t qty,
    Order::Side side)
{
    return std::vector<double>{
        static_cast<double>(price),
        static_cast<double>(qty),
        static_cast<double>(side == Order::Side::Buy ? 0.0 : 1.0)};
}

// Aggregate multiple factory parameter sets into one vector
inline std::vector<double> factories_to_mock_values(
    const std::vector<std::tuple<double, uint32_t, Order::Side>> &orders)
{
    std::vector<double> all_values;
    for (const auto &[price, qty, side] : orders)
    {
        auto vals = factory_params_to_mock_rng(price, qty, side);
        all_values.insert(all_values.end(), vals.begin(), vals.end());
    }
    return all_values;
}

// Create a MockRNG directly from factory parameters
inline std::shared_ptr<MockRNG> make_mock_rng_from_factory_params(
    const std::vector<std::tuple<double, uint32_t, Order::Side>> &orders)
{
    return std::make_shared<MockRNG>(factories_to_mock_values(orders));
}
