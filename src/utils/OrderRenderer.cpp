#include "utils/OrderRenderer.h"
#include <charconv>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cmath>

void OrderRenderer::append_text(std::array<char, OrderTracker::BUFFER_SIZE> &buf, size_t &pos, const char *txt)
{
    while (*txt && pos < buf.size())
        buf[pos++] = *txt++;
}

template <typename T>
void OrderRenderer::append_number(std::array<char, OrderTracker::BUFFER_SIZE> &buf, size_t &pos, T number)
{
    auto [ptr, ec] = std::to_chars(buf.data() + pos, buf.data() + buf.size(), number);
    if (ec == std::errc())
        pos = ptr - buf.data();
}

template <typename T>
void OrderRenderer::pad_right(std::array<char, OrderTracker::BUFFER_SIZE> &buf, size_t &pos, T value, size_t width)
{
    size_t start = pos;

    if constexpr (std::is_arithmetic_v<T>)
    {
        char temp[32];
        auto [ptr, ec] = std::to_chars(temp, temp + sizeof(temp), value);
        size_t len = ptr - temp;
        if (pos + len > buf.size())
            len = buf.size() - pos;
        memcpy(buf.data() + pos, temp, len);
        pos += len;
    }
    else if constexpr (std::is_convertible_v<T, std::string_view>)
    {
        std::string_view sv = value;
        size_t len = std::min(sv.size(), buf.size() - pos);
        memcpy(buf.data() + pos, sv.data(), len);
        pos += len;
    }

    while (pos - start < width && pos < buf.size())
        buf[pos++] = ' ';
}

// ============================================================
// Build side-by-side
// ============================================================
void OrderRenderer::build_side_by_side_view(
    const OrderTracker &tracker,
    std::array<char, OrderTracker::BUFFER_SIZE> &buf,
    size_t &pos)
{
    size_t buy_count = 0, sell_count = 0;
    double buy_qty = 0, sell_qty = 0;
    std::unordered_map<uint16_t, size_t> feeder_volumes, feeder_counts;

    for (const auto &[id, order] : tracker.orders_)
    {
        if (order.isBuy())
        {
            ++buy_count;
            buy_qty += order.quantity;
        }
        else
        {
            ++sell_count;
            sell_qty += order.quantity;
        }

        auto it = tracker.order_feeder_map_.find(id);
        if (it != tracker.order_feeder_map_.end())
        {
            feeder_volumes[it->second] += order.quantity;
            ++feeder_counts[it->second];
        }
    }

    // Sort feeders by volume
    std::vector<std::pair<uint16_t, size_t>> feeders(feeder_volumes.begin(), feeder_volumes.end());
    std::sort(feeders.begin(), feeders.end(), [](auto &a, auto &b)
              { return a.second > b.second; });

    // Sort feeders by count
    std::vector<std::pair<uint16_t, size_t>> feeders_count(feeder_counts.begin(), feeder_counts.end());
    std::sort(feeders_count.begin(), feeders_count.end(), [](auto &a, auto &b)
              { return a.second > b.second; });

    // --- REVISED LOGIC STARTS HERE ---

    // Use a temporary buffer to build each line and store string_views
    std::vector<std::string> col1_lines, col2_lines, col3_lines;
    char temp_buf[256];

    // Column 1: Order Summary
    // Column 1: Order Summary
    // Column 1: Order Summary
    std::snprintf(temp_buf, sizeof(temp_buf), "=== Order Summary ===");
    col1_lines.emplace_back(temp_buf);

    // Use consistent format specifiers for the header
    std::snprintf(temp_buf, sizeof(temp_buf), "%-10s %-10s %-10s", "Side", "Count", "TotalQty");
    col1_lines.emplace_back(temp_buf);
    col1_lines.emplace_back("");

    // Use the same format specifiers for the data rows
    std::snprintf(temp_buf, sizeof(temp_buf), "%-10s %-10zu %-10lld", "BUY", buy_count, (long long)buy_qty);
    col1_lines.emplace_back(temp_buf);

    std::snprintf(temp_buf, sizeof(temp_buf), "%-10s %-10zu %-10lld", "SELL", sell_count, (long long)sell_qty);
    col1_lines.emplace_back(temp_buf);
    // Column 2: Top Feeders
    std::snprintf(temp_buf, sizeof(temp_buf), "=== Top Feeders ===");
    col2_lines.emplace_back(temp_buf);
    std::snprintf(temp_buf, sizeof(temp_buf), "Feeder : Qty");
    col2_lines.emplace_back(temp_buf);
    col2_lines.emplace_back("");
    for (size_t i = 0; i < std::min<size_t>(feeders.size(), 5); ++i)
    {
        std::snprintf(temp_buf, sizeof(temp_buf), "Feeder %-3u: %-10zu", feeders[i].first, feeders[i].second);
        col2_lines.emplace_back(temp_buf);
    }

    // Column 3: Orders by Feeder
    std::snprintf(temp_buf, sizeof(temp_buf), "=== Orders by Feeder ===");
    col3_lines.emplace_back(temp_buf);
    std::snprintf(temp_buf, sizeof(temp_buf), "Feeder : Count");
    col3_lines.emplace_back(temp_buf);
    col3_lines.emplace_back("");
    for (size_t i = 0; i < std::min<size_t>(feeders_count.size(), 5); ++i)
    {
        std::snprintf(temp_buf, sizeof(temp_buf), "Feeder %-3u: %-10zu", feeders_count[i].first, feeders_count[i].second);
        col3_lines.emplace_back(temp_buf);
    }

    // Merge columns into the final buffer
    const size_t col_width = 34;
    size_t rows = std::max({col1_lines.size(), col2_lines.size(), col3_lines.size()});
    for (size_t r = 0; r < rows; ++r)
    {
        // Column 1
        if (r < col1_lines.size())
            pad_right(buf, pos, col1_lines[r], col_width);
        else
            pad_right(buf, pos, "", col_width);
        append_text(buf, pos, " | ");

        // Column 2
        if (r < col2_lines.size())
            pad_right(buf, pos, col2_lines[r], col_width);
        else
            pad_right(buf, pos, "", col_width);
        append_text(buf, pos, " | ");

        // Column 3
        if (r < col3_lines.size())
            pad_right(buf, pos, col3_lines[r], col_width);
        else
            pad_right(buf, pos, "", col_width);

        if (pos < buf.size())
            buf[pos++] = '\n';
    }

    // Order flow imbalance
    double imbalance = (buy_qty + sell_qty > 0) ? (buy_qty - sell_qty) / (buy_qty + sell_qty) * 100.0 : 0.0;
    append_text(buf, pos, "------------------------------------------------\n");
    append_text(buf, pos, "=== Order Flow Imbalance ===       | Imbalance: ");
    append_number(buf, pos, static_cast<int64_t>(std::round(imbalance)));
    append_text(buf, pos, "%\n\n");
}

// Explicit template instantiations
template void OrderRenderer::append_number<int>(std::array<char, OrderTracker::BUFFER_SIZE> &, size_t &, int);
template void OrderRenderer::append_number<long long>(std::array<char, OrderTracker::BUFFER_SIZE> &, size_t &, long long);
template void OrderRenderer::append_number<size_t>(std::array<char, OrderTracker::BUFFER_SIZE> &, size_t &, size_t);

template void OrderRenderer::pad_right<std::string_view>(std::array<char, OrderTracker::BUFFER_SIZE> &, size_t &, std::string_view, size_t);
template void OrderRenderer::pad_right<const char *>(std::array<char, OrderTracker::BUFFER_SIZE> &, size_t &, const char *, size_t);
