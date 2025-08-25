// events.h
#pragma once
#include "core/Order.h"
#include <cstdint>
#include <format>

using Seq = uint32_t;
using Ticks = uint32_t;

// We keep payload POD only
// structs uint64_t, int64_t, double and Order::Side enum class
// so union inside Event is safe and well-defined
// if we have no POD switch to std::variant
struct E_OrderAdded
{
    uint64_t id;
    Order::Side side;
    double px;
    int64_t qty;
};

struct E_OrderUpdated
{
    uint64_t id;
    double px;
    int64_t qty;
}; // after partial fill/change

struct E_OrderRemoved
{
    uint64_t id;
};

struct E_Fill
{
    uint64_t makerId;
    uint64_t takerId;
    double px;
    int64_t qty;
};

struct E_LevelAgg
{
    Order::Side side;
    double px;
    int64_t aggQty;
};

enum class EventType : uint8_t
{
    OrderAdded,
    OrderUpdated,
    OrderRemoved,
    Fill,
    LevelAgg
};

struct Event
{
    EventType type;
    Seq seq;
    Ticks ts;
    union
    {
        E_OrderAdded added;
        E_OrderUpdated updated;
        E_OrderRemoved removed;
        E_Fill fill;
        E_LevelAgg level;
    } d;
    static Event make(Ticks ts, Seq s, const E_OrderAdded &x)
    {
        Event e{EventType::OrderAdded, s, ts};
        e.d.added = x;
        return e;
    }
    static Event make(Ticks ts, Seq s, const E_OrderUpdated &x)
    {
        Event e{EventType::OrderUpdated, s, ts};
        e.d.updated = x;
        return e;
    }
    static Event make(Ticks ts, Seq s, const E_OrderRemoved &x)
    {
        Event e{EventType::OrderRemoved, s, ts};
        e.d.removed = x;
        return e;
    }
    static Event make(Ticks ts, Seq s, const E_Fill &x)
    {
        Event e{EventType::Fill, s, ts};
        e.d.fill = x;
        return e;
    }
    static Event make(Ticks ts, Seq s, const E_LevelAgg &x)
    {
        Event e{EventType::LevelAgg, s, ts};
        e.d.level = x;
        return e;
    }
};

// OrderAdded
template <>
struct std::formatter<E_OrderAdded> : std::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const E_OrderAdded &x, FormatContext &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("ID:{} Side:{} Price:{:.2f} Qty:{}",
                        x.id,
                        static_cast<int>(x.side),
                        x.px,
                        x.qty),
            ctx);
    }
};

// OrderUpdated
template <>
struct std::formatter<E_OrderUpdated> : std::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const E_OrderUpdated &x, FormatContext &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("ID:{} Price:{:.2f} Qty:{}", x.id, x.px, x.qty), ctx);
    }
};

// OrderRemoved
template <>
struct std::formatter<E_OrderRemoved> : std::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const E_OrderRemoved &x, FormatContext &ctx) const
    {
        return std::formatter<std::string>::format(std::format("ID:{}", x.id), ctx);
    }
};

// LevelAgg
template <>
struct std::formatter<E_LevelAgg> : std::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const E_LevelAgg &x, FormatContext &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("Side:{} Price:{:.2f} AggQty:{}", static_cast<int>(x.side), x.px, x.aggQty),
            ctx);
    }
};

// Fill
template <>
struct std::formatter<E_Fill> : std::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const E_Fill &x, FormatContext &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("Maker:{} Taker:{} Price:{:.2f} Qty:{}",
                        x.makerId,
                        x.takerId,
                        x.px,
                        x.qty),
            ctx);
    }
};