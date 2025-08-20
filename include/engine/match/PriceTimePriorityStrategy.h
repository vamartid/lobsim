#pragma once

#include "engine/match/IMatchingStrategy.h"

/**
 * @brief FIFO (Price-Time Priority) matcher.
 *        Walks best price → worse; FIFO within each level.
 */
class PriceTimePriorityStrategy : public IMatchingStrategy
{
public:
    MatchResult match(
        Order &incoming,
        const IOrderBookSideView &opposite,
        std::vector<FillOp> &out) override;
};
