#ifndef RUNTIME_H
#define RUNTIME_H

#include "GC.h"
#include "Obj.h"

typedef enum {
  MODE_NORMAL,
  MODE_DEF,
  MODE_FUNC_CALL,
} Mode;

typedef struct {
  int depth;
  Obj *p; // the program counter
  Mode mode;
} Frame;

typedef struct {
  GC *gc;
  Obj *global_env;
  Obj *nil;
  Frame frames[128];
  int top_frame;
} Runtime;

Runtime *runtime_new();
void runtime_delete(Runtime *r);

void runtime_eval(Runtime *r, const char *source);
void runtime_inspect_env(Runtime *r);

#endif
