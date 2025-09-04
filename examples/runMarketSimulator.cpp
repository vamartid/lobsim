#include "simulator/MarketSimulator.h"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    MarketSimulator simulator;

    std::cout << "Starting Market Simulator...\n";
    simulator.start();

    simulator.enable_live_view(true); // now safe to enable after start

    // Run the simulation for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(20));

    std::cout << "Stopping Market Simulator...\n";
    simulator.enable_live_view(false); // detach listeners first
    simulator.stop();
    std::cout << "\nSimulation ended.\n";
    return 0;
}
