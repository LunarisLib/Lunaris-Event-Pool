#pragma once

#include <functional>
#include <shared_mutex>
#include <chrono>
#include <thread>

#include <Lunaris/Event-Pool/pool.h>

namespace Lunaris {
namespace EventPool {

    /**
     * @brief Struct that contains useful tracking information about a thread running on AsyncEventPool
     */
    struct async_event_pool_stats{
        double latency_on_get_us_moving_avg{};
        double latency_on_run_us_moving_avg{};
        bool is_tasking_now = false;
    };

    /**
     * @brief Generic EventPool powered by asynchronous threads with a handler
     * 
     * @tparam `T` the type this pool will hold to be gotten later by the handlers
     */
    template<typename T>
    class AsyncEventPool : protected EventPool<T> {
    public:
        /**
         * @brief Construct a new Async Event Pool object without exception handling
         * 
         * @param handler the main handler, called when event arrives
         * @param threads_amount the amount of parallel threads allocated to handle events
         */
        AsyncEventPool(
            std::function<void(T)> handler,
            const size_t threads_amount = std::thread::hardware_concurrency());

        /**
         * @brief Construct a new Async Event Pool object with exception handling
         * 
         * @param handler the main handler, called when event arrives
         * @param exception_handler the handler for exceptions
         * @param threads_amount the amount of parallel threads allocated to handle events
         */
        AsyncEventPool(
            std::function<void(T)> handler,
            std::function<void(const std::exception&)> exception_handler,
            const size_t threads_amount = std::thread::hardware_concurrency());

        ~AsyncEventPool();

        /**
         * @brief Set the moving average factor used to calculate async_event_pool_stats
         * @brief Note: it won't reset the average, it'll only change the weight from now on
         * 
         * @param `factor` double value representing the weight of the current value over the new one [0.0..inf)
         */
        void set_moving_avg_factor(const double factor);

        /**
         * @brief Get the threads running status
         * 
         * @return `std::vector<async_event_pool_stats>` each thread run times, queue times and if they are busy
         */
        std::vector<async_event_pool_stats> get_threads_stats() const;

        using EventPool<T>::has;
        using EventPool<T>::post;
        using EventPool<T>::size;
    private:
        void async_event(const size_t id);

        std::vector<std::tuple<async_event_pool_stats, std::thread>> m_pool;
        const std::function<void(T)> m_handle;
        const std::function<void(const std::exception&)> m_handle_exception;
        std::atomic<double> m_moving_avg_factor{2.0}; // weight old value vs new
        std::atomic_bool m_running{true};
    };

    template<>
    class AsyncEventPool<void> : protected EventPool<void> {
    public:
        /**
         * @brief Construct a new Async Event Pool object without exception handling
         * 
         * @param handler the main handler, called when event arrives
         * @param threads_amount the amount of parallel threads allocated to handle events
         */
        AsyncEventPool(
            std::function<void()> handler,
            const size_t threads_amount = std::thread::hardware_concurrency());
        
        /**
         * @brief Construct a new Async Event Pool object with exception handling
         * 
         * @param handler the main handler, called when event arrives
         * @param exception_handler the handler for exceptions
         * @param threads_amount the amount of parallel threads allocated to handle events
         */
        AsyncEventPool(
            std::function<void()> handler,
            std::function<void(const std::exception&)> exception_handler,
            const size_t threads_amount = std::thread::hardware_concurrency());
            
        ~AsyncEventPool();

        /**
         * @brief Set the moving average factor used to calculate async_event_pool_stats
         * @brief Note: it won't reset the average, it'll only change the weight from now on
         * 
         * @param `factor` double value representing the weight of the current value over the new one [0.0..inf)
         */
        void set_moving_avg_factor(const double factor);

        /**
         * @brief Get the threads running status
         * 
         * @return `std::vector<async_event_pool_stats>` each thread run times, queue times and if they are busy
         */
        std::vector<async_event_pool_stats> get_threads_stats() const;

        using EventPool<void>::has;
        using EventPool<void>::post;
        using EventPool<void>::size;
    private:
        void async_event(const size_t id);

        std::vector<std::tuple<async_event_pool_stats, std::thread>> m_pool;
        const std::function<void()> m_handle;
        const std::function<void(const std::exception&)> m_handle_exception;
        std::atomic<double> m_moving_avg_factor{2.0}; // weight old value vs new
        std::atomic_bool m_running{true};
    };

} // namespace EventPool
} // namespace Lunaris

#include <Lunaris/Event-Pool/impl/async_pool.ipp>