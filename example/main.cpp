#include <iostream>
#include <atomic>
#include <thread>

#include <Lunaris/event_pool.h>

using namespace Lunaris::EventPool;

int main() {    
    std::atomic_size_t counter{0};
    AsyncEventPool<void> events([&counter]{ ++counter; });

    std::cout << "Testing simple watch tied there" << std::endl;

    for(size_t p = 0; p < 10; ++p) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10 + rand() % 100));
        events.post();
        std::this_thread::sleep_for(std::chrono::milliseconds(20 + rand() % 100));
    }

    std::cout << "Final: " << counter.load() << std::endl;

}