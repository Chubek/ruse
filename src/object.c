#include <stdio.h>
#include <unistd.h>

#include "heap.h"
#include "object.h"

#define STACK_GROWTH_FACTOR 0.85

object_t *
object_new (objtype_t type, void *value, heap_t *heap)
{
  object_t *obj = calloc (1, sizeof (object_t));
  obj->type = type;
  obj->hash = 0;
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
      memmove (&obj->v_integer, value, sizeof (intmax_t));
      break;
    case OBJ_Real:
      memmove (&obj->v_real, value, sizeof (double));
      break;
    case OBJ_Complex:
      memmove (&obj->v_complex, value, sizeof (double complex));
      break;
    case OBJ_Bool:
      memmove (&obj->v_bool, value, sizeof (bool));
      break;
    case OBJ_Character:
      memmove (&obj->v_char, value, sizeof (char32_t));
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
    case OBJ_Builtin:
      obj->v_builtin = (builtin_t *)value;
      break;
    case OBJ_Formal:
      obj->v_formal = (formal_t *)value;
      break;
    case OBJ_Conti:
      obj->v_conti = (conti_t *)value;
    case OBJ_Stack:
      obj->v_stack = (stack_t *)value;
    default:
      break;
    }

  heap_add_to_pool (heap, obj);
  return obj;
}

void
object_delete (object_t *obj)
{
  if (!obj)
    return;

  object_t *next = obj->next;
  switch (obj->type)
    {
    case OBJ_Nil:
    case oBJ_Bool:
    case OBJ_Integer:
    case OBJ_Real:
    case OBJ_Complex:
    case OBJ_Builtin:
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
    case OBJ_Conti:
      object_delete (obj->v_conti->captured_stack);
      free (obj->v_conti);
      break;
    case OBJ_Stack:
      for (size_t i = 0; i < obj->v_stack->count; i++)
        object_delete (obj->v_stack->objs[i]);
      free (obj->v_stack);
      break;
    case OBJ_Procedure:
      object_delete (obj->v_procedure->value);
      free (obj->v_procedure);
      break;
    case OBJ_Port:
      if (!obj->v_port->stdio)
        fclose (obj->v_port->stream);
      free (obj->v_port);
    case OBJ_Pair:
      object_delete (obj->v_pair->first);
      object_delete (obj = > v_pair->rest);
      free (obj->v_pair);
      break;
    case OBJ_Stack:
      for (size_t i = 0; i < obj->v_stack->count; i++)
        object_delete (obj->v_stack->objs[i]);
      free (obj->v_stack->objs);
      free (obj->v_stack);
    }

  free (obj);
  object_delete (next);
}

void
object_append (object_t *head, object_t *newobj)
{
  if (!head)
    return;

  head->tail->next = newobj;
  head->tail = newobj;
}

uint32_t
object_hash (object_t *obj)
{
  if (obj->hash)
    return obj->hash;

  if (obj->type == OBJ_String || obj->type == OBJ_Symbol
      || obj->type == OBJ_Lable)
    obj->hash = fnv1b_hash32 (obj->v_buffz) + 1;
  else if (obj->type == OBJ_Real || obj->type == OBJ_Integer
           || obj->type == OBJ_Complex)
    obj->hash = splitmax_hash32 (obj->v_integer) + 1;
  else
    raise_runtime_error ("Unhashable object");

  return obj->hash;
}

bool
object_equals (object_t *obj1, object_t *obj2)
{
  if (obj1->type != obj2->type)
    return false;

  return object_hash (obj1) == object_hash (obj2);
}

object_t *
object_new_pair (object_t *first, object_t *rest, heap_t *heap)
{
  pair_t *pair = malloc (sizeof (pair_t));
  pair->first = first;
  pair->rest = rest;
  return object_new (OBJ_Pair, (void *)pair, heap);
}

object_t *
object_new_port (const char *path, bool read, bool write, bool append,
                 bool binary, heap_t *heap)
{
  port_t *port = malloc (sizeof (port_t));
  port->read = read;
  port->write = write;
  port->append = append;
  port->binary = binary;

  if ((int)path == STDIN_FILENO)
    {
      port->stream = stdin;
      port->stdio = true;
    }
  else if ((int)path == STDOUT_FILENO)
    {
      port->stream = stdout;
      port->stdio = true;
    }
  else if ((int)path == STDERR_FILENO)
    {
      port->stream = stderr;
      port->stdio = true;
    }
  else
    {
      const char flags[8] = { 0 };
      int n = 0;
      if (read)
        flags[n++] = 'r';
      if (write)
        flags[n++] = 'w';
      if (append)
        flags[n++] = 'a';
      if (binary)
        flags[n++] = 'b';
      port->stream = fopen (path, &flags[0]);
      strncat (&port->path[0], path, PATH_MAX);
      port->path[PATH_MAX] = '\0';
    }

  return object_new (OBJ_Port, (void *)port, heap);
}

object_t *
object_new_closure (object_t *formals, object_t *env, object_t *body,
                    heap_t *heap)
{
  closure_t *closure = malloc (sizeof (closure_t));
  closure->formals = formals;
  closure->env = env;
  closure->body = body;

  return object_new (OBJ_Closure, (void *)closure, heap);
}

object_t *
object_new_environ (size_t size, heap_t *heap)
{
  environ_t *env = malloc (sizeof (environ_t));
  env->entries = calloc (size, sizeof (entry_t)); // table of entries
  env->size = size;
  env->count = 0;
  return object_new (OBJ_Environ, env, heap);
}

object_t *
object_new_vector (size_t size, heap_t *heap)
{
  vector_t *vec = malloc (sizeof (vector_t));
  vec->vals = calloc (size, sizeof (object_t *));
  vec->size = size;
  vec->count = 0;
  return object_new (OBJ_Vector, vec, heap);
}

object_t *
object_new_bytevector (size_t size, heap_t *heap)
{
  bytevector_t *bv = malloc (sizeof (bytevector_t));
  bv->vals = calloc (size, sizeof (uint8_t));
  bv->size = size;
  bv->count = 0;
  return object_new (OBJ_Bytevector, bv, heap);
}

object_t *
object_new_procedure (bool closure, object_t *value, heap_t *heap)
{
  procedure_t *proc = malloc (sizeof (procedure_t));
  proc->closure = closure;
  proc->value = value;
  return object_new (OBJ_Procedure, proc, heap);
}

object_t *
object_new_formal (bool varargs, bool ellipses, object_t *value, heap_t *heap)
{
  formal_t *f = malloc (sizeof (formal_t));
  f->varargs = varargs;
  f->ellipses = ellipses;
  f->value = value;
  return object_new (OBJ_Formal, f, heap);
}

object_t *
object_new_builtin (const char *name, primfn_t fn, heap_t *heap)
{
  builtin_t *b = malloc (sizeof (builtin_t));
  strncpy ((char *)b->name, name, MAX_PRIM_NAME);
  ((char *)b->name)[MAX_PRIM_NAME] = '\0';
  b->fn = fn;
  return object_new (OBJ_Builtin, b, heap);
}

object_t *
object_new_conti (object_t *captured_stack, heap_t *heap)
{
  conti_t *c = malloc (sizeof (conti_t));
  c->captured_stack = captured_stack;
  return object_new (OBJ_Conti, c, heap);
}

object_t *
object_new_stack (size_t size, heap_t *heap)
{
  stack_t *s = malloc (sizeof (stack_t));
  s->objs = calloc (size, sizeof (object_t *));
  s->size = size;
  s->count = 0;
  return object_new (OBJ_Stack, s, heap);
}

object_t *
object_new_integer (intmax_t value, heap_t *heap)
{
  return object_new (OBJ_Integer, (void *)&value, heap);
}

object_t *
object_new_real (double value, heap_t *heap)
{
  return object_new (OBJ_Real, (void *)&value, heap);
}

object_t *
object_new_complex (double complex value, heap_t *heap)
{
  return object_new (OBJ_Complex, (void *)&value, heap);
}

object_t *
object_new_bool (bool value, heap_t *heap)
{
  return object_new (OBJ_Bool, (void *)&value, heap);
}

object_t *
object_new_character (char32_t ch, heap_t *heap)
{
  return object_new (OBJ_Character, (void *)&ch, heap);
}

object_t *
object_new_symbol (const uint8_t *utf8, heap_t *heap)
{
  uint8_t *dup = strdup ((const char *)utf8);
  return object_new (OBJ_Symbol, dup, heap);
}

object_t *
object_new_string (const uint8_t *utf8, heap_t *heap)
{
  uint8_t *dup = strdup ((const char *)utf8);
  return object_new (OBJ_String, dup, heap);
}

object_t *
object_new_label (const uint8_t *utf8, heap_t *heap)
{
  uint8_t *dup = strdup ((const char *)utf8);
  return object_new (OBJ_Label, dup, heap);
}

object_t *
object_new_synobj (object_t *val, heap_t *heap)
{
  return object_new (OBJ_Synobj, val, heap);
}

object_t *
object_new_nil (heap_t *heap)
{
  return object_new (OBJ_Nil, NULL, heap);
}

void
stack_push (stack_t *stk, object_t *obj)
{
  if (stk->count / stk->size >= STACK_GROWTH_FACTOR)
    {
      stk->size *= 2;
      stk->objs = realloc (stk->objs, stk->size * sizeof (object_t *));
    }
  stk->objs[stk->count++] = obj;
}

object_t *
stack_pop (stack_t *stk)
{
  if (stk->count == 0)
    raise_runtime_error ("Stack underflow");
  return stk->objs[--stk->count];
}
