#pragma once

#include <algorithm>
#include <list>
#include <map>

#include "alex.h"
#include "fixed_size_priority_queue.h"
#include "inner_node.h"

#define KEY_TYPE entry_key_t
#define PAYLOAD_TYPE char *
#define OP_PERIOD 100
#define HOT_NODE_NUM 1
#define HOTNODE "HotNode"
#define COLDNODE "ColdNode"

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

  void AdjustNodeType() {
    fixed_size_priority_queue<InnerNode *, HotDegreeCmp> q(HOT_NODE_NUM);
    for (auto i = nodes_.begin(); i != nodes_.end(); ++i) {
      (*i)->UpdateHotDegree();
      q.push(*i);
    }
    // Record which nodes need to become hot node.
    map<uint16_t, bool> m;
    for (auto i = q.begin(); i != q.end(); ++i) {
      m[(*i)->Id()] = true;
    }
    for (auto i = nodes_.begin(); i != nodes_.end(); ++i) {
      if (m[(*i)->Id()]) {
        if ((*i)->type() == HOTNODE) {
          continue;
        } else {
          SwitchToHot(*i);
        }
      } else {
        if ((*i)->type() == COLDNODE) {
          continue;
        } else {
          SwitchToCold(*i);
        }
      }
    }
  }

  void SwitchToCold(InnerNode *node) {}

  void SwitchToHot(InnerNode *node) {}

  void insert(entry_key_t, char *);
  void erase(entry_key_t);
  char *search(entry_key_t);
};
