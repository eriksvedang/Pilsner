#ifndef RUNTIME_H
#define RUNTIME_H

#include "GC.h"
#include "Obj.h"

typedef enum {
  MODE_NORMAL,
  MODE_DEF,
  MODE_FUNC_CALL,
  MODE_LAMBDA_RETURN,
  MODE_IMMEDIATE_RETURN,
} FrameMode;

typedef struct {
  int depth;
  Obj *p; // the program counter
  FrameMode mode;
  int arg_count; // this is used when entering MODE_FUNC_CALL
  char name[128];
} Frame;

typedef enum {
  RUNTIME_MODE_RUN,
  RUNTIME_MODE_BREAK,
  RUNTIME_MODE_FINISHED,
} RuntimeMode;

typedef struct {
  GC *gc;
  Obj *global_env;
  Obj *nil;
  Frame frames[128];
  int top_frame;
  RuntimeMode mode;
} Runtime;

Runtime *runtime_new();
void runtime_delete(Runtime *r);

void runtime_eval(Runtime *r, const char *source);
void runtime_inspect_env(Runtime *r);

#endif
