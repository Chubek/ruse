#include "object.h"

object_t *
object_new (objtype_t type, void *value)
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
      fclose (obj->v_port->stream);
      object_delete (obj->v_port);
      free (obj->v_port);
    case OBJ_Pair:
      object_delete (obj->v_pair->first);
      object_delete (obj = > v_pair->rest);
      free (obj->v_pair);
      break;
    }

  free (obj);
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
