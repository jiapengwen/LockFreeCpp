/*************************************************************************
 @Author: jiapeng.wen(jiapeng.wen@ele.me)
 @Created Time : Wed 11 Sep 2019 02:34:05 PM CST
 @File Name: TestAtomicLock.cc
 @brief:
************************************************************************/
#pragma once
#include <atomic>
#include <thread>
#include <chrono>

//利用atomic模板实现自旋锁
template<size_t _SleepWhenAcquireFailedInMicroSeconds = size_t(-1)>
class SpinLockByCasT
{
  std::atomic<bool> locked_flag_ = ATOMIC_VAR_INIT(false);
public:
  void lock()
  {
    bool exp = false;
    while (!locked_flag_.compare_exchange_strong(exp, true)) {
      exp = false;
      if (_SleepWhenAcquireFailedInMicroSeconds == size_t(-1)) {
        std::this_thread::yield();
      } else if (_SleepWhenAcquireFailedInMicroSeconds != 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(_SleepWhenAcquireFailedInMicroSeconds));
      }
    }
  }
  void unlock()
  {
    locked_flag_.store(false);
  }
};
