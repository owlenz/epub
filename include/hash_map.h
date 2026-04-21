#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <sys/types.h>
#define HASH_MAP_SIZE 100

typedef struct HashNode {
  char *key;
  uint32_t value;
  struct HashNode *next;
} HashNode;

typedef struct {
  HashNode *buckets[HASH_MAP_SIZE];
} HashMap;


HashMap *hmp_init();

unsigned long _hash(const char *str);

HashNode *_create_node(const char *key, const uint32_t value);

void hmp_insert(HashMap *map, const char *key, const uint32_t value);

uint32_t hmp_get(HashMap map, const char *key);

void hmp_delete(HashMap *map, const char *key);

void free_hash_map(HashMap *map);
#endif
