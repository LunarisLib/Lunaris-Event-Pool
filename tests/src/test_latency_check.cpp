#include <iostream>
#include <atomic>
#include <thread>
#include <cmath>

#include <Lunaris/event_pool.h>

using namespace Lunaris::EventPool;

int main() {
    constexpr size_t tests = 10;
    constexpr auto fcn_wait_1s = [] { std::this_thread::sleep_for(std::chrono::seconds(1));};
    constexpr auto fcn_wait_100ms = [] { std::this_thread::sleep_for(std::chrono::milliseconds(100));};
    AsyncEventPool<void>* event = new AsyncEventPool<void>(fcn_wait_100ms, 1);

    std::cout << "Testing 100ms wait 1s send..." << std::endl;

    for(size_t p = 0; p < tests; ++p) {
        event->post();
        fcn_wait_1s();
    }

    auto ev_stats = event->get_threads_stats();
    delete event;

    if (const auto delta = fabs(ev_stats[0].latency_on_get_us_moving_avg - 900000.0); delta > 50000) {
        std::cout << "Latency get took " << ev_stats[0].latency_on_get_us_moving_avg << " (delta=" << delta << "). Too off." << std::endl;        
        return 1;
    }
    if (const auto delta = fabs(ev_stats[0].latency_on_run_us_moving_avg - 100000.0); delta > 50000) {
        std::cout << "Latency run took " << ev_stats[0].latency_on_get_us_moving_avg << " (delta=" << delta << "). Too off." << std::endl;        
        return 1;
    }
    std::cout << "Testing 100ms buffering." << std::endl;

    event = new AsyncEventPool<void>(fcn_wait_100ms, 1);
    for(size_t p = 0; p < tests; ++p) {
        event->post();
    }

    if (event->size() < 9) {
        std::cout << "Somehow events went too fast." << std::endl;        
        return 2;
    }

    while(event->size() > 0) {
        std::cout << "Wait... " << event->size() << std::endl;
        fcn_wait_100ms();
    }

    std::cout << "Good." << std::endl;

    return 0;
}