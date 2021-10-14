#include "hbtree.h"

int main() {
  HBTree *hbtree = new HBTree();
  entry_key_t *keys = new entry_key_t[100];
  for (int i = 0; i < 100; i++) {
    keys[i] = i + 1;
  }

  for (int i = 0; i < 50; i++) {
    hbtree->insert((KEY_TYPE)keys[i * 2], (PAYLOAD_TYPE)keys[i * 2]);
  }

  for (int i = 0; i < 50; i++) {
    hbtree->insert((KEY_TYPE)keys[i * 2 + 1], (PAYLOAD_TYPE)keys[i * 2 + 1]);
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

  hbtree->Recycle();

  int cnt = 0;
  for (auto node = hbtree->dummy_->next; node != nullptr; node = node->next) {
    node->printTree();
    printf("\n");
    cnt++;
  }
  printf("alex datanode count: %d\n", hbtree->index_.stats_.num_data_nodes);
  printf("inner node count: %d\n", cnt);

  delete hbtree;
  return 0;
}
