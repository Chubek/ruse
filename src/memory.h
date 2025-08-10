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
  size_t size;
  size_t count;
} heap_t;

void gc_collect (heap_t *heap);
void gc_mark (object_t *obj);
void gc_sweep (object_t *obj);

heap_t *heap_new (void);
void heap_delete (heap_t *heap);
void heap_add_root (heap_t *heap, object_t *root);

#endif
