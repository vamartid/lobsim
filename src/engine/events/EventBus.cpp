// event_bus.cpp
#include "engine/events/EventBus.h"

struct EventBus::Endpoint
{
    std::unique_ptr<SPSC<Event>> q;
    Callback cb;
    Backpressure bp{Backpressure::SpinYield};
    std::atomic<bool> run{false};
    std::thread th;
};

EventBus::EventBus(size_t ring_pow2) : ring_pow2_(ring_pow2) {}

EventBus::~EventBus()
{
    stop_all();
}

size_t EventBus::add_listener(Callback cb, Backpressure bp)
{
    auto ep = std::make_unique<Endpoint>();
    ep->q = std::make_unique<SPSC<Event>>(ring_pow2_);
    ep->cb = std::move(cb);
    ep->bp = bp;
    ep->run.store(true, std::memory_order_relaxed);
    ep->th = std::thread([p = ep.get()]
                         {
        Event ev{};
        while (p->run.load(std::memory_order_relaxed)) {
            if (p->q->pop(ev)) {
                p->cb(ev);
            } else {
                std::this_thread::yield();
            }
        }
        // drain remaining
        while (p->q->pop(ev)) {
            p->cb(ev);
        } });
    listeners_.push_back(std::move(ep));
    return listeners_.size() - 1;
}

void EventBus::remove_listener(size_t h)
{
    if (h >= listeners_.size() || !listeners_[h])
        return;
    auto &ep = listeners_[h];
    ep->run.store(false, std::memory_order_relaxed);
    if (ep->th.joinable())
        ep->th.join();
    listeners_[h].reset();
}

void EventBus::stop_all()
{
    for (auto &ep : listeners_)
    {
        if (ep)
            ep->run.store(false, std::memory_order_relaxed);
    }
    for (auto &ep : listeners_)
    {
        if (ep && ep->th.joinable())
            ep->th.join();
    }
    listeners_.clear();
}

void EventBus::publish(const Event &e)
{
    for (auto &ep : listeners_)
    {
        if (ep)
            push_one(*ep, e);
    }
}

// helper to construct and publish in one call
template <typename Payload>
void EventBus::operator()(Ticks ts, Seq seq, const Payload &payload)
{
    publish(Event::make(ts, seq, payload));
}

void EventBus::push_one(Endpoint &ep, const Event &e)
{
    switch (ep.bp)
    {
    case Backpressure::Drop:
        ep.q->push(e);
        break;
    case Backpressure::Block:
        while (!ep.q->push(e))
        {
        }
        break;
    case Backpressure::SpinYield:
    {
        int spins = 0;
        while (!ep.q->push(e))
        {
            if (++spins % 64 == 0)
                std::this_thread::yield();
        }
        break;
    }
    }
}

template void EventBus::operator()<E_Fill>(uint64_t, uint64_t, E_Fill const &);
template void EventBus::operator()<E_OrderAdded>(uint64_t, uint64_t, E_OrderAdded const &);
template void EventBus::operator()<E_OrderRemoved>(uint64_t, uint64_t, E_OrderRemoved const &);
template void EventBus::operator()<E_LevelAgg>(uint64_t, uint64_t, E_LevelAgg const &);