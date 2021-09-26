#pragma once

#include <algorithm>
#include <list>

#include "alex.h"
#include "inner_node.h"

#define KEY_TYPE entry_key_t
#define PAYLOAD_TYPE char*

class HBTree {
 private:
  alex::Alex<KEY_TYPE, PAYLOAD_TYPE> index;
  list<InnerNode*> nodes;

 public:
  HBTree();
  ~HBTree();
};
