#ifndef OBJ_H
#define OBJ_H

#include "Error.h"
#include <stdbool.h>

enum eCode;

typedef enum {
  CONS,
  SYMBOL,
  FUNC,
  NUMBER,
  STRING,
  LAMBDA,
  BYTECODE,
} Type;

typedef struct sObj {
  struct sObj *next;
  
  union {
    // CONS & LAMBDA
    struct {
      struct sObj *car;
      struct sObj *cdr;
    };
    // FUNC
    struct {
      void *func;
      //char *func_name; // TODO: use this again and move *name into union?
    };
    // NUMBER
    double number;
    // BYTECODE
    enum eCode *code;
  };

  char *name; // used by symbols and strings for their content

  // Put smaller types last to decrease size of the struct
  Type type;
  bool reachable;
  
} Obj;

//typedef Obj (*Func)(Obj *args);


const char *type_to_str(Type type);
const char *obj_to_str(Obj *o);

void print_obj(Obj *o);
void obj_describe(const char *description, Obj *o);
  
bool eq(Obj *a, Obj *b);
int count(Obj *list);

// Cons cell helpers
#define FIRST(o)  ((o)->car)
#define SECOND(o) ((o)->cdr->car)
#define THIRD(o)  ((o)->cdr->cdr->car)
#define REST(o)   ((o)->cdr)

// Lambda helpers
#define GET_ENV(o)  ((o)->car->car)
#define GET_ARGS(o) ((o)->car->cdr)
#define GET_BODY(o) ((o)->cdr->car)
#define GET_CODE(o) ((o)->cdr->cdr->code)

#endif
