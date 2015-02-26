#include "GC.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define LOG 0
#define LOG_GC_COLLECT_RESULT 1
#define LOG_PUSH_AND_POP 0

#if GLOBAL_OBJ_COUNT
int g_obj_count = 0;
#endif

void gc_stack_push(GC *gc, Obj *o) {
  if(gc->stackSize >= STACK_MAX) error("Stack overflow.");
  gc->stack[gc->stackSize++] = o;
  #if LOG_PUSH_AND_POP
  printf("Pushed: ");
  print_obj(o);
  printf("\n");
  #endif
}

Obj *gc_stack_pop(GC *gc) {
  if(gc->stackSize < 0) error("Stack underflow.");
  Obj *o = gc->stack[--gc->stackSize];
  #if LOG_PUSH_AND_POP
  printf("Popped: ");
  print_obj(o);
  printf("\n");
  #endif
  return o;
}

void gc_stack_print(GC *gc, bool show_bottom_frame) {
  printf("--- VALUE STACK ---\n");
  for(int i = gc->stackSize - 1; i >= 0; i--) {
    printf("%d: \t", i);
    if(!show_bottom_frame && i == 0) {
      printf("...");
    } else {
      print_obj(gc->stack[i]);
    }
    printf("\n");
  }
  printf("-------------------\n");
}

Obj *gc_make_obj(GC *gc, Type type) {
  #if USE_MEMORY_POOL
  Obj *o = pool_obj_get(gc->pool);
  #else
  Obj *o = malloc(sizeof(Obj));
  #endif
  o->reachable = false;
  o->type = type;
  o->name = NULL;

  #if LOG
  printf("Created obj %p of type %s.\n", o, type_to_str(o->type));
  #endif
  
  // Put new object first in the linked list containing all objects:
  o->next = gc->firstObj; 
  gc->firstObj = o;

  #if GLOBAL_OBJ_COUNT
  g_obj_count++;
  #endif
  
  return o;
}

Obj *gc_make_cons(GC *gc, Obj *car, Obj *cdr) {
  Obj *o = gc_make_obj(gc, CONS);
  o->car = car;
  o->cdr = cdr;
  return o;
}

void set_name(Obj *o, const char *name) {
  char *name_copy = calloc(strlen(name) + 1, sizeof(char));
  strcpy(name_copy, name);
  o->name = name_copy;
}

Obj *gc_make_symbol_from_malloced_string(GC *gc, char *name) {
  Obj *o = gc_make_obj(gc, SYMBOL);
  o->name = name;
  return o;
}

Obj *gc_make_symbol(GC *gc, const char *name) {
  Obj *o = gc_make_obj(gc, SYMBOL);
  set_name(o, name);
  return o;
}

Obj *gc_make_func(GC *gc, const char *name, void *f) {
  Obj *o = gc_make_obj(gc, FUNC);
  o->name = (char*)name; // names of funcs are static strings and will not need to be freed when Obj is GC:d
  o->func = f;
  return o;
}

Obj *gc_make_number(GC *gc, double x) {
  Obj *o = gc_make_obj(gc, NUMBER);
  o->number = x;
  return o;
}

Obj *gc_make_string(GC *gc, char *text) {
  Obj *o = gc_make_obj(gc, STRING);
  o->name = text;
  return o;
}

Obj *gc_make_bytecode(GC *gc, Code *code) {
  Obj *o = gc_make_obj(gc, BYTECODE);
  o->code = (enum eCode*)code;
  return o;
}

Obj *gc_make_lambda(GC *gc, Obj *args, Obj *body, Code *code) {
  Obj *o = gc_make_obj(gc, LAMBDA);
  Obj *envAndArgs = gc_make_cons(gc, NULL, args);
  Obj *bodyAndCode = gc_make_cons(gc, body, gc_make_bytecode(gc, code));
  o->car = envAndArgs;
  o->cdr = bodyAndCode;
  return o;
}

void gc_obj_free(GC *gc, Obj *o) {
  if(o->type == SYMBOL || o->type == STRING) {
    o->car = NULL;
    o->cdr = NULL;
    free(o->name);
  }
  
  #if USE_MEMORY_POOL
  pool_obj_return(gc->pool, o);
  #else
  free(o);
  #endif
  
  #if GLOBAL_OBJ_COUNT
  g_obj_count--;
  #endif
}

void mark(Obj *o) {
  #if LOG
  printf("Marking %p, %s as reachable.\n", o, obj_to_str(o));
  #endif
  
  if(o->reachable) {
    // This one has already been visited by mark(), return to avoid infinite loops
    return;
  }
  
  o->reachable = true;
  
  if (o->type == CONS || o->type == LAMBDA) {
    if(o->car) {
      mark(o->car);
    }
    if(o->cdr) {
      mark(o->cdr);
    }
  }
  else if(o->type == BYTECODE) {
    Code *code = (Code*)o->code;
    while(*code != END_OF_CODES) {
      if(pushes_obj(*code)) {
	code += 1;
	Obj **oo = (Obj**)code;
	Obj *inner_o = *oo;
	mark(inner_o);
	code += 2;
      }
      else if(pushes_int(*code)) {
	code += 2;
      }
      else if(*code == PUSH_LAMBDA) {
	// TODO: follow args and body pointers
	code += 7;
      }
      else {
	code++;
      }
    }
  }
}

GC *gc_new() {
  GC *gc = malloc(sizeof(GC));
  gc->stackSize = 0;
  gc->firstObj = NULL;

#if USE_MEMORY_POOL
  gc->pool = pool_new(0);
#endif

  return gc;
}

GCResult gc_collect(GC *gc) {
  // Objects on the stack are all 'roots', i.e. they get automatically marked
  for (int i = 0; i < gc->stackSize; i++) {
    mark(gc->stack[i]);
  }

  GCResult result = {
    .alive = 0,
    .freed = 0,
  };
  
  // Sweep!
  Obj** obj = &gc->firstObj;
  while (*obj) {
    #if LOG
    printf("Sweep visiting %p, %s. ", *obj, obj_to_str(*obj));
    #endif
    
    if ((*obj)->reachable) {
      #if LOG
      printf("\n");
      #endif
      Obj* reached = *obj;
      reached->reachable = false; // reached this time, unmark it for future sweeps
      obj = &(reached->next); // pointer to the next object
      result.alive++;
    } else {
      #if LOG
      printf("Will free object.\n");
      #endif      
      Obj* unreached = *obj;
      *obj = unreached->next; // change the pointer in place, *THE MAGIC*
      gc_obj_free(gc, unreached);
      result.freed++;
    }
  }

  #if LOG_GC_COLLECT_RESULT
  printf("Sweep done, %d objects freed and %d object still alive.\n", result.freed, result.alive);
  /* Obj *first = gc->firstObj; */
  /* while(first) { */
  /*   print_obj(first); printf("\n"); */
  /*   first = first->next; */
  /* } */
  #endif

  return result;
}

void gc_delete(GC *gc) {
  while(gc->stackSize >= 1) {
    gc_stack_pop(gc);
  }
  gc_collect(gc);

  #if GLOBAL_OBJ_COUNT
  assert(g_obj_count == 0);
  #endif
  
  free(gc);
}

