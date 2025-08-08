#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace OrderManagmentService
{

    template <typename T>
    class concurrent_queue
    {
    public:
        void push(T const &v)
        {

            std::unique_lock<std::mutex> lock(_mutex);
            _queue.push(v);
            lock.unlock();
            _condition_variable.notify_one();
        }

        bool empty() const
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _queue.empty();
        }

        bool try_pop(T &popped_value)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_queue.empty())
                return false;
            popped_value = _queue.front();
            _queue.pop();
            return true;
        }

        void wait_and_pop(T &popped_value)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            while (_queue.empty())
                _condition_variable.wait(lock);
            popped_value = _queue.front();
            _queue.pop();
        }

    private:
        std::queue<T> _queue;
        mutable std::mutex _mutex;
        std::condition_variable _condition_variable;
    };
};
