#ifndef _HBTREE_H_
#define _HBTREE_H_

#include <algorithm>
#include <map>

#include "alex.h"
#include "fixed_size_priority_queue.h"

#define KEY_TYPE entry_key_t
#define PAYLOAD_TYPE uint64_t
#define OP_PERIOD 100
#define HOT_NODE_NUM 1

string DefaultNodeState = HOTNODE;

class HBTree {
 public:
  alex::Alex<KEY_TYPE, PAYLOAD_TYPE> index_;
  InnerNode *dummy_;
  int ops_;

  HBTree();
  ~HBTree();

  void Recycle();
  void AdjustNodeType();
  void SwitchToCold(InnerNode *node);
  void SwitchToHot(InnerNode *node);

  void insert(KEY_TYPE, PAYLOAD_TYPE);
  void erase(KEY_TYPE);
  char *search(KEY_TYPE);
};

#endif  // _HBTREE_H_
