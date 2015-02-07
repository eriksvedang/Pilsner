#include "Obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
  else if(o->type == FUNC) {
    return "FUNC";
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
  else if(o->type == CONS && o->car == NULL && o->cdr == NULL) {
    printf("nil");
  }
  else if(o->type == CONS && o->cdr != NULL && o->cdr->type == CONS) {
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
  else if(o->type == CONS) {
    printf("(");
    print_obj(o->car);
    printf(" . ");
    print_obj(o->cdr);
    printf(")");
  }
  else if(o->type == SYMBOL) {
    printf("%s", o->name);
  }
  else if(o->type == FUNC) {
    printf("FUNC");
  }
}

bool eq(Obj *a, Obj *b) {
  if(a->type != b->type) return false;
  if(a->type == CONS) {
    return eq(a->car, b->car) && eq(a->cdr, b->cdr);
  }
  else if(a->type == SYMBOL) {
    return a->name == b->name || (strcmp(a->name, b->name) == 0);
  }
  else {
    return false;
  }
}


