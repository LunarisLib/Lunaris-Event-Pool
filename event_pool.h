#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>
#include <functional>
#include <thread>
#include <sstream>

namespace Lunaris {

    template<typename T>
    class event_pool_async;

    template<typename T>
    class event_pool {
        friend class event_pool_async<T>;

        std::condition_variable cond;
        std::mutex mu;
        std::vector<T> queue;

        std::atomic_size_t max_timeout_wait = 1000; // ms
        std::mutex post_lock;

        std::mutex& get_cond_mutex();
        bool wait_nolock(std::unique_lock<std::mutex>&);

        // may throw exception if empty
        T grab_front();
    public:
        // Waits for task. Returns if has something to do
        T wait();

        // Returns true if has something in queue to get (use wait())
        bool is_set() const;

        // Post task to queue
        void post(T);

        // Queue size
        size_t size() const;

        // Max delay on internal refresh loop. Tasks will delay at max this time, in ms
        void set_max_delay_wait(const size_t);
    };

    struct event_task_info {
        std::string thread_id_str; // converted
        std::thread::id thread_id; // original
        bool is_tasking = false; // thread is on a task right now
        double latency_get = 0; // per task get wait (max delay defined by user, dynamic average) [microsec]
        double latency_run = 0; // per task run time (defined by user, dynamic average) [microsec]
    };

    template<typename T>
    class event_pool_async {
        struct async_info {
            std::thread thr;
            event_task_info taskinf;
        };

        event_pool<T> pool;

        std::vector<async_info> thrs;
        mutable std::recursive_mutex thrs_safety;

        std::function<void(T&&)> handl;
        std::function<void(const std::exception&)> exception_handl;
        std::mutex handl_safety;

        bool must_quit = false; // thread sync
        bool is_ready = false;

        void loop(const size_t);
    public:
        event_pool_async(const unsigned int = std::thread::hardware_concurrency());
        event_pool_async(std::function<void(T&&)>, const unsigned int = std::thread::hardware_concurrency());
        event_pool_async(std::function<void(T&&)>, std::function<void(const std::exception&)>, const unsigned int = std::thread::hardware_concurrency());
        ~event_pool_async();

        // set function handler
        void set_handler(std::function<void(T&&)>);

        // set exception handler
        void set_exception_handler(std::function<void(const std::exception&)>);

        // stop, re-set and start this thread count
        void reset_threads(const unsigned int);

        // destroys threads
        void destroy_all();

        // threads still have work queued to do
        bool has_tasks_queued() const;

        // if any thread is running
        bool has_task_running() const;

        // queue size
        size_t size_queued() const;

        // thread count set at the beginning
        size_t thread_count() const;

        // get current thread tasking information individually
        std::vector<event_task_info> get_threads_status() const;

        // post something to be processed
        void post(T);

        // set max delay in millisec (lower gets more responsive, but CPU cost is higher) (minimum: 5 ms)
        void set_max_delay_wait(const size_t);
    };

}

#include "event_pool.ipp"