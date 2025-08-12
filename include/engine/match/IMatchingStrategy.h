#pragma once

#include "core/Order.h"
#include "engine/OrderBookSide.h"
#include "utils/Comparator.h"

#include <list>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>

class IMatchingStrategy
{
public:
    virtual ~IMatchingStrategy() = default;

    virtual void match(
        OrderBookSide<utils::comparator::Descending> &bids,
        OrderBookSide<utils::comparator::Ascending> &asks,
        std::unordered_map<uint64_t, std::tuple<Side, double, std::list<Order>::iterator>> &id_lookup) = 0;
};
