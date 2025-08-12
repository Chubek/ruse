#include <stdlib.h>
#include <string.h>

#include "memory.h"

heap_t *
heap_new (size_t size)
{
  heap_t *heap = malloc (sizeof (heap_t));
  heap->pool = calloc (size, sizeof (object_t *));
  heap->size = size;
  heap->count = 0;
  return heap;
}

void
heap_delete (heap_t *heap)
{
  free (heap->pool);
  free (heap);
}

void
heap_add_to_pool (heap_t *heap, object_t *obj)
{
  if (heap->count / heap->size >= HEAP_GROWTH_FACTOR)
    {
      heap->size *= 2;
      heap->pool = realloc (heap->pool, heap->size * sizeof (object_t *));
    }
  heap->pool[heap->count++] = obj;
}

void
heap_collect (heap_t *heap)
{
  gc_mark (heap->local_stack);
  gc_mark (heap->global_roots);
  gc_mark (heap->curr_env_chain);
  gc_mark (heap->curr_conti);

  gc_sweep (heap->pool);
}

void
heap_mark (object_t *obj)
{
  if (!obj)
    return;
  object_t *next = obj->next;
  obj->marked = true;
  switch (obj->type)
    {
    case OBJ_Pair:
      heap_mark (obj->v_pair->first);
      heap_mark (obj->v_pair->rest);
      break;
    case OBJ_Vector:
      for (size_t i = 0; i < obj->v_vector->count; i++)
        heap_mark (obj->v_vector->vals[i]);
      break;
    case OBJ_Stack:
      for (size_t i = 0; i < obj->v_stack->count; i++)
        heap_mark (obj->v_stack->objs[i]);
      break;
    case OBJ_Environ:
      for (size_t i = 0; i obj->v_environ->size; i++)
        {
          for (entry_t *e = obj->v_environ->entries[i]; e; e = e->next)
            {
              if (!e)
                continue;

              heap_mark (e->key);
              heap_mark (e->value);
            }
        }
      break;
    default:
      break;
    }

  heap_mark (next);
}

void
heap_sweep (object_t *obj)
{
  if (!obj)
    return;

  object_t *next = obj->next;
  if (!obj->marked)
    object_delete (obj);
  else
    obj->marked = false;
  heap_sweep (next);
}
