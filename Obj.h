#ifndef OBJ_H
#define OBJ_H

#include "Error.h"
#include <stdbool.h>

typedef enum {
  CONS,
  SYMBOL,
} Type;

typedef struct sObj {
  Type type;
  struct sObj *next;
  bool reachable;

  union {
    // CONS
    struct {
      struct sObj *car;
      struct sObj *cdr;
    };
    // SYMBOL
    const char *name;
  };
} Obj;

const char *type_to_str(Type type);
const char *obj_to_str(Obj *o);

void print_obj(Obj *o);
  
#endif
