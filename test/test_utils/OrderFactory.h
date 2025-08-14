#pragma once
#include "core/Order.h"
#include <cstdint>

struct TestOrderFactory
{
    static Order CreateBuy(
        uint64_t orderId = 1,
        double price = 100.0,
        uint32_t qty = 10,
        uint8_t feeder = 0,
        uint32_t ts = 123456789,
        uint8_t controlFlags = 0)
    {
        Order order(orderId, price, qty, Order::Side::Buy, feeder, ts);
        order.controlFlags = controlFlags;
        return order;
    }

    static Order CreateSell(
        uint64_t orderId = 1,
        double price = 100.0,
        uint32_t qty = 10,
        uint8_t feeder = 0,
        uint32_t ts = 123456789,
        uint8_t controlFlags = 0)
    {
        Order order(orderId, price, qty, Order::Side::Sell, feeder, ts);
        order.controlFlags = controlFlags;
        return order;
    }
};
