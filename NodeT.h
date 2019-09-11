/*************************************************************************
 @Author: jiapeng.wen(jiapeng.wen@ele.me)
 @Created Time : Wed 11 Sep 2019 03:12:28 PM CST
 @File Name: NodeT.h
 @brief:
************************************************************************/
#pragma once
#include <memory>

template<typename _Ty>
struct NodeT
{
  std::unique_ptr<_Ty> data;
  NodeT* next;
  NodeT(const _Ty& _val, NodeT* _next)
    : data(new _Ty(_val))
    , next(_next)
  {
  }
};
