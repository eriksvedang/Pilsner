#ifndef GC_H
#define GC_H

#include "Obj.h"

#define STACK_MAX 512

typedef struct {
  Obj *stack[STACK_MAX];
  int stackSize;
  Obj *firstObj; // linked list of all objects
} GC;

typedef struct {
  int alive;
  int freed;
} GCResult;

void gc_init(GC *gc);
GCResult gc_collect(GC *gc);

// Stack
void gc_stack_push(GC *gc, Obj *o);
Obj *gc_stack_pop(GC *gc);

// Make objects
Obj *gc_make_cons(GC *gc, Obj *car, Obj *cdr);
Obj *gc_make_symbol(GC *gc, const char *name);
Obj *gc_make_func(GC *gc, const char *name, void *f);
Obj *gc_make_number(GC *gc, double x);
Obj *gc_make_string(GC *gc, const char *text);
Obj *gc_make_lambda(GC *gc, Obj *env, Obj *args, Obj *body);

#endif
