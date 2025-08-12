#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

#define HEAP_GROWTH_FACTOR 0.75

typedef struct Heap
{
  object_t **pool;
  size_t pool_size;
  size_t pool_count;
} heap_t;

heap_t *heap_new (size_t size);
void heap_delete (heap_t *heap);
void heap_add_to_pool (heap_t *heap, object_t *obj);
void heap_collect (heap_t *heap);
void heap_mark (heap_t *heap);
void heap_sweep (heap_t *heap);

#endif
