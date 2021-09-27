#pragma once

#include <algorithm>
#include <list>

#include "alex.h"
#include "fixed-size-priority-queue.h"
#include "inner_node.h"

#define KEY_TYPE entry_key_t
#define PAYLOAD_TYPE char *
#define OP_PERIOD 100
#define HOT_NODE_NUM 1

struct HotDegreeCmp {
  bool operator()(InnerNode *i, InnerNode *j) {
    return i->GetHotDegree() < j->GetHotDegree();
  }
};

class HBTree {
 private:
  alex::Alex<KEY_TYPE, PAYLOAD_TYPE> index_;
  list<InnerNode *> nodes_;
  int ops_;

 public:
  HBTree();

  ~HBTree();

  void SwitchNodeType() {
    fixed_size_priority_queue<InnerNode *, HotDegreeCmp> q(HOT_NODE_NUM);
    for (auto i = nodes_.begin(); i != nodes_.end(); ++i) {
      (*i)->UpdateHotDegree();
      q.push(*i);
    }
  }

  void insert(entry_key_t, char *);
  void erase(entry_key_t);
  char *search(entry_key_t);
};
