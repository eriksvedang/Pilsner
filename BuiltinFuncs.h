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

Obj *minus(Runtime *r, Obj *args) {
  if(args->car == NULL) {
    return gc_make_number(r->gc, 0.0);
  }
  double sum = args->car->number;
  Obj *arg = args->cdr;
  while(arg && arg->car) {
    if(arg->car->type != NUMBER) {
      printf("Can't call - on ");
      print_obj(arg->car);
      printf("\n");
    }
    sum -= arg->car->number;
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

Obj *greater_than(Runtime *r, Obj *args) {
  if(args->car == NULL) {
    return r->nil;
  }
  double last_value = args->car->number;
  Obj *arg = args->cdr;
  while(arg && arg->car) {
    if(arg->car->type != NUMBER) {
      printf("Can't call < on ");
      print_obj(arg->car);
      printf("\n");
    }
    double current_value = arg->car->number;
    if(last_value > current_value) {
      return r->nil;
    }
    last_value = current_value;
    arg = arg->cdr;
  }
  return r->true_val;
}

Obj *equal(Runtime *r, Obj *args) {
  if(eq(args->car, args->cdr->car)) {
    return gc_make_symbol(r->gc, "true");
  } else {
    return r->nil;
  }
}

#endif
