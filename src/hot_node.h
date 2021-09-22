#pragma once

#include "inner_node.h"
#include "nvm_allocator.h"

class HotNode : public InnerNode {
 private:
  btree *bt_;
  NVMLogFile *log_;
  int op_;
  int hot_degree_;

 public:
  HotNode();
  HotNode(const std::string path, size_t size);

  ~HotNode();

  void hnode_insert(entry_key_t, char *);
  void hnode_delete(entry_key_t);
  char *hnode_search(entry_key_t);
};

/*
 *  class HotNode
 */
HotNode::HotNode() {
  bt_ = new btree();
  log_ = new NVMLogFile(LOGPATH, LOGSIZE);
  op_ = 0;
  hot_degree_ = 0;
}

HotNode::HotNode(const std::string path, size_t size) {
  bt_ = new btree();
  log_ = new NVMLogFile(path, size);
  op_ = 0;
  hot_degree_ = 0;
}

HotNode::~HotNode() { delete bt_; }

void HotNode::hnode_insert(entry_key_t key, char *value) {
  log_->Write(key, value);
  bt_->btree_insert(key, value);
}

void HotNode::hnode_delete(entry_key_t key) {
  log_->Delete(key);
  bt_->btree_delete(key);
}

char *HotNode::hnode_search(entry_key_t key) { return bt_->btree_search(key); }
