#include "inner_node.h"

atomic<uint64_t> node_id;

InnerNode::InnerNode(string type) {
  id_ = node_id.fetch_add(1, memory_order_relaxed);
  to_be_recycled_ = false;
  cnt_ = 0;
  op_ = 0;
  hot_degree_ = 0;
  type_ = type;
  prev = nullptr;
  next = nullptr;

  if (type != INNERNODE) {
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
  } else {
    persistent_path_ = "";
  }

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
  prev = nullptr;
  next = nullptr;
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

InnerNode *InnerNode::split(entry_key_t key) {
  auto new_node = new InnerNode(HOTNODE);
  auto next_node = this->next;
  new_node->prev = this;
  new_node->next = next_node;
  this->next = new_node;
  if (next_node != nullptr) {
    next_node->prev = new_node;
  }

  log_->Split(key, new_node);
  return new_node;
}

void InnerNode::printTree() { D_RW(bt_)->printAll(); }
