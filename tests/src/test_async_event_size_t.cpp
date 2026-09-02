#include <iostream>
#include <atomic>
#include <thread>

#include <Lunaris/event_pool.h>

using namespace Lunaris::EventPool;

int main() {
    constexpr size_t expected_len = 10;
    size_t accum_ext = 0;

    std::atomic_size_t counter{0};
    AsyncEventPool<size_t> events([&counter](size_t acc){ counter += acc; });

    std::cout << "Testing simple watch tied there" << std::endl;

    for(size_t p = 0; p < expected_len; ++p) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10 + rand() % 100));
        events.post(p);
        accum_ext += p;
        std::this_thread::sleep_for(std::chrono::milliseconds(20 + rand() % 100));
    }

    std::cout << "Final: " << counter.load() << " (and expected: " << accum_ext << ")" << std::endl;

    return counter.load() != accum_ext ? 1 : 0;
}