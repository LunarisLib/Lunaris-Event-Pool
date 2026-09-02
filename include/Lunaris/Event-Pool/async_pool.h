#pragma once

#include <functional>
#include <shared_mutex>
#include <thread>

#include <Lunaris/Event-Pool/pool.h>

namespace Lunaris {
namespace EventPool {

    struct async_event_pool_stats{
        double latency_on_get_us_moving_avg{};
        double latency_on_run_us_moving_avg{};
        bool is_tasking_now = false;
    };

    template<typename T>
    class AsyncEventPoolBase : protected EventPool<T> {
    public:
        AsyncEventPoolBase(
            std::function<void(T)> handler,
            std::function<void(const std::exception&)> exception_handler = [](const auto& ex){ std::terminate(); },
            const size_t threads_amount = std::thread::hardware_concurrency());
        ~AsyncEventPoolBase();

        void set_moving_avg_factor(const double factor);
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
    class AsyncEventPoolBase<void> : protected EventPool<void> {
    public:
        AsyncEventPoolBase(
            std::function<void()> handler,
            std::function<void(const std::exception&)> exception_handler = [](const auto& ex){ std::terminate(); },
            const size_t threads_amount = std::thread::hardware_concurrency());
        ~AsyncEventPoolBase();

        void set_moving_avg_factor(const double factor);
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