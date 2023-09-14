#pragma once
#include <queue>
#include <memory>
#include <mutex>

template<class T>
class thread_safe_queue {
public:
    thread_safe_queue() { }

    void push(T& item) {
        std::lock_guard<std::mutex> locl(the_locker);
        data_queue.push(std::move(item));
        m_notifier.notify_one();
    }

    void wait_pop(T& value) {
        std::unique_lock<mutex> lock(the_locker);
        if (data_queue.empty())
            m_notifier.wait(lock, [this] {return !data_queue.empty(); });
        value = std::move(data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> wait_pop() {
        std::unique_lock<std::mutex> lock(the_locker);
        if (data_queue.empty())
            m_notifier.wait(lock, [this] {return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value) {
        std::local_guard<std::mutex> lock(the_locker);
        if (data_queue.empty()) {
            return false;
        }
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lock(the_locker);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(the_locker);
        return data_queue.empty();
    }

private:
    mutable std::mutex the_locker;
    std::queue<T> data_queue;
    condition_variable m_notifier;
};