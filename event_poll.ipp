#pragma once
#include "event_poll.h"

namespace Lunaris {

    template<typename T>
    inline std::mutex& event_poll<T>::get_cond_mutex()
    {
        return mu;
    }

    template<typename T>
    inline bool event_poll<T>::wait_nolock(std::unique_lock<std::mutex>& lk)
    {
        if (is_set()) return true;
        cond.wait_for(lk, std::chrono::milliseconds(2000), [&]{ return is_set(); }); // this will wait for max 2 times lol
        return is_set();
    }

    template<typename T>
    inline T event_poll<T>::grab_front()
    {
        if (queue.size() == 0) throw std::out_of_range("Queue was empty on get!");
        T mov = std::move(queue.front());
        queue.erase(queue.begin());
        return mov;
    }

    template<typename T>
    inline T event_poll<T>::wait()
    {
        std::unique_lock<std::mutex> lk(mu);

        if (!is_set())
            cond.wait_for(lk, std::chrono::milliseconds(50), [this]{ return is_set();});
        
        return grab_front();
    }

    template<typename T>
    inline bool event_poll<T>::is_set() const
    {
        return queue.size();
    }

    template<typename T>
    inline void event_poll<T>::post(T t)
    {
        std::lock_guard<std::mutex> lp(post_lock);
        {
            std::lock_guard<std::mutex> lk(mu);
            queue.emplace_back(std::move(t));
        }
        cond.notify_one();
    }

    template<typename T>
    inline void event_poll_async<T>::loop(const size_t id)
    {
        while (!is_ready && !must_quit) std::this_thread::sleep_for(std::chrono::milliseconds(20));

        async_info& self = thrs[id];
        self.taskinf.thread_id = std::this_thread::get_id();
        {
            std::stringstream ss;
            ss << std::this_thread::get_id();
            self.taskinf.thread_id_str = ss.str();
        }

        while(!must_quit) {
            decltype(std::chrono::high_resolution_clock::now()) dts[3]{};

            dts[0] = std::chrono::high_resolution_clock::now();

            std::unique_lock<std::mutex> lk(poll.get_cond_mutex());
            //std::cout << std::this_thread::get_id() << ":" << id << " IDLE" << std::endl;
            self.taskinf.is_tasking = false;

            if (!poll.wait_nolock(lk)) continue;

            //std::cout << std::this_thread::get_id() << ":" << id << " --- GOT" << std::endl;
            self.taskinf.is_tasking = true;
            T _t = poll.grab_front();

            //std::cout << std::this_thread::get_id() << ":" << id << " RUNNING" << std::endl;
            lk.unlock();

            poll.cond.notify_all(); // unlock locked ones if any

            std::unique_lock<std::mutex> lu(handl_safety);
            std::function<void(T&&)> handl_cpy = handl;
            lu.unlock();

            dts[1] = std::chrono::high_resolution_clock::now();

            if (handl_cpy) {
                try {
                    handl_cpy(std::move(_t));
                }
                catch(const std::exception& e){
                    std::lock_guard<std::mutex> lu(handl_safety);
                    if (exception_handl) exception_handl(e);
                    // else do nothing, so bad.
                }
                catch(...){
                    std::lock_guard<std::mutex> lu(handl_safety);
                    if (exception_handl) exception_handl(std::runtime_error("UNCAUGHT"));
                    // else do nothing, so bad.
                }
            }

            dts[2] = std::chrono::high_resolution_clock::now();

            self.taskinf.latency_get = (self.taskinf.latency_get * 2.0 + static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(dts[1] - dts[0]).count())) / 3.0;
            self.taskinf.latency_run = (self.taskinf.latency_run * 2.0 + static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(dts[2] - dts[1]).count())) / 3.0;
        }
        self.taskinf.is_tasking = false;
    }

    template<typename T>
    inline event_poll_async<T>::event_poll_async(const unsigned int numthr)
    {
        reset_threads(numthr);
    }

    template<typename T>
    inline event_poll_async<T>::event_poll_async(std::function<void(T&&)> f, const unsigned int numthr)
        : handl(f)
    {
        if (!f) throw std::invalid_argument("Function is empty! If you don't want to specify one on constructor, don't call the one that sets it!");
        reset_threads(numthr);
    }
    
    template<typename T>
    inline event_poll_async<T>::event_poll_async(std::function<void(T&&)> f, std::function<void(const std::exception&)> ef, const unsigned int numthr) 
        : handl(f), exception_handl(ef)
    {
        if (!f || !ef) throw std::invalid_argument("One of the functions is empty! If you don't want to specify one on constructor, don't call the one that sets it!");
        reset_threads(numthr);
    }
    
    template<typename T>
    inline event_poll_async<T>::~event_poll_async()
    {
        destroy_all();
    }

    template<typename T>
    inline void event_poll_async<T>::set_handler(std::function<void(T&&)> f)
    {
        std::lock_guard<std::mutex> lu(handl_safety);
        handl = f;
    }

    template<typename T>
    inline void event_poll_async<T>::set_exception_handler(std::function<void(const std::exception&)> f)
    {
        std::lock_guard<std::mutex> lu(handl_safety);
        exception_handl = f;
    }

    template<typename T>
    inline void event_poll_async<T>::reset_threads(const unsigned int numthr)
    {
        std::lock_guard<std::recursive_mutex> lu(thrs_safety);
        destroy_all();
        must_quit = false;
        is_ready = false;
        for(unsigned int a = 0; a < numthr; ++a) thrs.push_back({std::thread{[this, a]{ loop(static_cast<size_t>(a)); }}, {}});
        is_ready = true;
    }

    template<typename T>
    inline void event_poll_async<T>::destroy_all()
    {
        std::lock_guard<std::recursive_mutex> lu(thrs_safety);
        must_quit = true;
        if (thrs.size()) {
            std::lock_guard<std::mutex> lp(poll.post_lock);
            for(auto& it : thrs) if (it.thr.joinable()) it.thr.join();
            thrs.clear();
        }
    }

    template<typename T>
    inline bool event_poll_async<T>::has_tasks_queued() const
    {
        return poll.is_set();
    }

    template<typename T>
    inline bool event_poll_async<T>::has_task_running() const
    {
        if (poll.is_set()) return true;
        std::lock_guard<std::recursive_mutex> lu(thrs_safety);
        for(const auto& i : thrs) if (i.taskinf.is_tasking) return true;
        return false;
    }

    template<typename T>
    inline std::vector<event_task_info> event_poll_async<T>::get_threads_status() const
    {
        std::lock_guard<std::recursive_mutex> lu(thrs_safety);
        std::vector<event_task_info> vec;
        for(const auto& i : thrs) vec.push_back(i.taskinf);
        return vec;
    }

    template<typename T>
    inline void event_poll_async<T>::post(T t)
    {
        poll.post(std::move(t));
    }
}