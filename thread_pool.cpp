#include <assert.h>
#include <iostream>
#include <sstream>
#include "thread_pool.hpp"

namespace sola {
    /*
	* static修饰的函数叫做静态函数，静态函数有两种，根据其出现的地方来分类：
         如果这个静态函数出现在类里，那么它是一个静态成员函数；
		 静态成员函数的作用在于：调用这个函数不会访问或者修改任何对象（非static）数据成员。

		 类的静态成员(变量和方法)属于类本身，在类加载的时候就会分配内存，可以通过类名直接去访问；
         非静态成员（变量和方法）属于类的对象，所以只有在类的对象产生（创建类的实例）时才会分配内存，然后通过类的对象（实例）去访问。

        如果它不是出现在类中，那么它是一个普通的全局的静态函数，静态变量只会初始化一次。

		这样的static函数与普通函数的区别是：
		静态全局变量：限制变量的作用域，仅在本文件中访问，其他文件不可访问；
        静态局部变量：仅在本函数体内访问，本文件其他函数体内不可访问；但静态局部变量的值在程序运行期间不会销毁；
        静态函数：仅在本文件中调用，其他文件中不可调用，即程序员不用担心编写的函数与其他文件的函数同名。
    */
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
        /*
          c++变量有两个属性：作用域和生命周期
              作用域就是一个变量可以被引用的范围;
              生命周期就是这个变量可以被引用的时间段
		局部变量也称为内部变量。局部变量是在函数内作定义说明的。其作用域仅限于函数内，离开该函数后再使用这种变量是非法的。
	    全局变量也称为外部变量，它是在函数外部定义的变量。它不属于哪一个函数，它属于一个源程序文件。其作用域是整个源程序。
        */
        {
            std::unique_lock<std::mutex> lock(m_mutex);//互斥锁，离开作用域释放，使其它地方可以获得这个锁
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
                task();//testFunc
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
