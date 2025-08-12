#include <complex.h>
#include <math.h>

#include "environ.h"
#include "heap.h"
#include "object.h"

extern heap_t *current_heap;

object_t *
builtin_add (object_t *args, object_t *env)
{
  double complex result = 0.0;
  for (object_t *a = args; a; a = a->next)
    {
      switch (a->type)
        {
        case OBJ_Integer:
          result += a->v_integer;
          continue;
        case OBJ_Real:
          result += a->v_real;
          continue;
        case OBJ_Complex:
          result += a->v_complex;
          continue;
        default:
          raise_runtime_error ("Can only add numeric types together");
          break;
        }
    }

  return object_new_complex (result, current_heap);
}

object_t *
builtin_sub (object_t *args, object_t *env)
{
  double complex result = 0.0;
  bool init = false;
  for (object_t *a = args; a; a = a->next)
    {
      switch (a->type)
        {
        case OBJ_Integer:
          if (!init)
            {
              result = a->v_integer;
              init = true;
              continue;
            }
          result -= a->v_integer;
          continue;
        case OBJ_Real:
          if (!init)
            {
              result = a->v_real;
              init = true;
              continue;
            }
          result -= a->v_real;
          continue;
        case OBJ_Complex:
          if (!init)
            {
              result = a - v_complex;
              init = true;
              continue;
            }
          result -= a->v_complex;
          continue;
        default:
          raise_runtime_error ("Can only subtract numeric types together");
          break;
        }
    }

  return object_new_complex (result, current_heap);
}

object_t *
builtin_mul (object_t *args, object_t *env)
{
  double complex result = 1.0;
  for (object_t *a = args; a; a = a->next)
    {
      switch (a->type)
        {
        case OBJ_Integer:
          result *= a->v_integer;
          continue;
        case OBJ_Real:
          result *= a->v_real;
          continue;
        case OBJ_Complex:
          result *= a->v_complex;
          continue;
        default:
          raise_runtime_error ("Can only multiply numeric types together");
          break;
        }
    }

  return object_new_complex (result, current_heap);
}

object_t *
builtin_div (object_t *args, object_t *env)
{
  if (args->next == NULL)
    raise_runtime_error ("Division needs a denumerator");

  object_t *num = args;
  objec_t *denum = args->next;
  double complex result = 0;

  switch (num->type)
    {
    case OBJ_Integer:
      result = num->v_integer;
      break;
    case OBJ_Real:
      result = num->v_real;
      break;
    case OBJ_Complex:
      result = num->v_complex;
      break;
    default:
      raise_runtime_error ("Division only works on numeric values");
      break;
    }

  if (denum->v_integer == 0 || denum->v_real == 0.0 || denum->v_complex == 0.0)
    raise_runtime_error ("Division by zero");

  switch (denum->type)
    {
    case OBJ_Integer:
      result /= denum->v_integer;
      break;
    case OBJ_Real:
      result /= denum->v_real;
      break;
    case OBJ_Complex:
      result /= denum->v_complex;
      break;
    default:
      raise_runtim_error ("Division needs a numeric denumerator");
      break;
    }

  return object_new_complex (result, current_heap);
}

object_t *
builtin_mod (object_t *args, object_t *env)
{
  if (args->next == NULL)
    raise_runtime_error ("Modulo needs a denumerator");

  object_t *num = args;
  object_t *denum = args->next;

  if (!(num->type == OBJ_Integer && denum->type == OBJ_Integer))
    raise_runtime_error ("Modulo takes two integers");

  intmax_t inum = abs (num->v_integer);
  intmax_t idenum = abs (denum->v_integer);

  if (idenum == 0)
    raise_runtime_error ("Division by zero");

  return object_new_integer (inum % idenum, current_heap);
}

object_t *
builtin_rem (object_t *args, object_t *env)
{
  if (args->next == NULL)
    raise_runtime_error ("Remainder needs a denumerator");

  object_t *num = args;
  object_t *denum = args->next;

  if (!(num->type == OBJ_Integer && denum->type == OBJ_Integer))
    raise_runtime_error ("Remainder takes two integers");

  intmax_t inum = abs (num->v_integer);
  intmax_t idenum = abs (denum->v_integer);

  if (idenum == 0)
    raise_runtime_error ("Division by zero");

  int sign = denum->v_integer / denum->v_integer;

  return object_new_integer ((inum % idenum) * sign, current_heap);
}
