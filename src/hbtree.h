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

struct HotDegreeCmp {
  bool operator()(InnerNode *i, InnerNode *j) {
    return i->GetHotDegree() < j->GetHotDegree();
  }
};

class HBTree {
 private:
  alex::Alex<KEY_TYPE, PAYLOAD_TYPE> index_;
  InnerNode *dummy_;
  int ops_;

 public:
  HBTree();

  ~HBTree();

  void AdjustNodeType() {
    fixed_size_priority_queue<InnerNode *, HotDegreeCmp> q(HOT_NODE_NUM);
    for (auto node = dummy_->next; node != nullptr; node = node->next) {
      node->UpdateHotDegree();
      q.push(node);
    }
    // Record which nodes need to become hot node.
    map<uint16_t, bool> m;
    for (auto i = q.begin(); i != q.end(); ++i) {
      m[(*i)->Id()] = true;
    }
    for (auto node = dummy_->next; node != nullptr; node = node->next) {
      if (m[node->Id()]) {
        if (node->type() == HOTNODE) {
          continue;
        } else {
          SwitchToHot(node);
        }
      } else {
        if (node->type() == COLDNODE) {
          continue;
        } else {
          SwitchToCold(node);
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

/*
 *  class HBTree
 */
HBTree::HBTree() { dummy_ = new InnerNode(COLDNODE); }

#endif  // _HBTREE_H_
