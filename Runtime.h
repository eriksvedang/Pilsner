#ifndef RUNTIME_H
#define RUNTIME_H

#include "GC.h"
#include "Obj.h"

typedef struct {
  GC *gc;
  Obj *global_env;
} Runtime;

Runtime *runtime_new();
void runtime_delete(Runtime *r);

void runtime_eval(Runtime *r, const char *source);
void runtime_inspect_env(Runtime *r);

#endif
