#ifndef _thread_pool_HPP
#define _thread_pool_HPP

#include <vector>
#include <deque>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace sola {

	class thread_pool {
	public:
		typedef std::function<void()> task_t;

		thread_pool(int init_size = 3);
		~thread_pool();

		void stop();
		void add_task(const task_t&);  //thread safe;

	private:
		thread_pool(const thread_pool&);//½ûÖ¹¸´ÖÆ¿½±´.
		const thread_pool& operator=(const thread_pool&);

		bool is_started() { return m_is_started; }
		void start();

		void thread_loop();
		task_t take();

		typedef std::vector<std::thread*> threads_t;
		typedef std::deque<task_t> tasks_t;

		int m_init_threads_size;

		threads_t m_threads;
		tasks_t m_tasks;

		std::mutex m_mutex;
		std::condition_variable m_cond;
		bool m_is_started;
	};

}
#endif



