#pragma once

#include "core/Order.h"

#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <atomic>
#include <array>

class OrderRenderer; // forward declaration

class OrderTracker
{
public:
    static constexpr size_t BUFFER_SIZE = 8192;

    OrderTracker() = default;

    void enable(bool on);
    bool is_enabled() const;

    void add_order(const Order &order);
    void add_order(const Order &order, uint16_t feeder_id);
    void update_order(const Order &order);
    void remove_order(uint64_t order_id);

    std::vector<Order> get_snapshot() const;
    bool has_updates() const;

    // Live rendering
    void render_live_view() const;

private:
    friend class OrderRenderer;

    mutable std::shared_mutex mutex_;
    std::unordered_map<uint64_t, Order> orders_;
    std::unordered_map<uint64_t, uint16_t> order_feeder_map_;
    mutable std::atomic<bool> updated_{false};
    std::atomic<bool> enabled_{false};

    // Thread-local buffer for rendering
    static thread_local std::array<char, BUFFER_SIZE> final_buffer_;

    // Internal builder (delegated to OrderRenderer)
    void build_side_by_side_view(std::array<char, BUFFER_SIZE> &buf, size_t &pos) const;
};
