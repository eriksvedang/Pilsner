#ifndef OBJ_H
#define OBJ_H

#include "Error.h"
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

const char *type_to_str(Type type);
const char *obj_to_str(Obj *o);
  
#endif
