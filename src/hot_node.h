#pragma once

#include <map>

#include "background.h"
#include "inner_node.h"
#include "nvm_allocator.h"

#define LOGPATH "/pmem0/log/"
#define LOGSIZE 64 * (1 << 20)

class HotNode : public InnerNode {
 private:
  // TODO: Switch the type of data_array.
  std::map<entry_key_t, char *> data_array;
  NVMLogFile *log_;
  LogHandler *handler_;
  const char *log_path_;
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
HotNode::HotNode() : InnerNode() {
  string file = "log" + std::to_string(this->Id());
  string path = LOGPATH + file;
  log_path_ = path.c_str();
  if (file_exists(log_path_) == 0) {
    // TODO: process last remaining.
  }

  log_ = new NVMLogFile(log_path_, LOGSIZE);
  char *begin = log_->GetBeginAddr();
  handler_ = new LogHandler(begin, bt_);
  op_ = 0;
  hot_degree_ = 0;
}

HotNode::~HotNode() { delete log_; }

void HotNode::hnode_insert(entry_key_t key, char *value) {
  char *addr = log_->Write(key, value);
  data_array[key] = value;
  handler_->update(addr);
}

void HotNode::hnode_delete(entry_key_t key) {
  char *addr = log_->Delete(key);
  data_array.erase(key);
  handler_->update(addr);
}

char *HotNode::hnode_search(entry_key_t key) { return data_array[key]; }
