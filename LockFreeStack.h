/*************************************************************************
 @Author: jiapeng.wen(jiapeng.wen@ele.me)
 @Created Time : Wed 11 Sep 2019 03:01:37 PM CST
 @File Name: LockFreeStack.h
 @brief:
************************************************************************/
#pragma once 
#include <atomic>
#include <cstring>
#include <memory>
#include "NodeT.h"

template<typename _Ty, typename _MemoryReclamation>
class LockFreeStackT
{
  using Node = NodeT<_Ty>;
  struct TaggedPointer
  {
    Node* ptr;
    size_t tag;
    TaggedPointer() {}
    TaggedPointer(Node* _ptr, size_t _tag)
      : ptr(_ptr)
      , tag(_tag)
    {
    }
  };
public:
  ~LockFreeStackT()
  {
    TaggedPointer o(nullptr, 0);
    head_.exchange(o);

    Node* head = o.ptr;
    while (head) {
      Node* node = head;
      head = node->next;
      delete node;
    }
  }
  void Push(const _Ty& val)
  {
    TaggedPointer o = head_.load();
    TaggedPointer n(new Node(val, o.ptr), o.tag + 1);
    while (!head_.compare_exchange_strong(o, n)) {
      n.ptr->next = o.ptr;
      n.tag = o.tag + 1;
    }
  }
  std::unique_ptr<_Ty> Pop()
  {
    memory_reclamation_.Addref();
    TaggedPointer o = head_.load(), n;
    while (true) {
      if (!o.ptr) break;
      memory_reclamation_.Store(o.ptr);
      // HazardPointer算法储存(相当于上锁)后，需要对有效值进行二次确认，否则还是有先删除的问题
      // 这样做并没效率问题，不等的情况CAS操作也会进行循环，因此可以作为针对任何内存回收算法的固定写法
      const TaggedPointer t = head_.load();
      if (memcmp(&t, &o, sizeof(TaggedPointer))) {
        o = t;
        continue;
      }
      n.ptr = o.ptr->next;
      n.tag = o.tag + 1;
      if (head_.compare_exchange_strong(o, n)) break;
    }
    memory_reclamation_.Store(nullptr);
    std::unique_ptr<_Ty> ret;
    if (o.ptr) {
      ret.swap(o.ptr->data);
      memory_reclamation_.Release(o.ptr);
    }
    return std::move(ret);
  }
private:
  std::atomic<TaggedPointer> head_ = ATOMIC_VAR_INIT(TaggedPointer(nullptr, 0));
  _MemoryReclamation memory_reclamation_;
};
