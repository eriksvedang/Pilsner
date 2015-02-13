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

Obj *divide(Runtime *r, Obj *args) {
  if(args->car == NULL) {
    return gc_make_number(r->gc, 1.0);
  }
  double result = args->car->number;
  Obj *arg = args->cdr;
  while(arg && arg->car) {
    if(arg->car->type != NUMBER) {
      printf("Can't call / on ");
      print_obj(arg->car);
      printf("\n");
    }
    result /= arg->car->number;
    arg = arg->cdr;
  }
  return gc_make_number(r->gc, result);
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

Obj *not_internal(Runtime *r, Obj *o) {
  if(eq(o, r->nil)) {
    return r->true_val;
  } else {
    return r->nil;
  }
}

Obj *not(Runtime *r, Obj *args) {
  return not_internal(r, args->car);
}

Obj *less_than(Runtime *r, Obj *args) {
  return not_internal(r, greater_than(r, args));
}

Obj *equal(Runtime *r, Obj *args) {
  if(eq(args->car, args->cdr->car)) {
    return gc_make_symbol(r->gc, "true");
  } else {
    return r->nil;
  }
}

Obj *cons(Runtime *r, Obj *args) {
  //printf("Calling cons!\n");
  Obj *o = args->car;
  Obj *rest = args->cdr->car;
  /* printf("Consing "); */
  /* print_obj(o); */
  /* printf(" onto "); */
  /* print_obj(rest); */
  /* printf("\n"); */
  Obj *cons = gc_make_cons(r->gc, o, rest);
  return cons;
}

Obj *first(Runtime *r, Obj *args) {
  if(args->car->type != CONS) {
    printf("Can't call 'first' on non-list: ");
    print_obj(args->car);
    printf("\n");
  }
  return args->car->car;
}

Obj *rest(Runtime *r, Obj *args) {
  if(args->car->type != CONS) {
    printf("Can't call 'rest' on non-list: ");
    print_obj(args->car);
    printf("\n");
  }
  return args->car->cdr;
}

Obj *nil_p(Runtime *r, Obj *args) {
  Obj *o = args->car;
  if(o->car == NULL && o->cdr == NULL) {
    return r->true_val;
  } else {
    return r->nil;
  }
}

#endif
