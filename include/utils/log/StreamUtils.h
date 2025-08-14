#pragma once

#include "utils/log/ConsoleColors.h"

#include <regex>
#include <string>
#include <sstream>
#include <string>
#include <iomanip>
#include <utility>

namespace utils::stream
{
    // Capture output from func() into any ostream
    template <typename Stream, typename Func>
    inline void capture_output(Stream &stream_to_capture, std::ostream &captured, Func func)
    {
        // Save original buffer
        std::streambuf *old_buf = stream_to_capture.rdbuf();

        // Redirect the stream to a temporary buffer
        std::ostringstream buffer;
        stream_to_capture.rdbuf(buffer.rdbuf());

        func(); // run code that writes to the stream

        // Restore the original buffer
        stream_to_capture.rdbuf(old_buf);

        // Copy captured content to provided stream
        captured << buffer.str();
    }

    // Remove ANSI codes from an input stream and write to output stream
    inline void strip_ansi(std::ostream &out, std::istream &in)
    {
        static const std::regex ansi_escape("\x1B\\[[0-9;]*[mK]");
        std::string buffer(
            (std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()); // read entire stream
        out << std::regex_replace(buffer, ansi_escape, "");
    }

    // Stream-based colorize: avoids string creation
    template <typename T>
    inline std::ostream &colorize(std::ostream &os, const T &text, const char *color)
    {
        using namespace utils::console_colors;
        os << color << text << RESET;
        return os;
    }

    template <typename... Streams>
    inline void col(std::ostream &os, size_t column_width, Streams &...streams)
    {
        std::string line_buffers[sizeof...(streams)]; // reuse buffers
        bool has_data;

        do
        {
            has_data = false;
            size_t idx = 0;

            // fold expression to iterate over each stream
            ([&]()
             {
             auto &stream = streams;
             auto &buffer = line_buffers[idx];

             if (std::getline(stream, buffer))
             {
                 has_data = true;
                 os.write(buffer.data(), buffer.size()); // write directly to ostream
                 // pad to column width
                 if (buffer.size() < column_width)
                     os << std::string(column_width - buffer.size(), ' ');
             }
             else
             {
                 os << std::string(column_width, ' ');
             }
             ++idx; }(), ...);

            if (has_data)
                os.put('\n');

        } while (has_data);
    }

} // namespace utils::stream