#include "hbtree.h"

int main() {
  HBTree *hbtree = new HBTree();
  entry_key_t *keys = new entry_key_t[100];
  for (int i = 0; i < 100; i++) {
    keys[i] = i + 1;
  }

  for (int i = 0; i < 90; i++) {
    hbtree->insert((KEY_TYPE)keys[i], (PAYLOAD_TYPE)keys[i]);
  }

  sleep(1);  // Wait for logs to be applied to NVM

  char *value;
  for (int i = 0; i < 100; i++) {
    value = hbtree->search(keys[i]);
    if (value != nullptr) {
      printf("key(%ld), value(%ld)\n", keys[i], (PAYLOAD_TYPE)value);
    } else {
      printf("not found: key(%ld)\n", keys[i]);
    }
  }

  delete hbtree;
  return 0;
}
