// #include "MarketFeeder.h"
// #include "ThreadSafeQueue.h"

// #include <iostream>
// #include <format>

// int main()
// {
//     ThreadSafeQueue<Order> queue;
//     MarketFeeder feeder(queue);

//     feeder.start();

//     for (int i = 0; i < 10; ++i)
//     {
//         Order order = queue.wait_and_pop();
//         std::cout << std::format("{}", order.to_string()) << '\n';
//     }

//     feeder.stop();
//     return 0;
// }
// #include <iostream>
// #include "engine/OrderBookEngine.h"
// #include "core/Order.h" // Assuming Order, Side defined here

// int main()
// {
//     OrderBookEngine ob;

//     ob.add_order(Order{1, Side::Buy, 100.50, 10, 1000000});
//     ob.add_order(Order{2, Side::Buy, 101.00, 5, 1000001});

//     ob.add_order(Order{3, Side::Sell, 102.00, 7, 1000002});
//     ob.add_order(Order{4, Side::Sell, 101.50, 12, 1000003});

//     ob.print();

//     return 0;
// }

// #include "simulator/MarketSimulator.h"

// #include <iostream>
// #include <chrono>
// #include <thread>

// int main()
// {
//     MarketSimulator simulator;

//     std::cout << "Starting Market Simulator...\n";
//     simulator.start();

//     // Run the simulation for 10 seconds
//     std::this_thread::sleep_for(std::chrono::seconds(10));

//     std::cout << "Stopping Market Simulator...\n";
//     simulator.stop();

//     std::cout << "\nSimulation ended.\n";

//     return 0;
// }
// #include "simulator/MarketSimulator.h"

// #include <iostream>
// #include <chrono>
// #include <thread>

// int main()
// {
//     MarketSimulator simulator;

//     simulator.enable_live_view(true); // Enable live view before starting

//     std::cout << "Starting Market Simulator...\n";
//     simulator.start();

//     // Run the simulation for 10 seconds
//     std::this_thread::sleep_for(std::chrono::seconds(10));

//     std::cout << "Stopping Market Simulator...\n";
//     simulator.stop();

//     simulator.enable_live_view(false); // Optionally disable live view after stopping

//     std::cout << "\nSimulation ended.\n";

//     return 0;
// }

#include "simulator/MarketSimulator.h"
#include "engine/listeners/OrderBookView.h"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    MarketSimulator simulator;

    // Create a live order book view and subscribe to events
    auto view = std::make_shared<OrderBookView>();
    size_t handle = simulator.add_listener([view](const Event &e)
                                           { view->on_event(e); });

    std::cout << "Starting Market Simulator...\n";
    simulator.start();

    simulator.enable_live_view(true); // now safe to enable after start

    // Run the simulation for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Stopping Market Simulator...\n";
    simulator.stop();

    simulator.enable_live_view(false);

    std::cout << "\nSimulation ended.\n";
    return 0;
}
