#pragma once
#include <cstdint>

struct MatchResult
{
    uint32_t filledQty = 0;       // total filled qty for incoming
    bool allOrNoneFailed = false; // true if FOK/AON failed
};

// --- MatchResult ---
template <>
struct std::formatter<MatchResult> : std::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const MatchResult &m, FormatContext &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("filledQty:{} allOrNoneFailed:{}",
                        m.filledQty,
                        m.allOrNoneFailed ? "true" : "false"),
            ctx);
    }
};
