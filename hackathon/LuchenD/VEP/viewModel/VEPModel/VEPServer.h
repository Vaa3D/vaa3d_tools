//
// Created by SEU on 2023/3/19.
//

#ifndef VEPMODEL_VEPSERVER_H
#define VEPMODEL_VEPSERVER_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <atomic>
#include <json/json.h>

using namespace std;

class ThreadPool {
public:
    ThreadPool(int num_threads) : stop_(false) {
        for (int i = 0; i < num_threads; ++i) {
            workers_.emplace_back(
                    [this] {
                        for (;;) {
                            std::function<void()> task;
                            {
                                std::unique_lock<std::mutex> lock(this->queue_mutex_);
                                this->condition_.wait(lock,
                                                      [this] { return this->stop_ || !this->tasks_.empty(); });
                                if (this->stop_ && this->tasks_.empty())
                                    return;
                                task = std::move(this->tasks_.front());
                                this->tasks_.pop();
                            }
                            task();
                        }
                    }
            );
        }
    }

    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace([=] { std::forward<F>(f)(std::forward<Args>(args)...); });
        }
        condition_.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread& worker : workers_)
            worker.join();
    }


private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;

};

class VEPServer {
public:
    VEPServer(int port, int num_threads) : port_(port), thread_pool_(num_threads);
    ~VEPServer() = default;

    void start();

private:
    int port_;
    ThreadPool thread_pool_;

    void hadle_client(int sockfd) {
        // Handle client connection here
    }

};


#endif //VEPMODEL_VEPSERVER_H
