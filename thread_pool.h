#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>
#include <queue>
#include "safe_queue.h"
#include "work_stealing_queue.h"

class ThreadPool {
public:
    template<typename FuncType> 
    std::future<typename std::result_of<FuncType()>::type>
        submit(FuncType f) {
        std::packaged_task<result_type()> task(f);
        std::future<result_type> res(task.get_future());
        if (local_work_queue) {
            local_queue_type->push(std::move(task))
        }
        else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }

    void run_pending_task() {
        function_wrapper task;
        if (local_work_queue && !local_work_queue->empty()) {
            task = std::move(local_work_queue->front());
            local_work_queue->pop();
            task();
        }
        else if (pool_work_queue.try_pop(task)) {
            task();
        }
        else {
            std::this_thread::yield();
        }
     }
private: 
    std::atomic_bool done;
   
    typedef function_wrapper task_type;
    thread_safe_queue<task_type> pool_work_queue;
    
    std::vector<std::unique_ptr<work_stealing_queue> > m_queues;
    std::vector<std::thread> m_threads;

    static thread_local work_stealing_queue* local_work_queue;
    static thread_local unsigned qindex;

    void worker_thread(unsigned qindex_) {
        qindex = qindex_;
        local_work_queue = m_queues[qindex].get();
        while (!done) {
            run_pending_task();
        }
    }

    bool pop_task_from_local_queue(task_type& task) {
        return local_work_queue && local_work_queue->try_pop();
    }

    bool pop_task_from_pool_queue(task_type& task) {
        return pool_work_queue.try_pop(task);
    }

    bool pop_task_from_other_thread_queue(task_type& task) {
        for (unsigned int i = 0; i < m_queues.size(); ++i) {
            unsigned const index = (qindex + i + 1) % m_queues.size();
            if (m_queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }

};

//ThreadPool::ThreadPool() {
//    if (std::thread::hardware_concurrency() == 0) {
//        m_thread_count = 4;
//    }
//    else {
//        m_thread_count = std::thread::hardware_concurrency();
//    }
//    for (int i = 0; i < m_thread_count; i++) {
//        m_thread_queues.emplace_back();
//    }
//}