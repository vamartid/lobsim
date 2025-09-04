#pragma once
#include <string>
#include <ostream>

class IView
{
public:
    virtual ~IView() = default;

    // Render into provided ostream. Return how many lines were printed.
    virtual size_t render(std::ostream &os) = 0;

    void set_visible(bool v) { visible_ = v; }
    bool visible() const { return visible_; }

private:
    bool visible_ = true;
};
