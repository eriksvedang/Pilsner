#include "Obj.h"
#include <stdlib.h>

const char *type_to_str(Type type) {
  if(type == CONS) return "CONS";
  else if(type == SYMBOL) return "SYMBOL";
  else return "UNKNOWN";
}

const char *obj_to_str(Obj *o) {
  if(o == NULL) {
    return "NULL";
  }
  else if(o->type == CONS) {
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
