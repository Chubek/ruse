#include "memory.h"

void
gc_collect (environ_t *env)
{
  for (size_t i = 0; i < env->size; i++)
    {
      entry_t *e = env->entires[i];
      while (e)
        {
          gc_mark (e->value);
          gc_sweep (e->value);
          e = e->next;
        }
    }
}

void
gc_mark (object_t *obj)
{
  if (!obj)
    return;
  object_t *next = obj->next;
  obj->marked = true;
  switch (obj->type)
    {
    case OBJ_Pair:
      gc_mark (obj->v_pair->first);
      gc_mark (obj->v_pair->rest);
      break;
    case OBJ_Vector:
      for (size_t i = 0; i < obj->v_vector->size; i++)
        gc_mark (obj->v_vector->vals[i]);
      break;
    case OBJ_Environ:
      for (size_t i = 0; i obj->v_environ->size; i++)
        {
          for (entry_t *e = obj->v_environ->entries[i]; e; e = e->next)
            {
              if (!e)
                continue;

              gc_mark (e->key);
              gc_mark (e->value);
            }
        }
      break;
    default:
      break;
    }

  gc_mark (next);
}

void
gc_sweep (object_t *obj)
{
  if (!obj)
    return;

  object_t *next = obj->next;
  if (!obj->marked)
    object_delete (obj);
  else
    obj->marked = false;
  gc_sweep (next);
}

heap_t *
heap_new (void)
{
  heap_t *heap = malloc (sizeof (heap_t));
  heap->roots = calloc (INIT_ROOTS_SIZE, sizeof (uintptr_t));
  heap->size = INIT_ROOTS_SIZE;
  heap->count = 0;

  return heap;
}

void
heap_delete (heap_t *heap)
{
  free (heap->roots);
  free (heap);
}

void
heap_add_root (heap_t *heap, object_t *root)
{
  if (heap->count / heap->size >= COLL_GROWTH_RATE)
    {
      heap->size *= 1.5;
      heap->roots = realloc (heap->roots, heap->size);
    }
  heap->roots[heap->count++] = root;
}
