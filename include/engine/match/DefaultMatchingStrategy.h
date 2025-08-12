#pragma once

#include "engine/match/IMatchingStrategy.h"

class DefaultMatchingStrategy : public IMatchingStrategy
{
public:
    void match(
        OrderBookSide<utils::comparator::Descending> &bids,
        OrderBookSide<utils::comparator::Ascending> &asks,
        std::unordered_map<uint64_t, std::tuple<Side, double, std::list<Order>::iterator>> &id_lookup) override;
};