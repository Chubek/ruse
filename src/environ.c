#include <stdlib.h>
#include <string.h>

#include "object.h"

#define ENVIRON_GROWTH_FACTOR 0.65

void
environ_grow_if_should (environ_t **envp)
{
  if ((*envp)->count / (*envp)->size <= ENVIRON_GROWTH_RATE)
    return;

  environ_t *new_env = malloc (sizeof (environ_t));
  new_env->entries = calloc ((*envp)->size * 2, sizeof (entry_t));
  new_env->size = (*envp)->size * 2;
  new_env->count = 0;

  for (size_t i = 0; i < (*envp)->size; i++)
    {
      for (entry_t *e = (*envp)->entries[i]; e; e = e->next)
        {
          if (!e)
            continue;
          environ_insert (new_env, e->key, e->value);
        }
    }

  free ((*envp)->entries);
  free (*envp);
  *envp = new_env;
}

void
environ_insert (environ_t *env, object_t *key, object_t *value)
{
  environ_grow_if_should (&env);
  uint32_t hash = object_hash (key) % env->size;

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
  uint32_t hash = object_hash (key) % env->size;

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
