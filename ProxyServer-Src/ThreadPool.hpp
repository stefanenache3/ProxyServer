#ifndef ThreadPool_hpp
#define ThreadPool_hpp

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <tuple>
#include "Proxy.hpp"
class ThreadPool
{
private:
    int _nbThreads;
    std::vector<std::thread> _pool;
    std::condition_variable _cond;
    std::mutex _mutex;
    std::queue<std::pair<std::function<void(Proxy &, int, int)>, std::pair<int, int>>> _queue;
    bool _run;

    void addThreads()
    {
        for (int ii = 0; ii < _nbThreads; ii++)
        {
            _pool.push_back(std::thread(&ThreadPool::run, this));
        }
    };

public:
    ThreadPool(int nbThreads) : _nbThreads(nbThreads), _run(true) { addThreads(); };

    ThreadPool() : _nbThreads(std::thread::hardware_concurrency()), _run(false) { addThreads(); };

    void run()
    {
        std::function<void(Proxy &, int, int)> task;

        int arg1, arg2;
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(_mutex);

                _cond.wait(lock, [this]
                           { return !this->_queue.empty(); });

                task = _queue.front().first;
                arg1 = _queue.front().second.first;
                arg2 = _queue.front().second.second;
                _queue.pop();
            }
            task(Proxy::getInstance(), arg1, arg2);
            if (_run == false)
            {
                return;
            }
        }
    };

    void add(std::function<void(Proxy &, int, int)> task, int clientSocket, int conId)
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _queue.push(std::make_pair(task, std::make_pair(clientSocket, conId)));
        }
        _cond.notify_one();
    };

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _run = false;
        }
        _cond.notify_all();
        for (std::thread &t : _pool)
            t.join();
    };
};

#endif
