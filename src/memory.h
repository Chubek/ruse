#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

typedef struct Heap
{
  object_t *local_stack;
  object_t *global_roots;
  object_t *curr_env_chain;
  object_t *curr_conti;
} heap_t;

void heap_collect (heap_t *heap);
void heap_mark (object_t *obj);
void heap_sweep (object_t *obj);

#endif
