#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
  CONS,
  SYMBOL,
} Type;

typedef struct Obj {
  Type type;
  struct Obj *next;
  bool reachable;
  // CONS
  struct Obj *car, *cdr;
  // SYMBOL
  const char *name;
} Obj;

typedef struct {
  Obj *root; // the indestructible object where gc marking starts
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

Obj *gc_get_obj(GC *gc, Type type) {
  Obj *o = malloc(sizeof(Obj));
  o->reachable = false;
  o->type = type;

  printf("Created obj %p of type %s.\n", o, type_to_str(o->type));
  
  // Put new object first:
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
  gc->root = NULL;
  gc->firstObj = NULL;
}

void gc_collect(GC *gc) {
  if(gc->root) {
    mark(gc->root);
  } else {
    printf("Warning, no root set.\n");
  }

  int freeCount = 0;
  
  // SWEEP
  Obj *o = gc->firstObj;
  Obj *prev = NULL;
  while(o) {
    printf("Sweep visiting %p, %s. ", o, obj_to_str(o));
    if(o->reachable) {
      printf("\n");
      o->reachable = false; // unmark it so that it can be deleted in the future
      prev = o;
      o = o->next;
    }
    else {
      printf("Will free object.\n"); // %p, %s.\n", o, obj_to_str(o));
      if(o == gc->firstObj) {
	gc->firstObj = o->next;
      }
      if(prev != NULL) {
	prev->next = o->next;
      }
      o = o->next;
      free(o);
      freeCount++;
    }
  }

  printf("Sweep done, %d objects freed.\n", freeCount);
}

int main(int argc, char *argv[])
{
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

  cell3->car = cell4;
  cell4->cdr = cell3;
  
  cell1->car = cell3;
  cell4->car = sym1;
  cell4->cdr = sym2;
  cell3->cdr = sym3;
  cell2->cdr = sym2;

  gc.root = cell1;

  gc_collect(&gc);
  gc_collect(&gc);
  gc_collect(&gc);
  
  return 0;
}
