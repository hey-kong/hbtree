#include "alex.h"
#include "inner_node.h"

#define KEY_TYPE int
#define PAYLOAD_TYPE int

int main() {
  InnerNode *node = new InnerNode(HOTNODE);
  entry_key_t *keys = new entry_key_t[100];
  for (int i = 0; i < 100; i++) {
    keys[i] = i + 1;
  }

  for (int i = 0; i < 90; i++) {
    node->insert(keys[i], (char *)keys[i]);
  }

  char *value;
  for (int i = 0; i < 100; i++) {
    value = node->search(keys[i]);
    if (value != nullptr) {
      printf("key(%ld), value(%ld)\n", keys[i], (entry_key_t)value);
    } else {
      printf("not found: key(%ld)\n", keys[i]);
    }
  }

  node->split(21);

  sleep(3);  // Wait for logs to be applied to NVM

  InnerNode *next = node->next;
  node->printTree();
  next->printTree();

  // Create some synthetic data: keys are dense integers between 0 and 99, and
  // payloads are random values
  alex::Alex<KEY_TYPE, PAYLOAD_TYPE> index;
  const int num_keys = 100;
  pair<KEY_TYPE, PAYLOAD_TYPE> values[num_keys];
  mt19937_64 gen(random_device{}());
  uniform_int_distribution<PAYLOAD_TYPE> dis;
  for (int i = 0; i < num_keys; i++) {
    values[i].first = i;
    values[i].second = dis(gen);
  }

  index.bulk_load(values, num_keys);

  for (int i = num_keys; i < 2 * num_keys; i++) {
    KEY_TYPE new_key = dis(gen);
    PAYLOAD_TYPE new_payload = dis(gen);
    index.insert(new_key, new_payload);
  }

  for (int i = 0; i < 10; i++) {
    index.erase(i);
  }

  int num_entries = 0;
  for (auto it = index.begin(); it != index.end(); it++) {
    if (it.key() % 2 == 0) {  // it.key() is equivalent to (*it).first
      it.payload() = dis(gen);
    }
    num_entries++;
  }
  if (num_entries != 190) {
    cout << "Error! There should be 190 entries in the index." << endl;
  }

  num_entries = 0;
  for (auto it = index.lower_bound(50); it != index.lower_bound(100); it++) {
    num_entries++;
  }
  if (num_entries != 50) {
    cout
        << "Error! There should be 50 entries with keys in the range [50, 100)."
        << endl;
  }

  num_entries = 0;
  auto it = index.lower_bound(50);
  while (it.key() < 100 && it != index.end()) {
    num_entries++;
    it++;
  }
  if (num_entries != 50) {
    cout
        << "Error! There should be 50 entries with keys in the range [50, 100)."
        << endl;
  }

  for (int i = 0; i < 9; i++) {
    KEY_TYPE new_key = 42;
    PAYLOAD_TYPE new_payload = dis(gen);
    index.insert(new_key, new_payload);
  }

  int num_duplicates = 0;
  for (auto it = index.lower_bound(42); it != index.upper_bound(42); it++) {
    num_duplicates++;
  }
  if (num_duplicates != 10) {
    cout << "Error! There should be 10 entries with key of value 42." << endl;
  }

  it = index.find(1337);
  if (it != index.end()) {
    cout << "Error! Key with value 1337 should not exist." << endl;
  }

  auto stats = index.get_stats();
  cout << "Final num keys: " << stats.num_keys << endl;
  cout << "Num inserts: " << stats.num_inserts << endl;

  delete node;
  return 0;
}
