#pragma once
#include <string>
#include <ostream>

class IView
{
public:
    virtual ~IView() = default;

    // Render into provided ostream. Return how many lines were printed.
    virtual size_t render(std::ostream &os) = 0;

    // Optional: handle key input
    virtual void on_key(char key) {}
};
