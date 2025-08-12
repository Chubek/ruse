#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

void gc_collect (environ_t *env);
void gc_mark (object_t *obj);
void gc_sweep (object_t *obj);

#endif
