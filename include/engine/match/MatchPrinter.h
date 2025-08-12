#pragma once

#include "core/Order.h"

namespace match_printer
{
    void print_match(const Order &incoming, const Order &matched, uint64_t quantity);
}
