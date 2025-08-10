#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "types.h"

#define COLL_GROWTH_RATE 0.75

object_t *
object_new (objtype_t type, void *value)
{
  object_t *obj = calloc (1, sizeof (object_t));
  obj->type = type;
  obj->marked = false;
  obj->next = NULL;
  obj->tail = obj;

  switch (obj->type)
    {
    case OBJ_Nil:
      break;
    case OBJ_String:
    case OBJ_Label:
    case OBJ_Symbol:
      obj->buffz = (const uint8_t *)value;
      break;
    case OBJ_Integer:
      obj->v_integer = *(intmax_t *)value;
      break;
    case OBJ_Real:
      obj->v_real = *(double *)value;
      break;
    case OBJ_Complex:
      obj->v_complex = *(double complex *)value;
      break;
    case OBJ_Bool:
      obj->v_bool = *(bool *)value;
      break;
    case OBJ_Vector:
      obj->v_vector = (vector_t *)value;
      break;
    case OBJ_Bytevector:
      obj->v_bytevector = (vector_t *)value;
      break;
    case OBJ_Port:
      obj->v_port = (port_t *)value;
      break;
    case OBJ_Pair:
      obj->v_pair = (pair_t *)value;
      break;
    case OBJ_Environ:
      obj->v_environ = (environ_t *)value;
      break;
    case OBJ_Procedure:
      obj->v_procedure = (procedure_t *)value;
      break;
    case OBJ_Closure:
      obj->v_closure = (closure_t *)value;
      break;
    case OBJ_PrimFn:
      obj->v_primfn = (primfn_t *)value;
      break;
    case OBJ_Formal:
      obj->v_formal = (formal_t *)value;
      break;
    default:
      break;
    }

  return obj;
}

void
object_delete (object_t *obj)
{
  if (!obj)
    return;

  switch (obj->type)
    {
    case OBJ_Nil:
    case oBJ_Bool:
    case OBJ_Integer:
    case OBJ_Real:
    case OBJ_Complex:
    case OBJ_PrimFn:
      break;
    case OBJ_String:
    case OBJ_Label:
    case OBJ_Symbol:
      free (obj->v_buffz);
      break;
    case OBJ_Environ:
      for (size_t i = 0; i < obj->v_environ->size; i++)
        {
          entry_t *e = obj->v_environ->entries[i];
          while (e)
            {
              entry_t *next = e->next;
              free (e->key);
              object_delete (e->key);
              object_delete (e->value);
              free (e);
              e = next;
            }
        }
      free (obj->v_environ->entries);
      free (obj->v_environ);
      break;
    case OBJ_Vector:
      for (size_t i = 0; i < obj->v_vector->size; i++)
        object_delete (obj->v_vector->vals[i]);
      free (obj->v_vector->vals);
      free (obj->v_vector);
      break;
    case OBJ_Bytevector:
      free (obj->v_bytevector->vals);
      free (obj->v_bytevector);
      break;
    case OBJ_Formal:
      object_delete (obj->v_formal->value);
      free (obj->v_formal);
      break;
    case OBJ_Closure:
      object_delete (obj->v_closure->formals);
      object_delete (obj->v_closure->env);
      object_delete (obj->v_closure->body);
      free (obj->v_closure);
      break;
    case OBJ_Procedure:
      object_delete (obj->v_procedure->value);

      break;
    case OBJ_Port:
      fclose (obj->v_port->stream);
      object_delete (obj->v_port);
    case OBJ_Pair:
      object_delete (obj->v_pair->first);
      object_delete (obj = > v_pair->rest);
      break;
    }

  free (obj);
}

void
object_append (object_t *head, object_t *newobj)
{
  if (head)
    return;

  head->tail->next = newobj;
  head->tail = newobj;
}

void
gc_collect (object_t **roots, size_t num_roots)
{
  for (size_t i = 0; i < num_roots; i++)
    {
      gc_mark (roots[i]);
      gc_sweep (roots[i]);
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

void
environ_grow_if_should (environ_t **env)
{
  if ((*env)->count / (*env)->size <= COLL_GROWTH_RATE)
    return;

  environ_t *new_env = malloc (sizeof (environ_t));
  new_env->entries = calloc ((*env)->size * 2, sizeof (entry_t));
  new_env->size = (*env)->size * 2;
  new_env->count = 0;

  for (size_t i = 0; i < (*env)->size; i++)
    {
      for (entry_t *e = (*env)->entries[i]; e; e = e->next)
        {
          if (!e)
            continue;
          environ_insert (new_env, e->key, e->value);
        }
    }

  free ((*env)->entries);
  free (*env);
  *env = new_env;
}

void
environ_insert (environ_t *env, object_t *key, object_t *value)
{
  environ_grow_if_should (&env);
  uint32_t hash = object_hash (key);
  if (hash >= env->size)
    raise_runtime_error ("Object hash for insertion larger than table size");

  entry_t *bucket = env->entries[hash], *e;
  for (e = bucket; e->next; e = e->next)
    {
      if (object_equals (e->key, key))
        {
          e->value = value;
          return;
        }
    }

  e->next = malloc (sizeof (entry_t));
  e->next->next = NULL;
  e->next->key = key;
  e->next->value = value;
}

object_t *
environ_retrieve (environ_t *env, object_t *key)
{
  uint32_t hash = object_hash (key);
  if (hash >= env->size)
    raise_runtime_error ("Object hash for retrieval larger than table size");

  entry_t *bucket = env->entries[hash], *e;
  for (e = bucket; e && !object_equals (e->key, key); e = e->next)
    ;

  if (!e)
    raise_runtime_error ("Key given for retrieval does not exist in table");

  return e->value;
}

void
environ_delete (environ_t *env, object_t *key)
{
  uint32_t hash = object_hash (key);
  if (hash >= env->size)
    raise_runtime_error ("Object hash for deletion larger than table size");

  entry_t *bucket = env->buckets[hash], *e, *e_prev;
  for (e = bucket; e && !object_equals (e->key, key);)
    {
      e_prev = e;
      e = e->next;
    }

  if (!e)
    raise_runtime_error ("Key given for deletion does not exist in table");

  e_prev->next = e->next;
}
