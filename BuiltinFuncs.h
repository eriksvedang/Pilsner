#ifndef BUILTIN_FUNCS_H
#define BUILTIN_FUNCS_H

#include <stdio.h>
#include <assert.h>
#include "Obj.h"

Obj *bleh(Runtime *r, Obj *args) {
  printf("BLEH!\n");
  return r->nil;
}

Obj *print_sym(Runtime *r, Obj *args) {
  assert(args);
  assert(args->car);
  print_obj(args->car);
  printf("\n");
  return r->nil;
}

Obj *print_two_syms(Runtime *r, Obj *args) {
  print_obj(args->car);
  printf("\n");
  print_obj(args->cdr->car);
  printf("\n");
  return r->nil;
}

#endif
