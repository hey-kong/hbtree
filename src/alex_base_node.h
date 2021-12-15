#ifndef _ALEX_BASE_NODE_H_
#define _ALEX_BASE_NODE_H_

#include "alex_base.h"

namespace alex {

// A parent class for both types of ALEX nodes
template <class T, class P>
class AlexNode {
 public:
  // Whether this node is a leaf (data) node
  bool is_leaf_ = false;

  // Power of 2 to which the pointer to this node is duplicated in its parent
  // model node
  // For example, if duplication_factor_ is 3, then there are 8 redundant
  // pointers to this node in its parent
  uint8_t duplication_factor_ = 0;

  // Node's level in the RMI. Root node is level 0
  short level_ = 0;

  // Both model nodes and data nodes nodes use models
  LinearModel<T> model_;

  // Could be either the expected or empirical cost, depending on how this field
  // is used
  double cost_ = 0.0;

  AlexNode() = default;
  explicit AlexNode(short level) : level_(level) {}
  AlexNode(short level, bool is_leaf) : is_leaf_(is_leaf), level_(level) {}
  virtual ~AlexNode() = default;

  // The size in bytes of all member variables in this class
  virtual long long node_size() const = 0;
};
}  // namespace alex

#endif  // _ALEX_BASE_NODE_H_
