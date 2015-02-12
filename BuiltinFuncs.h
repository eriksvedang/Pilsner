#ifndef BUILTIN_FUNCS_H
#define BUILTIN_FUNCS_H

#include <stdio.h>
#include <assert.h>
#include "Obj.h"

Obj *plus(Runtime *r, Obj *args) {
  double sum = 0.0;
  Obj *arg = args;
  while(arg && arg->car) {
    if(arg->car->type != NUMBER) {
      printf("Can't call + on ");
      print_obj(arg->car);
      printf("\n");
    }
    sum += arg->car->number;
    arg = arg->cdr;
  }
  return gc_make_number(r->gc, sum);
}

Obj *multiply(Runtime *r, Obj *args) {
  double sum = 1.0;
  Obj *arg = args;
  while(arg && arg->car) {
    if(arg->car->type != NUMBER) {
      printf("Can't call * on ");
      print_obj(arg->car);
      printf("\n");
    }
    sum *= arg->car->number;
    arg = arg->cdr;
  }
  return gc_make_number(r->gc, sum);
}

Obj *equal(Runtime *r, Obj *args) {
  if(eq(args->car, args->cdr->car)) {
    return gc_make_symbol(r->gc, "true");
  } else {
    return r->nil;
  }
}

#endif
