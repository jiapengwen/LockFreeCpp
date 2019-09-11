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
//利用atomic_flag实现自旋锁
template<size_t _SleepWhenAcquireFailedInMicroSeconds = size_t(-1)>
class SpinLockByTasT
{
  std::atomic_flag locked_flag_ = ATOMIC_FLAG_INIT;
public:
  void lock()
  {
    while (locked_flag_.test_and_set()) {
      if (_SleepWhenAcquireFailedInMicroSeconds == size_t(-1)) {
        std::this_thread::yield();
      } else if (_SleepWhenAcquireFailedInMicroSeconds != 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(_SleepWhenAcquireFailedInMicroSeconds));
      }
    }
  }
  void unlock()
  {
    locked_flag_.clear();
  }
};
