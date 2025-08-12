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
typedef struct Formal formal_t;
typedef struct Builtin builtin_t;
typedef struct Conti conti_t;

typedef object_t *(*primfn_t) (object_t *args, object_t *env);

typedef enum ObjectType objtype_t;

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
    OBJ_Builtin,
    OBJ_Formal,
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

    intmax_t v_integer;
    double v_real;
    double complex v_complex;
    const uint8_t *v_buffz;
    bool v_bool;
  };

  uint32_t hash;
  bool marked;
  object_t *next, *tail;
};


object_t *object_new (objtype_t type, void *value);
void object_append (object_t *head, object_t *newobj);
void object_delete (object_t *obj);
uint32_t object_hash (object_t *obj);
bool object_equals (object_t *obj1, object_t *obj2);

#endif
