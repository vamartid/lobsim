#pragma once

#include "utils/ConsoleColors.h"

#include <regex>
#include <string>
#include <string_view>
#include <cstring> // for std::strlen
#include <iostream>
#include <iomanip>
namespace utils::string
{
    inline std::string strip_ansi(const std::string &input)
    {
        static const std::regex ansi_escape("\x1B\\[[0-9;]*[mK]");
        return std::regex_replace(input, ansi_escape, "");
    }

    inline std::string color_wrap(std::string_view text, const char *color)
    {
        using namespace utils::console_colors; // bring RESET into scope

        // One allocation, minimal copies
        std::string result;
        result.reserve(std::strlen(color) + text.size() + std::strlen(RESET));
        result.append(color);
        result.append(text);
        result.append(RESET);
        return result;
    }

    inline std::stringstream print_two_columns(std::stringstream left, std::stringstream right, size_t column_width = 30)
    {
        std::string lline, rline;
        std::stringstream output;

        while (true)
        {
            bool lhas = static_cast<bool>(std::getline(left, lline));
            bool rhas = static_cast<bool>(std::getline(right, rline));

            if (!lhas && !rhas)
                break;

            output << std::left << std::setw(column_width)
                   << (lhas ? lline : "")
                   << (rhas ? rline : "")
                   << "\n";
        }

        return output;
    }
}