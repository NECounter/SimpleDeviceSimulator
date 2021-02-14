// condition_variable example
#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

std::mutex mtx;
bool ready = false;

void print_id (int id) {
  std::cout << "try to lock\n";
  std::unique_lock<std::mutex> lck(mtx);
  std::cout << "try to lock??\n";
  while (!lck.owns_lock()) {
    
    lck.lock();
  }

  std::cout << "thread " << id << '\n';
}


int main ()
{
  std::unique_lock<std::mutex> lck(mtx);
  std::thread threads[10];
  // spawn 10 threads:
  for (int i=0; i<10; ++i)
    threads[i] = std::thread(print_id,i);

  std::cout << "10 threads ready to race...\n";
  lck.unlock();


  for (auto& th : threads) th.join();

  return 0;
}