#include <iostream>
#include <atomic>
#include <thread>
#include <cmath>

#include <Lunaris/event_pool.h>

using namespace Lunaris::EventPool;

int main() {
    constexpr auto fcn_dummy = [] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); throw std::runtime_error("Random error"); };
    std::atomic<bool> caused_exception{false};
    AsyncEventPool<void> event(fcn_dummy, [&caused_exception](const std::exception&){ 
        std::cout << "# Got exception!" << std::endl;
        caused_exception = true;
    }, 1);

    std::cout << "Attempting exception" << std::endl;

    event.post();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    if (!caused_exception) {
        std::cout << "Exception did not go through." << std::endl;
        return 1;
    }

    std::cout << "Good." << std::endl;

    return 0;
}