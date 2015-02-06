#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOG 0
#define STACK_MAX 512

typedef enum {
  CONS,
  SYMBOL,
} Type;

typedef struct Obj {
  Type type;
  struct Obj *next;
  bool reachable;
  union {
    // CONS
    struct Obj *car, *cdr;
    // SYMBOL
    const char *name;
  };
} Obj;

typedef struct {
  Obj *stack[STACK_MAX];
  int stackSize;
  Obj *firstObj; // sweeping starts here
} GC;

void error(const char *message) {
  printf("ERROR: %s", message);
  exit(1);
}

const char *type_to_str(Type type) {
  if(type == CONS) return "CONS";
  else if(type == SYMBOL) return "SYMBOL";
  else return "UNKNOWN";
}

const char *obj_to_str(Obj *o) {
  if(o->type == CONS) {
    return "CONS";
  }
  else if(o->type == SYMBOL) {
    return o->name;
  }
  else {
    error("Uknown type.");
    return NULL;
  }
}

void gc_stack_push(GC *gc, Obj *o) {
  if(gc->stackSize >= STACK_MAX) error("Stack overflow.");
  gc->stack[gc->stackSize++] = o;
}

Obj *gc_stack_pop(GC *gc) {
  if(gc->stackSize < 0) error("Stack underflow.");
  return gc->stack[--gc->stackSize];
}

Obj *gc_get_obj(GC *gc, Type type) {
  Obj *o = malloc(sizeof(Obj));
  o->reachable = false;
  o->type = type;

  #if LOG
  printf("Created obj %p of type %s.\n", o, type_to_str(o->type));
  #endif
  
  // Put new object first in the linked list containing all objects:
  o->next = gc->firstObj; 
  gc->firstObj = o;
  
  return o;
}

Obj *gc_get_cons(GC *gc, Obj *car, Obj *cdr) {
  Obj *o = gc_get_obj(gc, CONS);
  o->car = car;
  o->cdr = cdr;
  return o;
}

Obj *gc_get_symbol(GC *gc, const char *name) {
  Obj *o = gc_get_obj(gc, SYMBOL);
  o->name = name;
  return o;
}

void mark(Obj *o) {
  if(o->reachable) {
    // This one has already been visited by mark(), avoid infinite loops
    return;
  }
  
  o->reachable = true;
  
  if (o->type == CONS) {
    if(o->car) {
      mark(o->car);
    }
    if(o->cdr) {
      mark(o->cdr);
    }
  }
}

void gc_init(GC *gc) {
  gc->stackSize = 0;
  gc->firstObj = NULL;
}

void gc_collect(GC *gc) {
  // Objects on the stack are all 'roots', i.e. they get automatically marked
  for (int i = 0; i < gc->stackSize; i++) {
    mark(gc->stack[i]);
  }

  int aliveCount = 0;
  int freeCount = 0;
  
  // Sweep!
  Obj** obj = &gc->firstObj;
  while (*obj) {
    if ((*obj)->reachable) {
      Obj* reached = *obj;
      reached->reachable = false; // reached this time, unmark it for future sweeps
      obj = &(reached->next); // pointer to the next object
      aliveCount++;
    } else {
      Obj* unreached = *obj;
      *obj = unreached->next; // change the pointer in place, *THE MAGIC*
      free(unreached);
      freeCount++;
    }
  }

  #if LOG
  printf("Sweep done, %d objects freed and %d object still alive.\n", freeCount, aliveCount);
  #endif
}

void test1();

int main(int argc, char *argv[])
{
  GC gc;
  gc_init(&gc);

  
  gc_collect(&gc);

  test1();
  
  return 0;
}



/* TESTS */

void test1() {
  GC gc;
  gc_init(&gc);

  Obj *sym1 = gc_get_symbol(&gc, "sym1");
  Obj *sym2 = gc_get_symbol(&gc, "sym2");
  Obj *sym3 = gc_get_symbol(&gc, "sym3");
  Obj *sym4 = gc_get_symbol(&gc, "sym4");
  
  Obj *cell1 = gc_get_cons(&gc, NULL, NULL);
  Obj *cell2 = gc_get_cons(&gc, NULL, NULL);
  Obj *cell3 = gc_get_cons(&gc, NULL, NULL);
  Obj *cell4 = gc_get_cons(&gc, NULL, NULL);

  
  cell1->car = cell3;
  cell2->cdr = sym2;
  cell3->car = cell4;
  cell3->cdr = sym3;
  cell4->car = sym1;
  cell4->cdr = sym2;

  gc_stack_push(&gc, cell1);

  gc_collect(&gc);
  gc_collect(&gc);
  gc_collect(&gc);
}
