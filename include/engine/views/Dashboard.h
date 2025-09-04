#pragma once
#include "engine/views/IView.h"
#include <memory>
#include <vector>

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
            if (v->visible())
            {
                total_lines += v->render(os);
                os << "\n"; // spacing between views
                total_lines++;
            }
        }
        return total_lines;
    }

    void toggle_view(size_t index)
    {
        if (index < views_.size())
            views_[index]->set_visible(!views_[index]->visible());
    }

    void set_view_visible(size_t index, bool visible)
    {
        if (index < views_.size())
            views_[index]->set_visible(visible);
    }

private:
    std::vector<std::unique_ptr<IView>> views_;
};
