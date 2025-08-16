#ifndef OBJECT_H
#define OBJECT_H

#define POSIX_SOURCE
#define POSIX_C_SOURCE
#include <complex.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <uchar.h>

#include "heap.h"

#define MAX_PRIM_NAME 16

typedef struct Object object_t;
typedef struct Pair pair_t;
typedef struct Closure closure_t;
typedef struct Port port_t;
typedef struct Environ environ_t;
typedef struct Vector vector_t;
typedef struct Bytevector bytevector_t;
typedef struct Procedure procedure_t;
typedef struct Entry entry_t;
typedef struct Synobj synobj_t;
typedef struct Formal formal_t;
typedef struct Builtin builtin_t;
typedef struct Conti conti_t;
typedef struct Stack stack_t;
typedef struct Symbol symbol_t;

typedef object_t *(*primfn_t) (object_t *args, object_t *env);

typedef enum ObjectType objtype_t;
typedef enum OpCode opcode_t;

struct Pair
{
  object_t *first;
  object_t *rest;
};

struct Port
{
  bool read;
  bool write;
  bool append;
  bool binary;
  FILE *stream;
  bool stdio;
  const char fpath[PATH_MAX + 1];
};

struct Builtin
{
  const char name[MAX_PRIM_NAME + 1];
  primfn_t *fn;
};

struct Environ
{
  struct Entry
  {
    object_t *key;
    object_t *value;
    entry_t *next;
  } *entries;
  size_t size;
  size_t count;
  environ_t *parent;
};

struct Vector
{
  object_t *vals;
  size_t size;
  size_t count;
};

struct Bytevector
{
  uint8_t *vals;
  size_t size;
  size_t count;
};

struct Closure
{
  object_t *formals;
  object_t *env;
  object_t *body;
};

struct Procedure
{
  bool closure;
  object_t *value;
};

struct Formal
{
  bool varargs;
  bool ellipses;
  object_t *value;
};

struct Conti
{
  object_t *captured_stack;
};

struct Stack
{
  object_t **objs;
  size_t size;
  size_t count;
};

struct Synobj
{
  object_t *datum;
  object_t *env;
  const char srcfile[MAX_PATH + 1];
  size_t line, column;
};

struct Symbol
{
  const char32_t *id;
  int mark;
};

enum OpCode
{
  OP_Halt,
  OP_Refer,
  OP_Constant,
  OP_Close,
  OP_Test,
  OP_Assign,
  OP_Conti,
  OP_Nuate,
  OP_Frame,
  OP_Argument,
  OP_Apply,
  OP_Return,
};

struct Object
{
  enum ObjectType
  {
    OBJ_Pair,
    OBJ_Nil,
    OBJ_Bool,
    OBJ_Port,
    OBJ_Environ,
    OBJ_Vector,
    OBJ_Bytevector,
    OBJ_Integer,
    OBJ_Real,
    OBJ_Complex,
    OBJ_Symbol,
    OBJ_String,
    OBJ_Label,
    OBJ_Synobj,
    OBJ_Character,
    OBJ_Procedure,
    OBJ_Closure,
    OBJ_Conti,
    OBJ_Stack,
    OBJ_Builtin,
    OBJ_Formal,
    OBJ_OpCode,
  } type;

  union
  {
    pair_t *v_pair;
    port_t *v_port;
    environ_t *v_environ;
    vector_t *v_vector;
    bytevector_t *v_bytevector;
    procedure_t *v_procedure;
    formal_t *v_formal;
    builtin_t *v_builtin;
    closure_t *v_closure;
    conti_t *v_conti;
    stack_t *v_stack;
    symbol_t *v_symbol;
    synobj_t *v_synobj;

    opcode_t v_opcode;
    intmax_t v_integer;
    double v_real;
    double complex v_complex;
    const char32_t *v_buffz;
    bool v_bool;
    char32_t v_char;
  };

  uint32_t hash;
  bool marked;
  object_t *next, *tail;
};

object_t *object_new (objtype_t type, void *value, heap_t *heap);
void object_append (object_t *head, object_t *newobj);
void object_delete (object_t *obj);
uint32_t object_hash (object_t *obj);
bool object_equals (object_t *obj1, object_t *obj2);

object_t *object_new_pair (object_t *first, object_t *rest, heap_t *heap);
object_t *object_new_port (const char *path, bool read, bool write,
                           bool append, bool binary, heap_t *heap);
object_t *object_new_closure (object_t *formals, object_t *env, object_t *body,
                              heap_t *heap);

object_t *object_new_environ (environ_t *parent, size_t size, heap_t *heap);

object_t *object_new_vector (size_t size, heap_t *heap);
object_t *object_new_bytevector (size_t size, heap_t *heap);

object_t *object_new_procedure (bool closure, object_t *value, heap_t *heap);
object_t *object_new_formal (bool varargs, bool ellipses, object_t *value,
                             heap_t *heap);

object_t *object_new_builtin (const char *name, primfn_t *fn, heap_t *heap);

object_t *object_new_conti (object_t *captured_stack, heap_t *heap);

object_t *object_new_stack (size_t size, heap_t *heap);

object_t *object_new_symbol (const char32_t *id, size_t id_len, heap_t *heap);
object_t *object_new_synobj (object_t *datum, object_t *env, heap_t *heap);

object_t *object_new_integer (intmax_t value, heap_t *heap);
object_t *object_new_real (double value, heap_t *heap);
object_t *object_new_complex (double complex value, heap_t *heap);
object_t *object_new_bool (bool value, heap_t *heap);
object_t *object_new_character (uint32_t ch, heap_t *heap);

object_t *object_new_string (const char32_t *str, size_t str_len,
                             heap_t *heap);
object_t *object_new_label (const char32_t *lbl, size_t lbl_len, heap_t *heap);

object_t *object_new_synobj (object_t *val, heap_t *heap);
object_t *object_new_nil (heap_t *heap);

object_t *object_new_opcode (opcode_t opcode, heap_t *heap);

void stack_push (stack_t *stk, object_t *obj);
stack_t *stack_pop (stack_t *stk);

void environ_install (environ_t *env, object_t *key, object_t *value);
object_t *environ_retrieve (environ_t *env, environ_t *key);
void environ_delete (environ_t *env, object_t *key);

object_t *
create_list (heap_t *heap, size_t n, ...);

static inline object_t *
car (object_t *pair)
{
  if (pair->type != OBJ_Pair)
    raise_runtime_error ("car only works on pairs");

  return pair->v_pair->first;
}

static inline object_t *
cdr (object_t *pair)
{
  if (pair->type != OBJ_Pair)
    raise_runtime_error ("cdr only works on pairs");

  return pair->v_pair->rest;
}

static inline object_t *
cons (object_t *car, object_t *cdr)
{
  return object_new_pair (car, cdr);
}

static inline size_t
list_length (object_t *lst)
{
  size_t length = 1;
  while (lst->type == OBJ_Pair && lst->type != OBJ_Nil)
    {
      length++;
      lst = cdr (lst);
    }

  return length;
}

#endif
