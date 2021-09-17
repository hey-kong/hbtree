#pragma once

#include "inner_node.h"

class HotNode : public InnerNode {
 private:
  btree *bt;
  void *log;
  int op;
  int hot_degree;

 public:
  HotNode();
  void hnode_insert(entry_key_t, char *);
  void hnode_delete(entry_key_t);
  char *hnode_search(entry_key_t);
};

/*
 *  class HotNode
 */
HotNode::HotNode() {
  bt = new btree();
  log = nullptr;
  op = 0;
  hot_degree = 0;
}

void HotNode::hnode_insert(entry_key_t key, char *right) {
  bt->btree_insert(key, right);
  // TODO: Write log and do extra processing.
}

void HotNode::hnode_delete(entry_key_t key) {
  bt->btree_delete(key);
  // TODO: Write log and do extra processing.
}

char *HotNode::hnode_search(entry_key_t key) {
  // TODO: Write log and do extra processing.
  return bt->btree_search(key);
}
