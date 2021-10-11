#include "hbtree.h"

struct HotDegreeCmp {
  bool operator()(InnerNode *i, InnerNode *j) {
    return i->GetHotDegree() < j->GetHotDegree();
  }
};

HBTree::HBTree() {
  dummy_ = new InnerNode(INNERNODE);
  InnerNode *node = new InnerNode(HOTNODE);
  dummy_->next = node;
  node->prev = dummy_;

  index_.InitRootNode(node);
  ops_ = 0;
}

HBTree::~HBTree() {}

void HBTree::AdjustNodeType() {
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

void HBTree::SwitchToCold(InnerNode *node) {}

void HBTree::SwitchToHot(InnerNode *node) {}

void HBTree::insert(entry_key_t, char *) {}

void HBTree::erase(entry_key_t) {}

char *HBTree::search(entry_key_t) {}
