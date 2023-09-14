#pragma once
#include <future>
#include <algorithm>
#include <chrono>
#include "thread_pool.h"

template<typename T>
struct sorter {
    ThreadPool pool;

    std::list<T> do_sort(std::list<T&> chunk_data) {

        auto begin = chunk_data.begin();
        auto end = chunk_data.end();

        if (begin >= end) {
            return;
        }

        std::list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());

        auto mid = begin + (end - begin) / 2;
        std::list<T> new_left_chunk;
        new_left_chunk.splice(new_left_chunk.end(), chunk_data, chunk_data.begin(), mid);
        std::future<std::list<T> > new_left = pool.submit(std::bind(&sorter::do_sort, this, std::move(new_left_chunk)));
        
        std::list<T> new_right_chunk(do_sort(chunk_data));

        result.splice(result.end(), new_right_chunk);
        while (!new_left_chunk.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
            pool.run_pending_task();
        }

        std::merge(result.begin(), new_left.get());
        return result;
    }
};


