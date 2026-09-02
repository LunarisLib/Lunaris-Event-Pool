namespace Lunaris {
namespace EventPool {

    template<typename T>
    inline AsyncEventPool<T>::AsyncEventPool(
        std::function<void(T)> handler, 
        const size_t threads_amount)
        : m_handle(handler), m_handle_exception([](const auto& ex){ std::terminate(); })
    {
        for(size_t p = 0; p < threads_amount; ++p) {
            m_pool.push_back({
                async_event_pool_stats{},
                std::thread([this, p]{ async_event(p); })
            });
        }
    }

    template<typename T>
    inline AsyncEventPool<T>::AsyncEventPool(
        std::function<void(T)> handler, 
        std::function<void(const std::exception&)> exception_handler,
        const size_t threads_amount)
        : m_handle(handler), m_handle_exception(exception_handler)
    {
        for(size_t p = 0; p < threads_amount; ++p) {
            m_pool.push_back({
                async_event_pool_stats{},
                std::thread([this, p]{ async_event(p); })
            });
        }
    }

    template<typename T>
    inline AsyncEventPool<T>::~AsyncEventPool() {
        m_running = false;
        for(auto& [stat, thr] : m_pool) thr.join();
    }

    template<typename T>
    inline void AsyncEventPool<T>::set_moving_avg_factor(const double factor) {
        m_moving_avg_factor = factor;
    }

    template<typename T>
    inline std::vector<async_event_pool_stats> AsyncEventPool<T>::get_threads_stats() const {
        std::vector<async_event_pool_stats> stats;

        for (const auto& [stat, thr] : m_pool) 
            stats.push_back(stat);

        return stats;
    }
    
    template<typename T>
    inline void AsyncEventPool<T>::async_event(const size_t id) {
        async_event_pool_stats& stats = std::get<0>(m_pool[id]);
        std::chrono::high_resolution_clock::time_point run[3];

        while(m_running) {
            run[0] = std::chrono::high_resolution_clock::now();

            try {
                auto t = this->get_abort_if_false(m_running);

                run[1] = std::chrono::high_resolution_clock::now();

                stats.is_tasking_now = true;
                m_handle(std::move(t));
                stats.is_tasking_now = false;

                run[2] = std::chrono::high_resolution_clock::now();
            }
            catch(const EventPoolTimeoutException& ex) {
                stats.is_tasking_now = false;
                continue; // check m_running again and maybe exit
            }
            catch(const std::exception& ex) {
                stats.is_tasking_now = false;
                m_handle_exception(ex);
                continue;
            }

            stats.latency_on_get_us_moving_avg = 
                (stats.latency_on_get_us_moving_avg * m_moving_avg_factor + static_cast<double>(
                    std::chrono::duration_cast<std::chrono::microseconds>(run[1] - run[0]).count()
                )) / (m_moving_avg_factor + 1.0);

            stats.latency_on_run_us_moving_avg = 
                (stats.latency_on_run_us_moving_avg * m_moving_avg_factor + static_cast<double>(
                    std::chrono::duration_cast<std::chrono::microseconds>(run[2] - run[1]).count()
                )) / (m_moving_avg_factor + 1.0);
        }
    }

} // namespace EventPool
} // namespace Lunaris