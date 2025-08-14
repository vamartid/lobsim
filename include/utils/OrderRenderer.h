#pragma once
#include "OrderTracker.h"
#include <array>
#include <cstddef>
#include <string_view>

class OrderRenderer
{
public:
    static void build_side_by_side_view(
        const OrderTracker &tracker,
        std::array<char, OrderTracker::BUFFER_SIZE> &buf,
        size_t &pos);

private:
    static void append_text(std::array<char, OrderTracker::BUFFER_SIZE> &buf, size_t &pos, const char *txt);

    template <typename T>
    static void append_number(std::array<char, OrderTracker::BUFFER_SIZE> &buf, size_t &pos, T number);

    template <typename T>
    static void pad_right(std::array<char, OrderTracker::BUFFER_SIZE> &buf, size_t &pos, T value, size_t width);
};
