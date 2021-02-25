// condition_variable example
#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <atomic>

std::mutex mtx; // 全局互斥锁.
std::condition_variable cv; // 全局条件变量.
std::atomic<bool> ready = false; // 全局标志位.


void do_print_id(int id)
{ 
    std::unique_lock <std::mutex> lck(mtx);
    //lck.unlock();
    while (!ready.load()) // 如果标志位不为 true, 则等待...
        cv.wait(lck); // 当前线程被阻塞, 当全局标志位变为 true 之后,
    // 线程被唤醒, 继续往下执行打印线程编号id.
    //lck.unlock();
    std::cout << "thread " << id << '\n';
}

void go()
{
    ready.store(true); // 设置全局标志位为 true.
    cv.notify_all(); // 唤醒所有线程.
}

int main()
{
    std::thread threads[10];
    // spawn 10 threads:
    for (int i = 0; i < 10; ++i)
        threads[i] = std::thread(do_print_id, i);

    std::cout << "10 threads ready to race...\n";
    go(); // go!

    for (auto & th:threads)
        th.join();

    return 0;
}