#pragma once
#include <memory>
#include <vector>
#include "engine/views/IView.h"

class Dashboard
{
public:
    void add_view(std::unique_ptr<IView> v)
    {
        views_.push_back(std::move(v));
    }

    size_t render_all(std::ostream &os)
    {
        size_t total_lines = 0;
        for (auto &v : views_)
        {
            total_lines += v->render(os);
            os << "\n"; // spacing between views
            total_lines++;
        }
        return total_lines;
    }

private:
    std::vector<std::unique_ptr<IView>> views_;
};
