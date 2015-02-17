#ifndef GC_H
#define GC_H

#include "Obj.h"
#include "Bytecode.h"

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
