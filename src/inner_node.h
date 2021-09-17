#pragma once

#include "btree.h"

class InnerNode {
 private:
  entry_key_t min_key;
  InnerNode *next;

 public:
  InnerNode() {}
};
