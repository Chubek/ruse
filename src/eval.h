#ifndef EVAL_H
#define EVAL_H

#include "heap.h"
#include "object.h"

typedef struct Interpreter interp_t;
typedef struct Expander expander_t;

struct Interpreter
{
   heap_t *heap;
   stack_t *stack;
   environ_t *environ;
   object_t *accumulator;
   object_t *next_expr;
};

struct 

#endif
