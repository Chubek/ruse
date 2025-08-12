#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

typedef struct Heap
{
  object_t **roots;
  size_t roots_size;
  size_t roots_count;
} heap_t;

heap_t *heap_new (size_t size);
void heap_delete (heap_t *heap);
void heap_add_root (heap_t *heap, object_t *obj);
void heap_collect (heap_t *heap);
void heap_mark (heap_t *heap);
void heap_sweep (heap_t *heap);

#endif
