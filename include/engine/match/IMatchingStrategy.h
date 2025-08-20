#pragma once
#include "core/Order.h"
#include "engine/side/IOrderBookSideView.h"
#include "engine/match/FillOp.h"
#include "engine/match/MatchResult.h"
#include <vector>

class IMatchingStrategy
{
public:
    virtual ~IMatchingStrategy() = default;

    /**
     * @brief Attempt to match an incoming order against the opposite side.
     *
     * @param incoming      The incoming order (mutable: remaining qty updated).
     * @param oppositeSide  Read-only abstract view of the opposite side.
     * @param outFills      Buffer where strategy appends FillOps.
     * @return              MatchResult with aggregate status info.
     */
    virtual MatchResult match(
        Order &incoming,
        const IOrderBookSideView &opposite,
        std::vector<FillOp> &outFills) = 0;
};
