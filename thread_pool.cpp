#include <assert.h>
#include <iostream>
#include <sstream>
#include "thread_pool.hpp"

namespace sola {
    /*
	* static���εĺ���������̬��������̬���������֣���������ֵĵط������ࣺ
         ��������̬���������������ô����һ����̬��Ա������
		 ��̬��Ա�������������ڣ������������������ʻ����޸��κζ��󣨷�static�����ݳ�Ա��

		 ��ľ�̬��Ա(�����ͷ���)�����౾��������ص�ʱ��ͻ�����ڴ棬����ͨ������ֱ��ȥ���ʣ�
         �Ǿ�̬��Ա�������ͷ�����������Ķ�������ֻ������Ķ���������������ʵ����ʱ�Ż�����ڴ棬Ȼ��ͨ����Ķ���ʵ����ȥ���ʡ�

        ��������ǳ��������У���ô����һ����ͨ��ȫ�ֵľ�̬��������̬����ֻ���ʼ��һ�Ρ�

		������static��������ͨ�����������ǣ�
		��̬ȫ�ֱ��������Ʊ����������򣬽��ڱ��ļ��з��ʣ������ļ����ɷ��ʣ�
        ��̬�ֲ����������ڱ��������ڷ��ʣ����ļ������������ڲ��ɷ��ʣ�����̬�ֲ�������ֵ�ڳ��������ڼ䲻�����٣�
        ��̬���������ڱ��ļ��е��ã������ļ��в��ɵ��ã�������Ա���õ��ı�д�ĺ����������ļ��ĺ���ͬ����
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
          c++�������������ԣ����������������
              ���������һ���������Ա����õķ�Χ;
              �������ھ�������������Ա����õ�ʱ���
		�ֲ�����Ҳ��Ϊ�ڲ��������ֲ��������ں�����������˵���ġ�������������ں����ڣ��뿪�ú�������ʹ�����ֱ����ǷǷ��ġ�
	    ȫ�ֱ���Ҳ��Ϊ�ⲿ�����������ں����ⲿ����ı���������������һ��������������һ��Դ�����ļ�����������������Դ����
        */
        {
            std::unique_lock<std::mutex> lock(m_mutex);//���������뿪�������ͷţ�ʹ�����ط����Ի�������
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
