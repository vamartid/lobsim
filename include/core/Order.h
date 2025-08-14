#pragma once

#include <cstdint>
#include <type_traits>
#include <assert.h>
#include <string>
#include <sstream>
#include <iomanip>

// Compile specific macros
#if defined(_MSC_VER)                          // MSVC
#define ALIGNED(x) __declspec(align(x))        // MSVC Aligned
#elif defined(__GNUC__) || defined(__clang__)  // GCC/Clang
#define ALIGNED(x) __attribute__((aligned(x))) // GCC/Clang Aligned
#else
#error "Unknown compiler"
#endif

// ---------------------------
// Goal: high-performance, cache-line aligned struct for LOB engine
// Each Order:
// - 64-byte size, 64-byte alignment each Order fits within a single cache line.
// - Prevents false sharing when accessed by multiple threads.
// - Enables SIMD-friendly memory access for high-performance trading engines.
// - `_padding` is used to explicitly fill unused bytes, guaranteeing
//   the struct remains exactly 64 bytes.
// - Queue containers may not automatically preserve alignment; use
//   cache-line-aware structures for multithreaded access.

struct ALIGNED(64) Order
{
public:
    // --- Enums ---
    enum class Side : uint8_t
    {
        Buy = 0,
        Sell = 1,
    };

    enum class Control : uint8_t
    {
        Iceberg = 1 << 0,
        Hidden = 1 << 1,
        Weight = 1 << 2,
        Auction = 1 << 3,
        IOC = 1 << 4,
        FOK = 1 << 5,
        Market = 1 << 6,
        Reserved = 1 << 7,
    };

    // --- Data layout ---
    union
    {
        struct
        {
            uint32_t visibleQty;
            uint32_t hiddenQty;
        };
        uint64_t weight;
        uint64_t auctionMeta;
        uint64_t customData;
    } extra;                 // 8
    uint64_t id;             // 8
    double price;            // 8
    uint64_t sequenceNumber; // 8
    uint32_t quantity;       // 4
    uint32_t timestamp;      // 4
    uint8_t sideFlags;       // 1
    uint8_t controlFlags;    // 1
    uint8_t feederId;        // 1
    uint8_t reserved;        // 1 align to 4-byte boundary
    uint8_t _padding[16];    // instead of 4

public:
    Order() noexcept = default; // allow default construction

    // Order(uint64_t orderId, double price_, uint32_t qty, Side side, uint8_t feeder, uint32_t ts) noexcept
    //     : id(orderId), price(price_), quantity(qty), timestamp(ts),
    //       sideFlags(static_cast<uint8_t>(side) & 1), // directly set the bit
    //       controlFlags(0), feederId(feeder)
    // {
    // }
    Order(uint64_t orderId, double prc, uint32_t qty, Side side, uint8_t feeder, uint32_t ts) noexcept
        : id(orderId), price(prc), quantity(qty), sideFlags(static_cast<uint8_t>(side) & 1),
          feederId(feeder), timestamp(ts),
          controlFlags(0),   // Add this to initialize
          reserved(0),       // Add this to initialize
          sequenceNumber(0), // Add this to initialize
          extra({}),         // Add this to initialize (assuming extra is a struct)
          _padding{}         // Add this to initialize the padding
    {
    }
    // lightweight version for tests we have also log/printer

    std::string to_string() const
    {
        std::ostringstream oss;
        oss << "ID:" << id
            << " Side:" << (side() == Side::Buy ? "BUY" : "SELL")
            << " Price:" << std::fixed << std::setprecision(2) << price
            << " Qty:" << quantity
            << " Time:" << timestamp;
        return oss.str();
    }

    // --- Side ---
    inline Side side() const noexcept
    {
        return (sideFlags & 1) ? Side::Sell : Side::Buy;
    }

    // Friendly setter safe, optional debug checks
    inline void setSide(Side s) noexcept
    {
#ifndef NDEBUG
        // optional check for correctness in debug
        assert(static_cast<uint8_t>(s) <= 1 && "Invalid Side value");
#endif
        sideFlags = (sideFlags & ~1u) | (static_cast<uint8_t>(s) & 1);
    }

    // Internal setter fast, no checks
    inline void setSideRaw(uint8_t s) noexcept
    {
        sideFlags = (sideFlags & ~1u) | (s & 1);
    }

    inline bool isBuy() const noexcept { return side() == Side::Buy; }
    inline bool isSell() const noexcept { return side() == Side::Sell; }

    // --- Generic Control flag ops ---
    inline bool hasFlag(Control f) const noexcept
    {
        return (controlFlags & static_cast<uint8_t>(f)) != 0;
    }
    inline void setFlag(Control f, bool val) noexcept
    {
        if (val)
            controlFlags |= static_cast<uint8_t>(f);
        else
            controlFlags &= ~static_cast<uint8_t>(f);
    }

    // --- Per-flag helpers ---
    inline bool isIceberg() const noexcept { return hasFlag(Control::Iceberg); }
    inline void setIceberg(bool v) noexcept { setFlag(Control::Iceberg, v); }

    inline bool isHidden() const noexcept { return hasFlag(Control::Hidden); }
    inline void setHidden(bool v) noexcept { setFlag(Control::Hidden, v); }

    inline bool isWeighted() const noexcept { return hasFlag(Control::Weight); }
    inline void setWeighted(bool v) noexcept { setFlag(Control::Weight, v); }

    inline bool isAuction() const noexcept { return hasFlag(Control::Auction); }
    inline void setAuction(bool v) noexcept { setFlag(Control::Auction, v); }

    inline bool isIOC() const noexcept { return hasFlag(Control::IOC); }
    inline void setIOC(bool v) noexcept { setFlag(Control::IOC, v); }

    inline bool isFOK() const noexcept { return hasFlag(Control::FOK); }
    inline void setFOK(bool v) noexcept { setFlag(Control::FOK, v); }

    inline bool isMarket() const noexcept { return hasFlag(Control::Market); }
    inline void setMarket(bool v) noexcept { setFlag(Control::Market, v); }

    inline bool isReservedFlag() const noexcept { return hasFlag(Control::Reserved); }
    inline void setReservedFlag(bool v) noexcept { setFlag(Control::Reserved, v); }
};

static_assert(sizeof(Order) == 64, "Order struct must be 64 bytes");
static_assert(alignof(Order) == 64, "Order must be 64-byte aligned");
// Compile-time guarantees
// static_assert(sizeof(Order) == 64, "Order struct must be 64 bytes");
// static_assert(alignof(Order) == 8, "Order must be 8-byte aligned");
