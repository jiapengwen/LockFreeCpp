/*************************************************************************
 @Author: jiapeng.wen(jiapeng.wen@ele.me)
 @Created Time : Wed 11 Sep 2019 02:34:05 PM CST
 @File Name: TestAtomicLock.cc
 @brief:
************************************************************************/
#include <iostream>
#include <thread>
#include "LockedStackT.h"
#include "SpinLockByTasT.h"
#include "SpinLockByCasT.h"
#include "MemoryReclamationByReference.h"
#include "MemoryReclamationByHazardPointer.h"
#include "LockFreeStack.h"

template<typename _Ty, int _ThreadCount = 16, int _LoopCount = 100000>
struct LockFreePerformanceTestT
{
  template<class _ProcessUnit>
  static double Run(_ProcessUnit puf)
  {
    std::thread ths[_ThreadCount];
    auto st = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < _ThreadCount; ++i) ths[i] = std::thread([&puf]() {
      for (int i = 0; i < _LoopCount; ++i) {
        puf();
      }
    });
    for (int i = 0; i < _ThreadCount; ++i) ths[i].join();
    const double period_in_ms = static_cast<double>((std::chrono::high_resolution_clock::now() - st).count())
      / std::chrono::high_resolution_clock::period::den * 1000;
    return period_in_ms;
  }
  static void Run()
  {
    _Ty s;
    std::cout << Run([&s]() {s.Push(0); }) << "\t\t";
    std::cout << Run([&s]() { s.Pop(); }) << std::endl;
  }
};
int main()
{
  std::cout << "LockedStack with std::mutex" << "\t\t\t\t\t";
  LockFreePerformanceTestT<LockedStackT<uint32_t, std::mutex>>::Run();
  std::cout << "LockedStack with SpinLockByTas yield" << "\t\t\t\t";
  LockFreePerformanceTestT<LockedStackT<uint32_t, SpinLockByTasT<>>>::Run();
  std::cout << "LockedStack with SpinLockByCas yield" << "\t\t\t\t";
  LockFreePerformanceTestT<LockedStackT<uint32_t, SpinLockByCasT<>>>::Run();
  std::cout << "LockedStack with SpinLockByTas usleep(5)" << "\t\t\t";
  LockFreePerformanceTestT<LockedStackT<uint32_t, SpinLockByTasT<5>>>::Run();
  std::cout << "LockedStack with SpinLockByCas usleep(5)" << "\t\t\t";
  LockFreePerformanceTestT<LockedStackT<uint32_t, SpinLockByCasT<5>>>::Run();
  std::cout << "LockFreeStack with MemoryReclamationByReferenceCounting" << "\t\t";
  LockFreePerformanceTestT<LockFreeStackT<uint32_t, MemoryReclamationByReferenceCountingT<NodeT<uint32_t>>>>::Run();
  std::cout << "LockFreeStack with MemoryReclamationByHazardPointer" << "\t\t";
  LockFreePerformanceTestT<LockFreeStackT<uint32_t, MemoryReclamationByHazardPointerT<NodeT<uint32_t>>>>::Run();
  return 0;
}
