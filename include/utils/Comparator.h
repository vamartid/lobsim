namespace utils::comparator
{
    // Define comparator type alias for bids and asks
    struct Descending
    {
        bool operator()(double a, double b) const { return a > b; }
    };

    struct Ascending
    {
        bool operator()(double a, double b) const { return a < b; }
    };
}
