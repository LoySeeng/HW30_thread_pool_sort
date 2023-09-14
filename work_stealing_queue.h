#pragma once
#include <queue>
#include <future>
#include <mutex>
#include <queue>
#include "wrapper.h"

typedef function_wrapper data_type;

class work_stealing_queue {
public:
    work_stealing_queue() { }
    work_stealing_queue(const work_stealing_queue& other) = delete;
    work_stealing_queue operator=(const work_stealing_queue other) = delete;

    void push(data_type item) {
        std::lock_guard<std::mutex> lock(m_locker);
        m_task_queue.push(std::move(item));
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_locker);
        return m_task_queue.empty();
    }

    bool try_pop(data_type& res) {
        std::lock_guard<std::mutex> lock(m_locker);
        if (m_task_queue.empty()) {
            return false;
        }
        res = std::move(m_task_queue.front());
        m_task_queue.pop_front();
        return true;
    }

    bool try_steal(data_type& res) {
        std::lock_guard<std::mutex> lock(m_locker);
        if (m_task_queue.empty()) {
            return false;
        }
        res = std::move(m_task_queue.back());
        m_task_queue.pop_back();
        return true;
    }

private:
    std::queue<data_type> m_task_queue;

    mutable std::mutex m_locker;
};
