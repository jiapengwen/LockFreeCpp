/*************************************************************************
 @Author: jiapeng.wen(jiapeng.wen@ele.me)
 @Created Time : Wed 11 Sep 2019 02:26:13 PM CST
 @File Name: LockedStackT.h
 @brief:
************************************************************************/
#include <mutex>
#include "NodeT.h"

template<typename _Ty, typename _Lock>
class LockedStackT
{
  using Node = NodeT<_Ty>;
public:
  ~LockedStackT()
  {
    std::lock_guard<_Lock> lock(lock_);
    while (head_) {
      Node* node = head_;
      head_ = node->next;
      delete node;
    }
  }
  void Push(const _Ty& val)
  {
    Node* node(new Node(val, nullptr)); // 不需要锁构造函数，这个可能是一个耗时操作
    std::lock_guard<_Lock> lock(lock_);
    node->next = head_;
    head_ = node;
  }
  std::unique_ptr<_Ty> Pop()
  {
    std::unique_ptr<_Ty> ret;
    Node* node;
    {
      // 同上，只需要锁链表本身，其他操作可以放到链表外执行
      std::lock_guard<_Lock> lock(lock_);
      node = head_;
      if (node) head_ = node->next;
    }
    if (node) {
      ret.swap(node->data);
      delete node;
    }
    return std::move(ret);
  }
 
private:
  Node* head_ = nullptr;
  _Lock lock_;
};
