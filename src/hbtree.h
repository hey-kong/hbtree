#ifndef _HBTREE_H_
#define _HBTREE_H_

#include <algorithm>
#include <map>

#include "alex.h"

#define KEY_TYPE entry_key_t
#define PAYLOAD_TYPE uint64_t

#define OP_PERIOD 100
#define HOT_NODE_NUM 1

extern string DefaultNodeState;

class HBTree {
 public:
  alex::Alex<KEY_TYPE, PAYLOAD_TYPE> index_;
  InnerNode *dummy_;
  int ops_;

  HBTree();
  ~HBTree();

  void SwitchInnerNode();

  void insert(KEY_TYPE, PAYLOAD_TYPE);
  void erase(KEY_TYPE);
  char *search(KEY_TYPE);
};

#endif  // _HBTREE_H_
