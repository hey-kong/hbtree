#include "hbtree.h"

int main() {
  HBTree *hbtree = new HBTree();
  entry_key_t *keys = new entry_key_t[500];
  for (int i = 0; i < 500; i++) {
    keys[i] = i + 1;
  }

  for (int i = 0; i < 200; i++) {
    hbtree->insert((KEY_TYPE)keys[i * 2], (PAYLOAD_TYPE)keys[i * 2]);
  }

  for (int i = 0; i < 200; i++) {
    hbtree->insert((KEY_TYPE)keys[i * 2 + 1], (PAYLOAD_TYPE)keys[i * 2 + 1]);
  }

  for (int i = 0; i < 100; i++) {
    hbtree->erase((KEY_TYPE)keys[i]);
  }

  sleep(3);  // Wait for logs to be applied to NVM
  hbtree->SwitchInnerNode();

  for (int i = 400; i < 450; i++) {
    hbtree->insert((KEY_TYPE)keys[i], (PAYLOAD_TYPE)keys[i]);
  }

  for (int i = 0; i < 50; i++) {
    hbtree->insert((KEY_TYPE)keys[i], (PAYLOAD_TYPE)keys[i]);
  }

  sleep(2);  // Wait for logs to be applied to NVM

  char *value;
  for (int i = 0; i < 500; i += 10) {
    value = hbtree->search(keys[i]);
    if (value != nullptr) {
      printf("key(%ld), value(%ld)\n", keys[i], (PAYLOAD_TYPE)value);
    } else {
      printf("not found: key(%ld)\n", keys[i]);
    }
  }

  int cnt = 0;
  for (auto node = hbtree->dummy_->next; node != nullptr; node = node->next) {
    node->printTree();
    printf("\n");
    cnt++;
  }
  printf("alex data node count: %d\n", hbtree->index_.stats_.num_data_nodes);
  printf("inner node count: %d\n", cnt);
  for (auto node = hbtree->index_.first_data_node(); node != nullptr;
       node = node->next_leaf_) {
    printf("data node first: %ld, data node last: %ld\n", node->first_key(),
           node->last_key());
  }

  delete hbtree;
  return 0;
}
