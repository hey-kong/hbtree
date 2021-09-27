#pragma once

#include <atomic>

#include "btree.h"

#define BTREEPATH "/pmem0/data/"
#define BTREESIZE 16 * (1 << 20)
#define DECAY 0.2

/*
 *file_exists -- checks if file exists
 */
static inline int file_exists(char const *file) { return access(file, F_OK); }

std::atomic<uint64_t> node_id;

class InnerNode {
 protected:
  uint64_t id_;
  entry_key_t min_key_;
  int op_;
  double hot_degree_;
  InnerNode *next_;
  const char *persistent_path_;
  TOID(btree) bt_;
  PMEMobjpool *pop_;

 public:
  InnerNode();

  ~InnerNode();

  uint64_t Id() { return this->id_; }

  void UpdateHotDegree() { hot_degree_ = DECAY * hot_degree_ + op_; }
  double GetHotDegree() { return hot_degree_; }

  void bt_insert(entry_key_t, char *);
  void bt_delete(entry_key_t);
  char *bt_search(entry_key_t);
};

/*
 *  class InnerNode
 */
InnerNode::InnerNode() {
  id_ = node_id.fetch_add(1, std::memory_order_relaxed);

  string file = "data" + std::to_string(id_);
  string path = BTREEPATH + file;
  persistent_path_ = path.c_str();

  bt_ = TOID_NULL(btree);
  if (file_exists(persistent_path_) != 0) {
    pop_ = pmemobj_create(persistent_path_, file.c_str(), BTREESIZE, 0666);
    bt_ = POBJ_ROOT(pop_, btree);
    D_RW(bt_)->constructor(pop_);
  } else {
    pop_ = pmemobj_open(persistent_path_, file.c_str());
    bt_ = POBJ_ROOT(pop_, btree);
  }
}

InnerNode::~InnerNode() {
  delete next_;
  pmemobj_close(pop_);
}

void InnerNode::bt_insert(entry_key_t key, char *value) {
  D_RW(bt_)->btree_insert(key, value);
}

void InnerNode::bt_delete(entry_key_t key) { D_RW(bt_)->btree_delete(key); }

char *InnerNode::bt_search(entry_key_t key) {
  return D_RW(bt_)->btree_search(key);
}
