#include "hbtree.h"

string DefaultNodeState = HOTNODE;

HBTree::HBTree() {
  dummy_ = new InnerNode(INNERNODE);
  InnerNode *node = new InnerNode(DefaultNodeState);
  dummy_->next = node;
  node->prev = dummy_;

  index_.InitRootNode(node);
  ops_ = 0;
}

HBTree::~HBTree() {
  delete dummy_;
  dummy_ = nullptr;
}

void HBTree::SwitchInnerNode() {
  for (auto node = dummy_->next; node != nullptr; node = node->next) {
    while (node != nullptr && node->to_be_recycled()) {
      node->prev->next = node->next;
      if (node->next != nullptr) {
        node->next->prev = node->prev;
      }
      auto tmp = node;
      node = node->next;
      delete tmp;
      tmp = nullptr;
    }
  }

  index_.AdjustNodeType(HOT_NODE_NUM);
}

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
