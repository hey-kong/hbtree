#include "hot_node.h"

int main() {
  HotNode *node = new HotNode();
  entry_key_t *keys = new entry_key_t[100];
  for (int i = 0; i < 100; i++) {
    keys[i] = i + 1;
  }

  for (int i = 0; i < 90; i++) {
    node->hnode_insert(keys[i], (char *)keys[i]);
  }

  for (int i = 0; i < 100; i++) {
    node->hnode_search(keys[i]);
  }

  return 0;
}
