#include <memory>

namespace utils::general
{
    constexpr uint64_t FEEDER_ID_BITS = 16;
    constexpr uint64_t COUNTER_BITS = 64 - FEEDER_ID_BITS;

    inline uint64_t encode_order_id(uint16_t feeder_id, uint64_t counter)
    {
        return (static_cast<uint64_t>(feeder_id) << COUNTER_BITS) | counter;
    }
}
