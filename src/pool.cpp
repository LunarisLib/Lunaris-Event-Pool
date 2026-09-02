#include <Lunaris/Event-Pool/pool.h>
#include <Lunaris/Event-Pool/exception.h>

namespace Lunaris {
namespace EventPool {

    void EventPoolBase::set_max_delay_wait(const size_t ms) {
        m_max_wait_step_ms = ms;
    }
    

    void EventPool<void>::get() {
        std::unique_lock<std::mutex> lk(m_cond_mtx);

        while (!has())
            m_cond.wait_for(lk, std::chrono::milliseconds(m_max_wait_step_ms), [this]{ return has();});

        if (m_queue_total == 0)
            throw EventPoolException("Condition test failed, caused invalid event.");
            
        --m_queue_total;
    }

    void EventPool<void>::get_abort_if_false(const std::atomic_bool& stay_trying) {
        std::unique_lock<std::mutex> lk(m_cond_mtx);

        while (!has() && stay_trying)
            m_cond.wait_for(lk, std::chrono::milliseconds(m_max_wait_step_ms), [this,&stay_trying]{ return has() || !stay_trying; });

        if (m_queue_total == 0) {
            if (!stay_trying) throw EventPoolTimeoutException("Boolean condition became false before event arrived.");
            else              throw EventPoolException("Condition test failed, caused invalid event.");
        }
            
        --m_queue_total;
    }

    bool EventPool<void>::has() const {
        return m_queue_total > 0;
    }

    void EventPool<void>::post() {
        std::lock_guard<std::mutex> lp(m_post_mtx);
        {
            std::lock_guard<std::mutex> lk(m_cond_mtx);
            ++m_queue_total;
        }
        m_cond.notify_one();
    }

    size_t EventPool<void>::size() const {
        return m_queue_total;
    }

} // namespace EventPool
} // namespace Lunaris