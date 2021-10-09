#pragma once

#include <atomic>
#include <map>

#include "btree.h"
#include "nvm_allocator.h"

#define HOTNODE "HotNode"
#define COLDNODE "ColdNode"
#define BTREEPATH "/pmem0/data/"
#define LOGPATH "/pmem0/log/"
#define LOGSIZE (1024 + (4 << 10))
#define DECAY 0.2

/*
 *file_exists -- checks if file exists
 */
static inline int file_exists(char const *file) { return access(file, F_OK); }

atomic<uint64_t> node_id;

class InnerNode {
 protected:
  uint64_t id_;
  int cnt_;
  entry_key_t min_key_;
  int op_;
  double hot_degree_;
  const char *persistent_path_;
  TOID(btree) bt_;
  PMEMobjpool *pop_;
  string type_;

  // HotNode
  // TODO: Switch the type of data_array.
  map<entry_key_t, char *> data_array;
  const char *log_path_;
  NVMLogFile *log_;

 public:
  InnerNode *prev;
  InnerNode *next;

  InnerNode(string);

  ~InnerNode();

  uint64_t Id() { return this->id_; }
  string type() { return this->type_; }

  void UpdateHotDegree() { hot_degree_ = DECAY * hot_degree_ + op_; }
  double GetHotDegree() { return hot_degree_; }

  void insert(entry_key_t, char *);
  void erase(entry_key_t);
  char *search(entry_key_t);
  void split(entry_key_t);

  void printAll() { D_RW(bt_)->printAll(); }
};

/*
 *  class InnerNode
 */
InnerNode::InnerNode(string type) {
  id_ = node_id.fetch_add(1, memory_order_relaxed);
  cnt_ = 0;
  op_ = 0;
  hot_degree_ = 0;
  type_ = type;

  string file = "data" + to_string(id_);
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

  prev = nullptr;
  next = nullptr;

  if (type == HOTNODE) {
    string file = "log" + to_string(this->Id());
    string path = LOGPATH + file;
    log_path_ = path.c_str();
    if (file_exists(log_path_) == 0) {
      // TODO: process last remaining.
    }
    log_ = new NVMLogFile(log_path_, LOGSIZE, bt_);
    log_->start();
  } else {
    log_path_ = "";
    log_ = nullptr;
  }
}

InnerNode::~InnerNode() {
  delete prev;
  delete next;
  pmemobj_close(pop_);

  if (this->type() == HOTNODE) {
    delete log_;
  }
}

void InnerNode::insert(entry_key_t key, char *value) {
  if (this->type() == HOTNODE) {
    log_->Write(key, value);
    data_array[key] = value;
  } else {
    D_RW(bt_)->btree_insert(key, value);
  }
  cnt_++;
}

void InnerNode::erase(entry_key_t key) {
  if (this->type() == HOTNODE) {
    log_->Delete(key);
    data_array.erase(key);
  } else {
    D_RW(bt_)->btree_delete(key);
  }
  cnt_--;
}

char *InnerNode::search(entry_key_t key) {
  if (this->type() == HOTNODE) {
    return data_array[key];
  }
  return D_RW(bt_)->btree_search(key);
}

void InnerNode::split(entry_key_t right_boundary) {
  auto next_node = this->next;
  auto new_node = new InnerNode(HOTNODE);
  this->next = new_node;
  new_node->next = next_node;

  log_->Split(right_boundary, new_node);
}
