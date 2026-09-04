/*
FrameQueue

Thread-safe bounded queue used to pass frames between pipeline threads.

Typical flow:
Capture Thread → FrameQueue → Processing Thread

The queue prevents race conditions and limits memory usage by keeping
a maximum number of frames. If the queue is full, new frames are dropped
to maintain real-time behaviour.
*/

#pragma once 
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class FrameQueue
{
public:
    explicit FrameQueue(std::size_t max_size)
        : max_size_(max_size), stopped_(false), dropped_count(0)

    {
    }

    bool push(const T& item)
    {
        //different threads can not touch mutex at the same time
        std::lock_guard<std::mutex> lock(mutex_);

        cond_var_.wait(lock, [this]()
        {
            return !queue_.empty() || stopped_;
        });

        if (queue_.empty())
        {
            return false
        }

        item = queue_.front();
        queue_.pop()
        return true;
    }

    void stop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
        cond_var_.notify_all();
    }

    bool is_stopped() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return stopped_;
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return dropped_count_;
    }

private:
    std::queue<T> queue;
    std::size_t max_size_;

    mutable std::mutex mutex_;
    std::condition_variable cond_var_;

    bool stopped_;
    std::size_t dropped_count_;
};