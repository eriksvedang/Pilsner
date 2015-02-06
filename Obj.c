#include "Obj.h"
#include <stdlib.h>
#include <stdio.h>

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

void print_obj(Obj *o) {
  if(o == NULL) {
    printf("NULL");
  }
  else if(o->type == CONS) {
    printf("(");
    Obj *curr = o;
    while(curr) {
      if(curr->cdr) {
	print_obj(curr->car);
	if(curr->cdr->cdr) {
	  // the next cell is not nil
	  printf(" ");
	}
      }
      curr = curr->cdr;
      //printf("Curr set to %p", curr);
    }
    printf(")");
  }
  else if(o->type == SYMBOL) {
    printf("%s", o->name);
  }
}

