#ifndef _INNER_NODE_H_
#define _INNER_NODE_H_

#include <map>

#include "btree.h"
#include "nvm_allocator.h"

#define INNERNODE "InnerNode"
#define HOTNODE "HotNode"
#define COLDNODE "ColdNode"
#define BTREEPATH "/mnt/pmem/data/"
#define LOGPATH "/mnt/pmem/log/"
#define LOGSIZE (24 << 10)
#define DECAY 0.2

/*
 *file_exists -- checks if file exists
 */
static inline int file_exists(char const *file) { return access(file, F_OK); }

extern atomic<uint64_t> node_id;

class InnerNode {
 protected:
  uint64_t id_;
  bool to_be_recycled_;
  int cnt_;
  entry_key_t min_key_;
  int op_;
  double hot_degree_;
  const char *persistent_path_;
  PMEMobjpool *pop_;
  string type_;

  // HotNode
  const char *log_path_;
  NVMLogFile *log_;

 public:
  TOID(btree) bt_;
  InnerNode *prev;
  InnerNode *next;

  InnerNode(string);
  ~InnerNode();

  uint64_t Id() { return this->id_; }
  string type() { return this->type_; }
  void delete_node() { this->to_be_recycled_ = true; }
  bool to_be_recycled() { return this->to_be_recycled_; }
  void set_hot() { type_ = HOTNODE; }
  void set_cold() { type_ = COLDNODE; }

  void UpdateHotDegree();
  double GetHotDegree();

  void insert(entry_key_t, char *);
  void erase(entry_key_t);
  char *search(entry_key_t);
  InnerNode *split(entry_key_t, entry_key_t);

  void printTree();
};

#endif  // _INNER_NODE_H_
