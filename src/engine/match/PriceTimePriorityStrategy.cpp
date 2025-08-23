#include "engine/match/PriceTimePriorityStrategy.h"
#include <algorithm> // std::min

// Helper: price acceptance based on side/market
static inline bool price_ok(const Order &incoming, double levelPrice)
{
    if (incoming.isMarket())
        return true;
    return incoming.isBuy() ? (incoming.price >= levelPrice)
                            : (incoming.price <= levelPrice);
}

MatchResult PriceTimePriorityStrategy::match(
    Order &incoming,
    const IOrderBookSideView &opposite_side,
    std::vector<FillOp> &out)
{
    MatchResult result{};
    if (incoming.quantity == 0)
        return result;

    // --- FOK pre-check: ensure full fillability before emitting any FillOps
    if (incoming.isFOK())
    {
        uint64_t canFill = 0;
        opposite_side.for_each_level([&](const PriceLevelView &lvl)
                                     {
            if (!price_ok(incoming, lvl.price) || canFill >= incoming.quantity) return;
            opposite_side.for_each_order_at_price(lvl.price, [&](const Order& resting){
                if (!price_ok(incoming, lvl.price) || canFill >= incoming.quantity) return;
                canFill += resting.quantity;
            }); });
        if (canFill < incoming.quantity)
        {
            result.allOrNoneFailed = true;
            return result; // no FillOps emitted
        }
    }

    // --- Produce FillOps in FIFO order until filled or price no longer ok
    uint32_t remaining = incoming.quantity;

    opposite_side.for_each_level([&](const PriceLevelView &lvl)
                                 {
        if (remaining == 0) return;
        if (!price_ok(incoming, lvl.price)) return;

        opposite_side.for_each_order_at_price(lvl.price, [&](const Order& resting) {
            if (remaining == 0) return;

            const uint32_t exec = static_cast<uint32_t>(std::min<uint64_t>(remaining, resting.quantity));
            if (exec == 0) return;

            out.push_back(FillOp{
                /*makerOrderId*/ resting.id,
                /*qty*/          exec,
                /*price*/        lvl.price
            });

            remaining -= exec;
        }); });

    // Strategy can either leave mutation to engine or reflect planned fills:
    const uint32_t filled = incoming.quantity - remaining;
    result.filledQty = filled;

#ifdef PRICE_TIME_PRIORITY_DEBUG
    incoming.quantity = remaining; // Only in debug/test builds
#endif                             // reflect planned fills on taker

    // IOC is handled by engine: if remaining > 0 and IOC, engine cancels remainder.
    return result;
}
