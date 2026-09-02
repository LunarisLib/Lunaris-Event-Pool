#pragma once

#include <type_traits>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <vector>

#include <Lunaris/Event-Pool/exception.h>

namespace Lunaris {
namespace EventPool {

    /**
     * @brief Abstract class with what's in common in events of any type and void
     */
    class EventPoolBase {
    public:
        /**
         * @brief Set the max delay before retrying a get() without a signal
         * 
         * @param ms Time, in milliseconds, that it will re-check even without a signal, on get()
         */
        void set_max_delay_wait(const size_t ms);

        /**
         * @brief Check if there's something in the queue without locking
         * 
         * @return `bool` true means there is something there
         */
        virtual bool has() const = 0;

        /**
         * @brief Get the size of the queue right now without locking
         * 
         * @return `size_t` the amount of items in queue
         */
        virtual size_t size() const = 0;
    protected:
        std::condition_variable m_cond;
        std::mutex m_cond_mtx;
        
        std::atomic<size_t> m_max_wait_step_ms = 100;

        std::mutex m_post_mtx;
    };

    /**
     * @brief Generic EventPool for any type T
     * 
     * @tparam `T` the type this pool will hold to be gotten later with get()
     */
    template<typename T>
    class EventPool : public EventPoolBase {
    public:
        /**
         * @brief Attempts to get item in queue, or wait for one
         * 
         * @return `T` the item in front of the queue (oldest)
         */
        T get();

        /**
         * @brief Attempts to get item in queue, or wait for one, or throw if `stay_trying` goes false
         * 
         * @param stay_trying boolean that it keeps checked every timeout set with `m_max_wait_step_ms` at most
         * @return `T` the item in front of the queue (oldest)
         */
        T get_abort_if_false(const std::atomic_bool& stay_trying);

        /**
         * @brief Check if there's something in the queue without locking
         * 
         * @return `bool` true means there is something there
         */
        bool has() const;

        /**
         * @brief Post a new item to the list queue
         * 
         * @param item the item to be pushed into the queue
         */
        void post(T item);

        /**
         * @brief Get the size of the queue right now without locking
         * 
         * @return `size_t` the amount of items in queue
         */
        size_t size() const;

    private:
        std::vector<T> m_queue;
    };

    template<>
    class EventPool<void> : public EventPoolBase {
        size_t m_queue_total = 0;
    public:
        /**
         * @brief Attempts to get if there was a posted event, or wait for one
         */
        void get();

        /**
         * @brief Attempts to get if there was a posted event, or wait for one, or throw if `stay_trying` goes false
         * 
         * @param stay_trying boolean that it keeps checked every timeout set with `m_max_wait_step_ms` at most
         */
        void get_abort_if_false(const std::atomic_bool& stay_trying);

        /**
         * @brief Check if there's something in the queue without locking
         * 
         * @return `bool` true means there is something there
         */
        bool has() const;

        /**
         * @brief Post a new event to the list queue
         */
        void post();

        /**
         * @brief Get the size of the queue right now without locking
         * 
         * @return `size_t` the amount of items in queue
         */
        size_t size() const;
    };

} // namespace EventPool
} // namespace Lunaris

#include <Lunaris/Event-Pool/impl/pool.ipp>