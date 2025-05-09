#include "threadpool.hpp"



ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
        threads_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_); 
                    cv_.wait(lock, [this] {
                        return !tasks_.empty() || stop_; // only returns if tasks is not empty or stop is true
                    });

                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    task = std::move(tasks_.front());
                    tasks_.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }

    cv_.notify_all();

    for (auto& thread : threads_) {
        thread.join();
    }
}
// Enqueue task for execution
void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        tasks_.emplace(std::move(task)); // move tranfers ownership of task into tasks_ queue
    }
    cv_.notify_one();
}
