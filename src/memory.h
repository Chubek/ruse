#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "object.h"

void gc_collect(object_t **roots, size_t num_roots);
void gc_mark(object_t *obj);
void gc_sweep(object_t *obj);

#endif
