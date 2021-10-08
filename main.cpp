#include <iostream>
#include <chrono>
#include <condition_variable>
#include "thread_pool.hpp"

std::mutex g_mutex;

void priorityFunc()
{
    for (int i = 1; i < 4; ++i)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

        std::lock_guard<std::mutex> lock(g_mutex);
        std::cout << "priorityFunc() [" << i << "] at thread [ " << std::this_thread::get_id() << "] output" << std::endl;
    }

}

void testFunc()
{
    for (int i = 1; i < 4; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(g_mutex);
        std::cout << "testFunc() [" << i << "] at thread [ " << std::this_thread::get_id() << "] output" << std::endl;
    }

}

int main()
{
    sola::thread_pool thread_pool;

    for (int i = 0; i < 5; i++)
        thread_pool.add_task(testFunc);

    getchar();
    return 0;
}
