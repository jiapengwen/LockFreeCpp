/*************************************************************************
 @Author: jiapeng.wen(jiapeng.wen@ele.me)
 @Created Time : Wed 11 Sep 2019 02:58:54 PM CST
 @File Name: MemoryReclamationByHazardPointer.h
 @brief:
************************************************************************/
#pragma once
#include <atomic>
#include <cassert>

template<class _Node, size_t _MaxPopThreadCount = 16>
class MemoryReclamationByHazardPointerT
{
  struct HazardPointer
  {
    std::atomic<std::thread::id> id;
    std::atomic<_Node*> ptr = ATOMIC_VAR_INIT(nullptr);
  };
  HazardPointer hps_[_MaxPopThreadCount];
  std::atomic<_Node*> will_be_deleted_list_ = ATOMIC_VAR_INIT(nullptr);
  void _ReleaseImpl(_Node* node)
  {
    // 检查HazardPointers中是否有线程正在访问当前指针
    size_t i = 0;
    while (i < _MaxPopThreadCount) {
      if (hps_[i].ptr.load() == node)
        break;
      ++i;
    }
    if (i == _MaxPopThreadCount) { // 无任何线程正在访问当前指针，直接删除
      delete node;
    } else {  // 有线程正在访问，加入缓存列表
      node->next = will_be_deleted_list_.load();
      while (!will_be_deleted_list_.compare_exchange_strong(node->next, node));
    }
  }
public:
  ~MemoryReclamationByHazardPointerT()
  {
    // 自己不能删除自己，正常退出HazardPointer始终会持有一个节点，只能在此做清理
    size_t count = 0;
    _Node* to_delete_list = will_be_deleted_list_.exchange(nullptr);
    while (to_delete_list) {
      _Node* node = to_delete_list;
      to_delete_list = node->next;
      delete node;
      ++count;
    }
    assert(count < 2);
  }
  inline void Addref() {}
  bool Store(_Node* node)
  {
    struct HazardPointerOwner
    {
      HazardPointer* hp;
      HazardPointerOwner(HazardPointer* hps)
        : hp(nullptr)
      {
        for (size_t i = 0; i < _MaxPopThreadCount; ++i) {
          std::thread::id id;
          if (hps[i].id.compare_exchange_strong(id, std::this_thread::get_id())) {
            hp = &hps[i];
            break;
          }
        }
      }
      ~HazardPointerOwner()
      {
        if (hp) {
          hp->ptr.store(nullptr);
          hp->id.store(std::thread::id());
        }
      }
    };
    thread_local HazardPointerOwner owner(hps_);
    if (!node || !owner.hp) return false;
    owner.hp->ptr.store(node);
    return true;
  }
  bool Release(_Node* node)
  {
    if (!node) return false;
    _ReleaseImpl(node); // 对当前传入指针进行释放操作
                        // 循环检测will_be_deleted_list_, 可以另开一个线程定时检测效率会更高
    _Node* to_delete_list = will_be_deleted_list_.exchange(nullptr);
    while (to_delete_list) {
      _Node* node = to_delete_list;
      to_delete_list = node->next;
      _ReleaseImpl(node);
    }
    return true;
  }
};
