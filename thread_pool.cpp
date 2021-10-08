#include <assert.h>
#include <iostream>
#include <sstream>
#include "thread_pool.hpp"

namespace sola {
    static std::string
        get_tid() {
        std::stringstream tmp;
        tmp << std::this_thread::get_id();
        return tmp.str();
    }

    thread_pool::thread_pool(int init_size)
        :m_init_threads_size(init_size),
        m_mutex(),
        m_cond(),
        m_is_started(false)
    {
        start();
    }

    thread_pool::~thread_pool()
    {
        if (m_is_started)
        {
            stop();
        }
    }

    void thread_pool::start()
    {
        assert(m_threads.empty());
        m_is_started = true;
        m_threads.reserve(m_init_threads_size);
        for (int i = 0; i < m_init_threads_size; ++i)
        {
            m_threads.push_back(new std::thread(std::bind(&thread_pool::thread_loop, this)));
        }

    }

    void thread_pool::stop()
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_is_started = false;
            m_cond.notify_all();
        }

        for (threads_t::iterator it = m_threads.begin(); it != m_threads.end(); ++it)
        {
            (*it)->join();
            delete* it;
        }
        m_threads.clear();
    }


    void thread_pool::thread_loop()
    {
        while (m_is_started)
        {
            task_t task = take();
            if (task)
            {
                task();
            }
        }
    }

    void thread_pool::add_task(const task_t& task)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_tasks.push_back(task);
        m_cond.notify_one();
    }

    thread_pool::task_t thread_pool::take()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_tasks.empty() && m_is_started)
        {
            m_cond.wait(lock);
        }

        task_t task;
        tasks_t::size_type size = m_tasks.size();
        if (!m_tasks.empty() && m_is_started)
        {
            task = m_tasks.front();
            m_tasks.pop_front();
        }

        return task;

    }

}
