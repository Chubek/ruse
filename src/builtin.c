#include <complex.h>
#include <inttypes.h>
#include <math.h>

#include "eval.h"
#include "heap.h"
#include "object.h"

#define PROMOTED_TO_NONE 0
#define PROMOTED_TO_REAL 1
#define PROMOTED_TO_COMPLEX 2

typedef int promotion_t;

extern heap_t *current_heap;
extern object_t *object_nil;
extern object_t *object_true;
extern object_t *object_false;

static inline object_t *
deref_symbols (object_t *args, object_t *env)
{
  for (object_t *a = args; a; a = a->next)
    {
      if (a->type == OBJ_Symbol)
        {
          object_t **ap = &a;
          object_t *ref = environ_retrieve (env->v_environ, a);
          if (ref == NULL)
            raise_runtime_error ("Symbol does not exist");
          *ap = ref;
        }
    }
}

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

  deref_symbols (args, env);
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
      return object_nil;
    }
}

object_t *
builtin_subtract (object_t *args, object_t *env)
{
  if (!args)
    return object_new_integer (0, current_heap);

  deref_symbols (args, env);
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
      return object_nil;
    }
}

object_t *
builtin_multiply (object_t *args, object_t *env)
{
  deref_symbols (args, env);
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
      return object_nil;
    }
}

object_t *
builtin_divide (object_t *args, object_t *env)
{
  if (!args)
    return object_new_integer (0, current_heap);

  deref_symbols (args, env);
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
      return object_nil;
    }
}

object_t *
builtin_quotient (object_t *args, object_t *env)
{
  if (!args)
    return object_new_integer (0, current_heap);

  deref_symbols (args, env);
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

  deref_symbols (args, env);
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

  deref_symbols (args, env);
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

  deref_symbols (args, env);
  promotion_t _ = assess_promotion (args, "=");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            return object_false;
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

  return result ? object_true : object_false;
}

object_t *
builtin_nums_not_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("=/= takes at least two arguments");

  deref_symbols (args, env);
  promotion_t _ = assess_promotion (args, "=");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            return object_new_bool (true, );
          switch (a->type)
            {
            case OBJ_Integer:
              result = a->v_integer != b->v_integer;
              continue;
            case OBJ_Real:
              result = a->v_real != b->v_real;
              continue;
            case OBJ_Complex:
              result = a->v_complex != b->v_complex;
              i continue;
            }
        }
    }

  return result ? object_true : object_false;
}

object_t *
builtin_nums_greater (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("> takes at least two arguments");

  deref_symbols (args, env);
  promotion_t _ = assess_promotion (args, ">");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            return object_false;
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

  return result ? object_true : object_false;
}

object_t *
builtin_nums_greater_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error (">= takes at least two arguments");

  deref_symbols (args, env);
  promotion_t _ = assess_promotion (args, ">=");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            return object_false;
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

  return result ? object_true : object_false;
}

object_t *
builtin_nums_lesser (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("< takes at least two arguments");

  deref_symbols (args, env);
  promotion_t _ = assess_promotion (args, "<");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            return object_false;
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

  return result ? object_true : object_false;
}

object_t *
builtin_nums_lesser_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("<= takes at least two arguments");

  deref_symbols (args, env);
  promotion_t _ = assess_promotion (args, "<=");

  bool result = false;
  for (object_t *a = args; a; a = a->next)
    {
      for (object_t *b = args->next; b; b = b->next)
        {
          if (a->type != b->type)
            return object_false;
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

  return result ? object_true : object_false;
}

object_t *
builtin_eq (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("eq? requires two arguments");

  deref_symbols (args, env);
  bool result = (args == args->next);
  return result ? object_true : object_false;
}

object_t *
builtin_eqv (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtim_error ("eqv? takes two arguments");

  deref_symbols (args, env);
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
    raise_runtime_error ("equal? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type != OBJ_Vector || args->type != OBJ_Bytevector
        || obj->type != OBJ_Pair))
    return builtin_eqv (args, env);

  if (args->type == OBJ_Vector)
    return builtin_vectors_equal (args, env);
  else if (args->type == OBJ_Bytevector)
    return builtin_bytevectors_equal (args, env);
  else if (args->type == OBJ_Pair)
    return builtin_pairs_equal (args, env);

  return object_nil;
}

object_t *
builtin_strings_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("str=? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_String || args->next->type == OBJ_String))
    raise_runtime_error ("str=? takes two string arguments");

  return object_new_bools (object_equals (args, args->next), current_heap);
}

object_t *
builtin_synobjs_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("syntax=? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Synobj || args->next->type == OBJ_Synobj))
    raise_runtime_error ("syntax=? takes two syntax arguments");

  for (object_t *d1 = args->v_synobj->datum; d1; d1 = d1->next)
    {
      for (object_t *d2 = args->next->v_synobj->datum; d2; d2 = d2->next)
        {
          bool result = object_equals (d1, d2);
          if (!result)
            return object_false;
        }
    }
  return object_true;
}

object_t *
builtin_characters_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("char=? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Character || args->next->type == OBJ_Character))
    raise_runtime_error ("char=? takes two character arguments");

  return args->v_character == args->next->v_character ? object_true
                                                      : object_false;
}

object_t *
builtin_characters_greater (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("char>? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Character || args->next->type == OBJ_Character))
    raise_runtime_error ("char>? takes two character arguments");

  return args->v_character > args->next->v_character ? object_true
                                                     : object_false;
}

object_t *
builtin_characters_greater_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("char>=? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Character || args->next->type == OBJ_Character))
    raise_runtime_error ("char>=? takes two character arguments");

  return args->v_character >= args->next->v_character ? object_true
                                                      : object_false;
}

object_t *
builtin_characters_lesser (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("char<? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Character || args->next->type == OBJ_Character))
    raise_runtime_error ("char<? takes two character arguments");

  return args->v_character < args->next->v_character ? object_true
                                                     : object_false;
}

object_t *
builtin_characters_lesser_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("char<=? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Character || args->next->type == OBJ_Character))
    raise_runtime_error ("char<=? takes two character arguments");

  return args->v_character <= args->next->v_character ? object_true
                                                      : object_false;
}

object_t *
builtin_vectors_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("vector=? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Vector || args->next->type == OBJ_Vector))
    raise_runtime_error ("vector=? takes two vector arguments");

  for (size_t i = 0; i < args->v_vector->count; i++)
    {
      for (size_t j = 0; j < args->next->v_vector->count; j++)
        {
          bool result = object_equals (args->v_vector->vals[i], current_heap);
                                       args->next->v_vector->vals[i]);
                                       if (!result)
                                         return object_false;
        }
    }
  return object_true;
}

object_t *
builtin_bytevectors_equal (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("bytevector=? takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Bytevector || args->next->type == OBJ_Bytevector))
    raise_runtime_error ("bytevector=? takes two bytevector arguments");

  for (size_t i = 0; i < args->v_bytevector->count; i++)
    {
      for (size_t j = 0; j < args->next->v_bytevector->count; j++)
        {
          bool result = args->v_bytevector->vals[i]
                        == args->next->v_bytevector->vals[j];
          if (!result)
            return object_false;
        }
    }
  return object_true;
}

object_t *
builtin_string_ref (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("string-ref takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_String || args->next->type == OBJ_Integer))
    raise_runtime_error ("string-ref takes a string, and an integer argument");

  const char32_t *buffz = args->v_buffz;
  intmax_t idx = args->next->v_integer;

  return object_new_character (buffz[idx], current_heap);
}

object_t *
builtin_string_length (object_t *args, object_t *env)
{
  if (!args)
    raise_runtime_error ("string-length takes one argument");

  deref_symbols (args, env);
  if (args->type != OBJ_String)
    raise_runtime_error ("string-length takes a string argument");

  const char32_t *buffz = args->v_buffz;
  intmax_t length = u32strlen (buffz);

  return object_new_integer (length, current_heap);
}

object_t *
builtin_string_append (object_t *args, object_t *env)
{
  if (!args)
    return object_nil;

  if (!args->next)
    return args;

  deref_symbols (args, env);

  if (args->type != OBJ_String)
    raise_runtime_error ("string-append takes a string argument");

  char32_t *buffz = u32strndup (args->v_string, -1);
  for (object_t *a = args->next; a; a = a->next)
    {
      if (a->type != OBJ_String)
        raise_runtime_error ("string-append takes string arguments");

      const char32_t *a_buffz = u32strndup (a->v_string, -1);
      buffz = u32strncat (buffz, a_buffz, -1);
    }

  return object_new_string ((const char32_t *)buffz, current_heap);
}

object_t *
builtin_substring (object_t *args, object_t *env)
{
  if (!args | !args->next || !args->next->next)
    raise_runtime_error ("substring takes three arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_String || args->next->type == OBJ_Integer
        || args->next->next->type == OBJ_Integer))
    raise_runtime_error ("substring takes a string, an two integer arguments");

  const char32_t *buffz = args->v_string;
  intmax_t start = args->next->v_integer;
  intmax_t end = args->next->next->v_integer;

  return object_new_string (u32substring (buffz, start, end), current_heap);
}

object_t *
builtin_list_ref (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("list-ref takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Pair || args->next->type == OBJ_Integer))
    raise_runtime_error ("list-ref takes a list, and an integer as argument");

  intmax_t idx = args->next->v_integer;
  object_t *current = args;

  while (idx && current->type == OBJ_Pair && current->rest->type != OBJ_Nil)
    {
      idx--;
      current = current->rest;
    }

  if (current && current->type == OBJ_Pair)
    return current->first;

  return object_nil;
}

object_t *
builtin_vector_ref (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("vector-ref takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Vector || args->next->type == OBJ_Integer))
    raise_runtime_error ("vector-ref takes a vector, and an integer argument");

  vector_t *vec = args->v_vector;
  intmax_t idx = args->next->v_integer;

  return vec->vals[idx];
}

object_t *
builtin_bytevector_ref (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("bytevector-ref takes two arguments");

  deref_symbols (args, env);
  if (!(args->type == OBJ_Bytevector || args->next->type == OBJ_Integer))
    raise_runtime_error (
        "bytevector-ref takes a bytevector, and an integer argument");

  bytevector_t *bvec = args->v_bytevector;
  intmax_t idx = args->next->v_integer;

  return object_new_byte (bvec->vals[idx], current_heap);
}

object_t *
builtin_cons (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("cons takes two arguments");

  deref_symbols (args, env);
  object_t *first = args;
  object_t *rest = args->next;

  return object_new_pair (first, rest, current_heap);
}

object_t *
builtin_car (object_t *args, object_t *env)
{
  if (!args)
    raise_runtime_error ("car takes one argument");

  deref_symbols (args, env);
  if (args->type != OBJ_Pair)
    raise_runtime_error ("car takes a pair argument");

  return args->v_pair->first;
}

object_t *
builtin_cdr (object_t *args, object_t *env)
{
  if (!args)
    raise_runtime_error ("cdr takes one argument");

  deref_symbols (args, env);
  if (args->type != OBJ_Pair)
    raise_runtime_error ("cdr takes a pair argument");

  return args->v_pair->rest;
}

object_t *
builtin_length (object_t *args, object_t *env)
{
  if (!args)
    raise_runtime_error ("length takes one argument");

  deref_symbols (args, env);
  if (args->type != OBJ_Pair)
    raise_runtime_error ("length takes a list as argument");

  intmax_t len = 1;
  object_t *current = args;

  while (current->type == OBJ_Pair && current->rest->type != OBJ_Nil)
    {
      len++;
      current = current->rest;
    }

  return object_new_integer (len, current_heap);
}

object_t *
builtin_list (object_t *args, object_t *env)
{
  if (!args)
    raise_runtime_error ("list takes at least one argument");

  object_t *result = object_new_pair (args, object_nil, current_heap);
  object_t *tail = result;
  args = args->next;

  while (args)
    {
      object_t *new_pair = object_new_pair (args, object_nil, current_heap);
      tail->v_pair->rest = new_pair;
      tail = new_pair;
      args = args->next;
    }

  return result;
}

object_t *
builtin_append (object_t *args, object_t *env)
{
  deref_symbols (args, env);

  if (!args)
    return object_nil;

  if (!args->next)
    return args;

  size_t num_args = 1;
  object_t *cursor = args;
  for (cursor = args->next; cursor; cursor = cursor->next)
    num_args++;

  object_t **args_array = malloc (num_args * sizeof (object_t *));
  size_t i = 0;
  for (cursor = args; cursor; cursor = cursor->next)
    args_array[i++] = cursor;

  object_t *result = args_array[num_args - 1];
  for (size_t j = num_args - 2; j >= 0; j--)
    {
      object_t *lst = args_array[j];

      if (lst->type == OBJ_Nil)
        continue;

      if (lst->type != OBJ_Pair)
        raise_runtime_error ("append argument is not a list");

      object_t *new_head
          = object_new_pair (lst->v_pair->first, NULL, current_heap);
      object_t *new_tail = new_head;
      object_t *old_rest = lst->v_pair->rest;

      while (old_rest->type == OBJ_Pair)
        {
          object_t *new_pair
              = object_new_pair (old_rest->v_pair->first, NULL, current_head);
          new_tail->v_pair->rest = new_pair;
          new_tail = new_pair;
          old_rest = old_rest->v_pair->rest
        }

      if (old_rest->type != OBJ_Nil)
        raise_runtime_error (
            "append was given non-proper list in non-final argument");

      new_tail->v_pair->rest = result;
      result = new_head;
    }

  free (args_array);
  return result;
}

object_t *
builtin_set (object_t *args, object_t *env)
{
  if (!args || !args->next)
    raise_runtime_error ("set! takes two arguments");

  if (args->type != OBJ_Symbol)
    raise_runtime_error ("set! takes a symbol as first argument");

  object_t *key = args;
  object_t *val = args->next;

  environ_install (env->v_environ, key, val);

  return object_nil;
}

object_t *
builtin_apply (object_t *args, object_t *env)
{
  if (!args)
    raise_runtime_error ("apply takes at least one argument");

  deref_symbols (args, env);
  if (args->type != OBJ_Procedure)
    raise_runtime_error ("apply takes a procedure argument");

  procedure_t *proc = args->v_procedure;
  object_t *proc_args = args->next;

  if (proc->closure)
    {
      eval_closure (proc->value->v_closure, proc_args, env);
    }
  else
    {
      eval_builtin (proc->value->v_builtin, proc_args, env);
    }
}
