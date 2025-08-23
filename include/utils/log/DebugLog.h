#pragma once

#include <iostream>
#include <format>
#include <string>
#include <cstdint>

// --- Top-Level Debug Switch ---
#ifndef ENABLE_DEBUG
#define ENABLE_DEBUG 0
#endif

// --- Category-Specific Switches ---
// Consistent naming convention
#ifndef ENABLE_DEBUG_ENGINE
#define ENABLE_DEBUG_ENGINE 0
#endif

#ifndef ENABLE_DEBUG_ORDERBOOKSIDE
#define ENABLE_DEBUG_ORDERBOOKSIDE 0
#endif

#ifndef ENABLE_DEBUG_MATCHER
#define ENABLE_DEBUG_MATCHER 0
#endif

// --- Core Logging Implementation ---
#if ENABLE_DEBUG
#define DEBUG_LOG_IMPL(prefix, fmt, ...) \
    std::cout << "[" << prefix << "] " << std::format(fmt, ##__VA_ARGS__) << "\n"
#else
#define DEBUG_LOG_IMPL(prefix, fmt, ...) \
    do                                   \
    {                                    \
    } while (0)
#endif

// --- Category Debug Macros ---
// Use consistent macro names that match the ENABLE flags.
#if ENABLE_DEBUG && ENABLE_DEBUG_ENGINE
#define DEBUG_ENGINE(fmt, ...) DEBUG_LOG_IMPL("ENGINE", fmt, ##__VA_ARGS__)
#else
#define DEBUG_ENGINE(fmt, ...) \
    do                         \
    {                          \
    } while (0)
#endif

#if ENABLE_DEBUG && ENABLE_DEBUG_ORDERBOOKSIDE
// Here, we define the macro DEBUG_ORDERBOOKSIDE
#define DEBUG_ORDERBOOKSIDE(fmt, ...) DEBUG_LOG_IMPL("ORDERBOOKSIDE", fmt, ##__VA_ARGS__)
#else
#define DEBUG_ORDERBOOKSIDE(fmt, ...) \
    do                                \
    {                                 \
    } while (0)
#endif

#if ENABLE_DEBUG && ENABLE_DEBUG_MATCHER
#define DEBUG_MATCHER(fmt, ...) DEBUG_LOG_IMPL("MATCHER", fmt, ##__VA_ARGS__)
#else
#define DEBUG_MATCHER(fmt, ...) \
    do                          \
    {                           \
    } while (0)
#endif

// --- Universal Arithmetic Debug Macros ---
// These macros are fine as they are, they just need to be used with the
// correct category macro name.
#define DEBUG_SUBTRACT_INT64(category_macro, msg, a, b)                                 \
    do                                                                                  \
    {                                                                                   \
        category_macro("{}{}", msg, static_cast<int64_t>(a) - static_cast<int64_t>(b)); \
    } while (0)

#define DEBUG_ADD_INT64(category_macro, msg, a, b)                                      \
    do                                                                                  \
    {                                                                                   \
        category_macro("{}{}", msg, static_cast<int64_t>(a) + static_cast<int64_t>(b)); \
    } while (0)

#define DEBUG_MUL_INT64(category_macro, msg, a, b)                                      \
    do                                                                                  \
    {                                                                                   \
        category_macro("{}{}", msg, static_cast<int64_t>(a) * static_cast<int64_t>(b)); \
    } while (0)

#define DEBUG_DIV_INT64(category_macro, msg, a, b)                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        if (static_cast<int64_t>(b) == 0)                                                                              \
        {                                                                                                              \
            category_macro("{} Division by zero (a={}, b={})", msg, static_cast<int64_t>(a), static_cast<int64_t>(b)); \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            category_macro("{}{}", msg, static_cast<int64_t>(a) / static_cast<int64_t>(b));                            \
        }                                                                                                              \
    } while (0)
