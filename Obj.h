#ifndef OBJ_H
#define OBJ_H

#include "Error.h"
#include <stdbool.h>

typedef enum {
  CONS,
  SYMBOL,
  FUNC,
  NUMBER,
  STRING,
  LAMBDA,
} Type;

typedef struct sObj {
  Type type;
  struct sObj *next;
  bool reachable;
  char *name; // used by symbols and strings for their content
  
  union {
    // CONS & LAMBDA
    struct {
      struct sObj *car;
      struct sObj *cdr;
    };
    // FUNC
    void *func;
    // NUMBER
    double number;
  };
} Obj;

//typedef Obj (*Func)(Obj *args);


const char *type_to_str(Type type);
const char *obj_to_str(Obj *o);

void print_obj(Obj *o);

bool eq(Obj *a, Obj *b);
  
#endif
