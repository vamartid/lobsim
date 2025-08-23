// event_bus.h
#pragma once
#include "engine/events/Events.h"
#include "utils/data_structures/spsc.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

enum class Backpressure
{
    Drop,
    Block,
    SpinYield
};

class EventBus
{
public:
    using Callback = std::function<void(const Event &)>;

    explicit EventBus(size_t ring_pow2 = (1u << 12));
    ~EventBus();

    size_t add_listener(Callback cb, Backpressure bp = Backpressure::SpinYield);
    void remove_listener(size_t h);
    void stop_all();

    // single-writer only
    void publish(const Event &e);

    template <typename Payload>
    void operator()(Ticks ts, Seq seq, const Payload &payload);

private:
    struct Endpoint;

    static void push_one(Endpoint &ep, const Event &e);

    size_t ring_pow2_;
    std::vector<std::unique_ptr<Endpoint>> listeners_;
};
