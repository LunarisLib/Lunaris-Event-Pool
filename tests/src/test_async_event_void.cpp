#include <iostream>
#include <atomic>
#include <thread>

#include <Lunaris/event_pool.h>

using namespace Lunaris::EventPool;

int main() {
    constexpr size_t expected_len = 10;
    std::atomic_size_t counter{0};
    AsyncEventPoolBase<void> events([&counter]{ ++counter; });

    std::cout << "Testing simple watch tied there" << std::endl;

    for(size_t p = 0; p < expected_len; ++p) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10 + rand() % 100));
        events.post();
        std::this_thread::sleep_for(std::chrono::milliseconds(20 + rand() % 100));
    }

    std::cout << "Final: " << counter.load() << " (and expected: " << expected_len << ")" << std::endl;

    return counter.load() != expected_len ? 1 : 0;
}