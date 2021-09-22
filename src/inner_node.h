#pragma once

#include <atomic>

#include "btree.h"

std::atomic<uint64_t> node_id;

class InnerNode {
 private:
  uint64_t id_;
  entry_key_t min_key_;
  InnerNode *next_;

 public:
  InnerNode();
};

/*
 *  class InnerNode
 */
InnerNode::InnerNode() {
  id_ = node_id.fetch_add(1, std::memory_order_relaxed);
}
