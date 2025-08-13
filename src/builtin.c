#include <complex.h>
#include <inttypes.h>
#include <math.h>

#include "heap.h"
#include "object.h"

#define PROMOTED_TO_NONE 0
#define PROMOTED_TO_REAL 1
#define PROMOTED_TO_COMPLEX 2

typedef int promotion_t;

extern heap_t *current_heap;

static inline promotion_t
assess_promotion (object_t *args, const char *fnname)
{

  promotion_t promotion = PROMOTED_TO_NONE;
  for (object_t *a = args; a; a = a->next)
    {
      switch (a->type)
        {
        case OBJ_Integer:
          continue;
        case OBJ_Real:
          if (promotion < PROMOTED_TO_REAL)
            promotion = PROMOTED_TO_REAL;
          continue;
        case OBJ_Complex:
          if (promotion < PROMOTED_TO_COMPLEX)
            promotion = PROMOTED_TO_COMPLEX;
          break;
        default:
          raise_runtime_error ("%s only works on numeric values", fnname);
          break;
        }
    }

  return promotion;
}

object_t *
builtin_add (object_t *args, object_t *env)
{
  if (!args)
    return object_new_integer (0, current_heap);

  promotion_t promotion = assess_promotion (args, "Addition");

  switch (promotion)
    {
    case PROMOTED_TO_NONE:
      intmax_t result = 0;
      for (object_t *a = args; a; a = a->next)
        result += a->v_integer;
      return object_new_integer (result, current_heap);
    case PROMOTED_TO_REAL:
      double result = 0.0;
      for (object_t *a = args; a; a = a->next)
        {
          if (a->type == OBJ_Integer)
            result += a->v_integer;
          else
            result += a->v_real;
        }
      return object_new_real (result, current_heap);
    case PROMOTED_TO_COMPLEX:
      double complex result = 0.0 * I;
      for (object_t *a = args; a; a = a->next)
        {
          if (a->type == OBJ_Integer)
            result += a->v_integer;
          else if (a->type == OBJ_Real)
            result += a->v_real;
          else if (a->type == OBJ_Complex)
            result += a->v_complex;
        }
      return object_new_complex (result, current_heap);
    default:
      return NULL;
    }
}

object_t *
builtin_subtract (object_t *args, object_t *env)
{
  if (!args)
    return object_new_integer (0, current_heap);

  promotion_t promotion = assess_promotion (args, "Subtraction");
  switch (promotion)
    {
    case PROMOTED_TO_NONE:
      intmax_t result = args->v_integer;
      for (object_t *a = args->next; a; a = a->next)
        result -= a->v_integer;
      return object_new_integer (result, current_heap);
    case PROMOTED_TO_REAL:
      double result = args->v_real;
      for (object_t *a = args->next; a; a = a->next)
        {
          if (a->type == OBJ_Integer)
            result -= a->v_integer;
          else
            result -= a->v_real;
        }
      return object_new_real (result, current_heap);
    case PROMOTED_TO_COMPLEX:
      double complex result = args->v_complex;
      for (object_t *a = args->next; a; a = a->next)
        {
          if (a->type == OBJ_Integer)
            result -= a->v_integer;
          else if (a->type == OBJ_Real)
            result -= a->v_real;
          else
            result -= a->v_complex;
        }
      return object_new_complex (result, current_heap);
    default:
      return NULL;
    }
}

object_t *
builtin_multiply (object_t *args, object_t *env)
{
  promotion_t promotion = assess_promotion (args, "Multiplication");
  switch (promotion)
    {
    case PROMOTED_TO_NONE:
      intmax_t result = 1;
      for (object_t *a = args; a; a = a->next)
        result *= a->v_integer;
      return object_new_integer (result, current_heap);
    case ROMOTED_TO_REAL:
      double result = 1.0;
      for (object_t *a = args; a; a = a->next)
        {
          if (a->type == OBJ_Integer)
            result *= a->v_integer;
          else
            result *= a->v_real;
        }
      return object_new_real (result, current_heap);
    case PROMOTED_TO_COMPLEX:
      double complex result = 1.0 * I;
      for (object_t *a = args; a; a = a->next)
        {
          if (a->type == OBJ_Integer)
            result *= a->v_integer;
          else if (a->type == OBJ_Real)
            result *= a->v_real;
          else
            result *= a->v_complex;
        }
      return object_new_complex (result, current_heap);
    default:
      return NULL;
    }
}

object_t *
builtin_divide (object_t *args, object_t *env)
{
  if (!args)
    return object_new_integer (0, current_heap);

  promotion_t promotion = assess_promotion (args, "Division");
  switch (promotion)
    {
    case PROMOTED_TO_NONE:
      intmax_t result = args->v_integer;
      for (object_t *a = args->next; a; a = a->next)
        {
          if (a->v_integer == 0)
            raise_runtime_error ("Division by zero");
          result /= a->v_integer;
        }
      return object_new_integer (result, current_heap);
    case PROMOTED_TO_REAL:
      double result = args->v_real;
      for (object_t *a = args->next; a; a = a->next)
        {
          if (a->type == OBJ_Integer)
            {
              if (a->v_integer == 0)
                raise_runtime_error ("Division by zero");
              result -= a->v_integer;
            }
          else
            {
              if (a->v_real == 0.0)
                raise_runtime_error ("Division by zero");
              result -= a->v_real;
            }
        }
      return object_new_real (result, current_heap);
    case PROMOTED_TO_COMPLEX:
      double complex result = args->v_complex;
      for (object_t *a = args->next; a; a = a->next)
        {
          if (a->type == OBJ_Integer)
            {
              if (a->v_integer == 0)
                raise_runtime_error ("Division by zero");
              result -= a->v_integer;
            }
          else if (a->type == OBJ_Real)
            {
              if (a->v_real == 0.0)
                raise_runtime_error ("Division by zero");
              result -= a->v_real;
            }
          else
            {
              if (a->v_complex == 0.0 * I)
                raise_runtime_error ("Division by zero");
              result -= a->v_complex;
            }
        }
      return object_new_complex (result, current_heap);
    default:
      return NULL;
    }
}

object_t *
builtin_quotient (object_t *args, object_t *env)
{
  if (!args)
    return object_new_integer (0, current_heap);

  promotion_t promotion = assess_promotion (args, "Quotient");
  if (promotion != PROMOTED_TO_NONE)
    raise_runtime_error ("Quotient only accepts integral values");

  intmax_t result = args->v_integer;
  for (object_t *a = args->next; a; a = a->next)
    {
      if (a->v_integer == 0)
        raise_runtime_error ("Division by zero");
      result /= a->v_integer;
    }

  return object_new_integer (result, current_heap);
}

object_t *
builtin_modulo (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("Modulo takes 2 arguments");

  promotion_t promotion = assess_promotion (args, "Modulo");
  if (promotion != PROMOTED_TO_NONE)
    raise_runtime_error ("Modulo only accepts integral values");

  intmax_t dividend = imaxabs (args->v_integer);
  intmax_t divisor = imaxabs (args->next->v_integer);

  if (divisor == 0)
    raise_runtime_error ("Division by zero");

  intmax_t result = dividend % divisor;

  return object_new_integer (result, current_heap);
}

object_t *
builtin_remainder (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("Remainder takes two arguments");

  promotion_t promotion = assess_promotion (args, "Remainder");
  if (promotion != PROMOTED_TO_NONE)
    raise_runtime_error ("Remainder only accepts integral values");

  intmax_t dividend = args->v_integer;
  intmax_t divisor = args->next->v_integer;

  if (divisor == 0)
    raise_runtime_error ("Division by zero");

  int sign = dividend / dividend;

  intmax_t result = (imaxabs (dividend) % imaxabs (divisor)) * sign;

  return object_new_integer (result, current_heap);
}

object_t *
builtin_nums_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("= takes at least two arguments");

  promotion_t _ = assess_promotion (args, "=");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            {
              result = false;
              goto RET;
            }
          switch (a->type)
            {
            case OBJ_Integer:
              result = a->v_integer == b->v_integer;
              continue;
            case OBJ_Real:
              result = a->v_real == b->v_real;
              continue;
            case OBJ_Complex:
              result = a->v_complex == b->v_complex;
              i continue;
            }
        }
    }

RET:
  return object_new_bool (result, current_heap);
}

object_t *
builtin_nums_greater (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("> takes at least two arguments");

  promotion_t _ = assess_promotion (args, ">");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            {
              result = false;
              goto RET;
            }
          switch (a->type)
            {
            case OBJ_Integer:
              result = a->v_integer > b->v_integer;
              continue;
            case OBJ_Real:
              result = a->v_real > b->v_real;
              continue;
            case OBJ_Complex:
              result = a->v_complex > b->v_complex;
              continue;
            }
        }
    }

RET:
  return object_new_bool (result, current_heap);
}

object_t *
builtin_nums_greater_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error (">= takes at least two arguments");

  promotion_t _ = assess_promotion (args, ">=");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            {
              result = false;
              goto RET;
            }
          switch (a->type)
            {
            case OBJ_Integer:
              result = a->v_integer >= b->v_integer;
              continue;
            case OBJ_Real:
              result = a->v_real >= b->v_real;
              continue;
            case OBJ_Complex:
              result = a->v_complex >= b->v_complex;
              continue;
            }
        }
    }

RET:
  return object_new_bool (result, current_heap);
}

object_t *
builtin_nums_lesser (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("< takes at least two arguments");

  promotion_t _ = assess_promotion (args, "<");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            {
              result = false;
              goto RET;
            }
          switch (a->type)
            {
            case OBJ_Integer:
              result = a->v_integer < b->v_integer;
              continue;
            case OBJ_Real:
              result = a->v_real < b->v_real;
              continue;
            case OBJ_Complex:
              result = a->v_complex < b->v_complex;
              continue;
            }
        }
    }

RET:
  return object_new_bool (result, current_heap);
}

object_t *
builtin_nums_lesser_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("<= takes at least two arguments");

  promotion_t _ = assess_promotion (args, "<=");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            {
              result = false;
              goto RET;
            }
          switch (a->type)
            {
            case OBJ_Integer:
              result = a->v_integer <= b->v_integer;
              continue;
            case OBJ_Real:
              result = a->v_real <= b->v_real;
              continue;
            case OBJ_Complex:
              result = a->v_complex <= b->v_complex;
              continue;
            }
        }
    }

RET:
  return object_new_bool (result, current_heap);
}

object_t *
builtin_eq (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("eq? requires two arguments");

  bool result = (args == args->next);
  return object_new_bool (result, current_heap);
}

object_t *
builtin_eqv (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtim_error ("eqv? takes two arguments");

  if (args->type == OBJ_Integer || args->type == OBJ_Real
      || args->type == OBJ_Complex)
    return builtin_nums_equal (args, env);
  else if (args->type == OBJ_String || args->type == OBJ_Symbol)
    return builtin_strings_equal (args, env);
  else if (args->type == OBJ_Synobj)
    return builtin_synobjs_equal (args, env);
  else if (args->type == OBJ_Character)
    return builtin_characters_equal (args, env);
  else
    return builtin_eq (args, env);
}

object_t *
builtin_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("equal? takes two argument");

  if (!(args->type == OBJ_Vector || args->type != OBJ_Bytevector
        || obj->type != OBJ_Pair))
    return builtin_eqv (args, env);

  if (args->type == OBJ_Vector)
    return builtin_vectors_equal (args, env);
  else if (args->type == OBJ_Bytevector)
    return builtin_bytevectors_equal (args, env);
  else if (args->type == OBJ_Pair)
    return builtin_pairs_equal (args, env);

  return NULL;
}
