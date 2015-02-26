#ifndef GC_H
#define GC_H

#include "Obj.h"
#include "Bytecode.h"
#include "Pool.h"

#define STACK_MAX 512
#define GLOBAL_OBJ_COUNT 1
#define USE_MEMORY_POOL 0

#if GLOBAL_OBJ_COUNT
int g_obj_count;
#endif

typedef struct {
  Obj *stack[STACK_MAX];
  int stackSize;
  Obj *firstObj; // linked list of all objects
  #if USE_MEMORY_POOL
  Pool *pool;
  #endif
} GC;

typedef struct {
  int alive;
  int freed;
} GCResult;

GC *gc_new();
void gc_delete(GC *gc);
GCResult gc_collect(GC *gc);

// Stack
void gc_stack_push(GC *gc, Obj *o);
Obj *gc_stack_pop(GC *gc);
void gc_stack_print(GC *gc, bool show_bottom_frame);

// Make objects
Obj *gc_make_cons(GC *gc, Obj *car, Obj *cdr);
Obj *gc_make_symbol(GC *gc, const char *name);
Obj *gc_make_symbol_from_malloced_string(GC *gc, char *name);
Obj *gc_make_func(GC *gc, const char *name, void *f);
Obj *gc_make_number(GC *gc, double x);
Obj *gc_make_string(GC *gc, char *text);
Obj *gc_make_bytecode(GC *gc, Code *code);
Obj *gc_make_lambda(GC *gc, Obj *env, Obj *args, Obj *body, Code *code);

#endif
