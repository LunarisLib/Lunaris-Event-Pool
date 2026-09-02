namespace Lunaris {
namespace EventPool {

    template<typename T>
    inline T EventPool<T>::get() {
        std::unique_lock<std::mutex> lk(m_cond_mtx);

        while (!has())
            m_cond.wait_for(lk, std::chrono::milliseconds(m_max_wait_step_ms), [this]{ return has();});
        
        if (m_queue.size() == 0)
            throw EventPoolException("Condition test failed, caused invalid event.");

        T mov = std::move(m_queue.front());
        m_queue.erase(m_queue.begin());
        return mov;
    }

    template<typename T>
    inline bool EventPool<T>::has() const {
        return m_queue.size() > 0;
    }

    template<typename T>
    inline void EventPool<T>::post(T item) {
        std::lock_guard<std::mutex> lp(m_post_mtx);
        {
            std::lock_guard<std::mutex> lk(m_cond_mtx);
            m_queue.emplace_back(std::move(item));
        }
        m_cond.notify_one();
    }

    template<typename T>
    inline size_t EventPool<T>::size() const {
        return m_queue.size();
    }


} // namespace EventPool
} // namespace Lunaris