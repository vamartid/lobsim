#pragma once

#include "core/Order.h"

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <vector>

class OrderTracker
{
public:
    void enable(bool on);
    bool is_enabled() const;

    void render_live_view();

    // Updated signature
    void add_order(const Order &order);
    void add_order(const Order &order, uint16_t feeder_id);
    void update_order(const Order &order);
    void remove_order(uint64_t order_id);

    std::vector<Order> get_snapshot() const;
    bool has_updates();

private:
    std::unordered_map<uint64_t, Order> orders_;
    std::unordered_map<uint64_t, uint16_t> order_feeder_map_; // <order_id, feeder_id>

    mutable std::shared_mutex mutex_;
    std::atomic<bool> enabled_{false};
    std::atomic<bool> updated_{false};

    // --- Rendering helpers ---
    void render_order_summary(std::ostream &os) const;
    void render_top_feeders(std::ostream &os, size_t top_n = 5) const;
    void render_orders_by_feeder(std::ostream &os) const;
    // void render_order_book_ladder(std::ostream &os, size_t depth = 10) const;
    // void render_trade_blotter(std::ostream &os, size_t count = 20) const;
    // void render_feeder_latency(std::ostream &os) const;
    void render_order_flow_imbalance(std::ostream &os) const;
    // void render_position_summary(std::ostream &os) const;
};
