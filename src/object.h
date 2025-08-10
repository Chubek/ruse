#ifndef OBJECT_H
#define OBJECT_H

#define POSIX_SOURCE
#define POSIX_C_SOURCE
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>

#include "ast.h"

typedef struct Object object_t;
typedef struct Pair pair_t;
typedef struct Closure closure_t;
typedef struct Port port_t;
typedef struct Environ environ_t;
typedef struct Vector vector_t;
typedef struct Bytevector bytevector_t;
typedef struct Procedure procedure_t;
typedef struct Entry entry_t;

typedef object_t *(*primfn_t) (object_t *args, object_t *env);

struct Pair
{
  object_t *first;
  object_t *rest;
};

struct Port
{
  bool read;
  bool write;
  FILE *stream;
  const char fpath[PATH_MAX + 1];
};

struct Environ
{
  struct Entry
  {
    const char *key;
    object_t *value;
    entry_t *next;
  } *entries;
  size_t size;
  size_t count;
  int log2;
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
  astnode_t *body;
};

struct Procedure
{
  bool closure;
  union
  {
    closure_t *v_closure;
    primfn_t *v_prim;
  };
};

struct Object
{
  enum
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
    OBJ_Symbol,
    OBJ_String,
    OBJ_Label,
    OBJ_Synobj,
    OBJ_Character,
    OBJ_Procedure,
  } type;

  union
  {
    pair_t *v_pair;
    port_t *v_port;
    environ_t *v_environ;
    vector_t *v_vector;
    bytevector_t *v_bytevector;
    procedure_t *v_procedure;

    intmax_t v_integer;
    double v_real;
    const uint8_t *v_label, *v_symbol, *v_string;
  };
};

#endif
