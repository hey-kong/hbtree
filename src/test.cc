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

  char *value;
  for (int i = 0; i < 100; i++) {
    value = node->hnode_search(keys[i]);
    if (value != nullptr) {
      std::cout << "key("<< keys[i] << "), value(" << (entry_key_t)value << ")" << std::endl;
    } else {
      std::cout << "not found: key("<< keys[i] << ")" << std::endl;
    }
  }

  return 0;
}
