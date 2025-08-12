#ifndef ENVIRON_H
#define ENVIRON_H

#include "object.h"

void environ_grow_if_should (environ_t **envp);
void environ_insert (environ_t *env, object_t *key, object_t *value);
object_t *environ_retrieve (environ_t *env, object_t *key);
void environ_delete (environ_t *env, object_t *key);

#endif
