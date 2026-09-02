#include <iostream>
#include <atomic>
#include <thread>

#include <Lunaris/event_pool.h>

using namespace Lunaris::EventPool;

int main() {
    constexpr size_t expected_len = 10;
    size_t accum_ext = 0;
    EventPool<size_t> simple_events;
    std::atomic_size_t counter{0};
    std::atomic_bool running{true};

    std::thread watch([&simple_events,&counter,&running]{ 
        while(running) {
            try {
                counter += simple_events.get_abort_if_false(running);
            } catch(const EventPoolTimeoutException& e) {
                std::cout << "# thread got exception: " << e.what() << std::endl;
                continue;
            }
            std::cout << "# thread event [" << counter.load() << "]" << std::endl;
        }
    });

    std::cout << "Testing simple watch tied there" << std::endl;

    for(size_t p = 0; p < expected_len; ++p) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10 + rand() % 100));
        simple_events.post(p);
        accum_ext += p;
        std::this_thread::sleep_for(std::chrono::milliseconds(20 + rand() % 100));
    }

    running = false;
    watch.join();

    std::cout << "Final: " << counter.load() << " (and expected: " << accum_ext << ")" << std::endl;

    return counter.load() != accum_ext ? 1 : 0;
}