// Define the color codes as macros for easy use
#define RED_CODE "\033[0;31m"
#define GREEN_CODE "\033[0;32m"
#define CYAN_CODE "\033[0;36m"
#define RESET_CODE "\033[0m"

namespace utils::console_colors
{
    inline constexpr const char *RED = RED_CODE;
    inline constexpr const char *GREEN = GREEN_CODE;
    inline constexpr const char *CYAN = CYAN_CODE;
    inline constexpr const char *RESET = RESET_CODE;
}
