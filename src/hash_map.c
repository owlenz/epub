#include "erm.h"
#include <hash_map.h>
#include <stdlib.h>
#include <string.h>

HashMap *hmp_init() {
  HashMap *hmp = malloc(sizeof(HashMap));
  for (int i = 0; i < HASH_MAP_SIZE; i++)
    hmp->buckets[i] = NULL;
  ASSERT(hmp != NULL);
  return hmp;
}

unsigned long _hash(const char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  return hash % HASH_MAP_SIZE;
}

HashNode *_create_node(const char *key, const uint32_t value) {
  HashNode *new_node = (HashNode *)malloc(sizeof(HashNode));
  new_node->key = strdup(key);
  new_node->value = value;
  new_node->next = NULL;
  return new_node;
}

void hmp_insert(HashMap *map, const char *key, const uint value) {
  unsigned long index = _hash(key);
  HashNode *new_node = _create_node(key, value);

  if (!map->buckets[index]) {
    map->buckets[index] = new_node;
    return;
  }

  // Handle collision: seperate chaining
  HashNode *curr = map->buckets[index];
  while (curr) {
    if (strcmp(curr->key, key) == 0) {
      curr->value = value;
      free(new_node->key);
      free(new_node);
      return;
    }
    if (!curr->next)
      break;
    curr = curr->next;
  }
  curr->next = new_node;
}

uint hmp_get(HashMap map, const char *key) {
  unsigned long index = _hash(key);
  HashNode *curr = map.buckets[index];
  while (curr) {
    if (strcmp(curr->key, key) == 0)
      return curr->value;
    curr = curr->next;
  }
  return -1;
}

void hmp_delete(HashMap *map, const char *key) {
  unsigned long index = _hash(key);
  HashNode *curr = map->buckets[index];
  HashNode *prev = NULL;

  while (curr) {
    if (strcmp(curr->key, key) == 0) {
      if (prev)
        prev->next = curr->next;
      else
        map->buckets[index] = curr->next;

      free(curr->key);
      free(curr);
      return;
    }
    prev = curr;
    curr = curr->next;
  }
}

void free_hash_map(HashMap *map) {
  for (int i = 0; i < HASH_MAP_SIZE; i++) {
    HashNode *curr = map->buckets[i];
    while (curr) {
      HashNode *temp = curr;
      curr = curr->next;
      free(temp->key);
      free(temp);
    }
  }
  free(map);
}
