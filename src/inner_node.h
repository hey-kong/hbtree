#ifndef _INNER_NODE_H_
#define _INNER_NODE_H_

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

extern atomic<uint64_t> node_id;

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

  void printTree();
};

#endif  // _INNER_NODE_H_
