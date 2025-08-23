#include "utils/log/Logger.h"
#include <iostream>
#include <format>

void Logger::on_event(const Event &e)
{
    switch (e.type)
    {
    case EventType::OrderAdded:
        std::cout << std::format("[OrderAdded] {}", e.d.added) << "\n";
        break;
    case EventType::OrderUpdated:
        std::cout << std::format("[OrderUpdated] ID:{} Price:{:.2f} Qty:{}",
                                 e.d.updated.id, e.d.updated.px, e.d.updated.qty)
                  << "\n";
        break;
    case EventType::OrderRemoved:
        std::cout << std::format("[OrderRemoved] ID:{}", e.d.removed.id) << "\n";
        break;
    case EventType::Fill:
        std::cout << std::format("[Fill] {}", e.d.fill) << "\n";
        break;
    case EventType::LevelAgg:
        std::cout << std::format("[LevelAgg] {}", e.d.level) << "\n";
        break;
    }
}
