#include "hbtree.h"

string DefaultNodeState = HOTNODE;

struct HotDegreeCmp {
  bool operator()(InnerNode *i, InnerNode *j) {
    return i->GetHotDegree() < j->GetHotDegree();
  }
};

HBTree::HBTree() {
  dummy_ = new InnerNode(INNERNODE);
  InnerNode *node = new InnerNode(DefaultNodeState);
  dummy_->next = node;
  node->prev = dummy_;

  index_.InitRootNode(node);
  ops_ = 0;
}

HBTree::~HBTree() {}

void HBTree::Recycle() {
  for (auto node = dummy_->next; node != nullptr; node = node->next) {
    while (node != nullptr && node->to_be_recycled()) {
      node->prev->next = node->next;
      if (node->next != nullptr) {
        node->next->prev = node->prev;
      }
      auto tmp = node;
      node = node->next;
      delete tmp;
    }
  }
}

void HBTree::AdjustNodeType() {
  fixed_size_priority_queue<InnerNode *, HotDegreeCmp> q(HOT_NODE_NUM);
  for (auto node = dummy_->next; node != nullptr; node = node->next) {
    while (node != nullptr && node->to_be_recycled()) {
      node->prev->next = node->next;
      if (node->next != nullptr) {
        node->next->prev = node->prev;
      }
      auto tmp = node;
      node = node->next;
      delete tmp;
    }
    node->UpdateHotDegree();
    q.push(node);
  }

  // Record which nodes need to become hot node
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

void HBTree::insert(KEY_TYPE key, PAYLOAD_TYPE value) {
  index_.insert(key, value);
}

void HBTree::erase(KEY_TYPE key) { index_.erase(key); }

char *HBTree::search(KEY_TYPE key) {
  auto it = index_.find(key);
  if (it.cur_idx_ == -1) {
    // Cannot find data from data nodes of index, search directly in NVM
    return it.cur_leaf_->inner_node->search(key);
  }
  if (it == index_.end()) {
    return nullptr;
  }
  return (char *)(it.payload());
}
