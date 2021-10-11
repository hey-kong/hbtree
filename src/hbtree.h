#ifndef _HBTREE_H_
#define _HBTREE_H_

#include <algorithm>
#include <map>

#include "alex.h"
#include "fixed_size_priority_queue.h"
#include "inner_node.h"

#define KEY_TYPE entry_key_t
#define PAYLOAD_TYPE char *
#define OP_PERIOD 100
#define HOT_NODE_NUM 1

class HBTree {
 private:
  alex::Alex<KEY_TYPE, PAYLOAD_TYPE> index_;
  InnerNode *dummy_;
  int ops_;

 public:
  HBTree();
  ~HBTree();

  void AdjustNodeType();
  void SwitchToCold(InnerNode *node);
  void SwitchToHot(InnerNode *node);

  void insert(entry_key_t, char *);
  void erase(entry_key_t);
  char *search(entry_key_t);
};

#endif  // _HBTREE_H_
