#ifndef BUILTIN_FUNCS_H
#define BUILTIN_FUNCS_H

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>
#include "Obj.h"

Obj *plus(Runtime *r, Obj *args[], int arg_count) {
  double sum = 0.0;
  for(int i = 0; i < arg_count; i++) {
    if(args[i]->type != NUMBER) {
      printf("Can't call + on ");
      print_obj(args[i]);
      printf("\n");
    }
    sum += args[i]->number;
  }
  return gc_make_number(r->gc, sum);
}

Obj *minus(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count == 0) {
    return gc_make_number(r->gc, 0);
  }
  if(arg_count == 1) {
    return gc_make_number(r->gc, -args[0]->number);
  }
  double sum = args[0]->number;
  for(int i = 1; i < arg_count; i++) {
    if(args[i]->type != NUMBER) {
      printf("Can't call - on ");
      print_obj(args[i]);
      printf("\n");
    }
    sum -= args[i]->number;
  }
  return gc_make_number(r->gc, sum);
}

Obj *multiply(Runtime *r, Obj *args[], int arg_count) {
  double product = 1.0;
  for(int i = 0; i < arg_count; i++) {
    if(args[i]->type != NUMBER) {
      printf("Can't call * on ");
      print_obj(args[i]);
      printf("\n");
    }
    product *= args[i]->number;
  }
  return gc_make_number(r->gc, product);
}

Obj *divide(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count == 0) {
    return gc_make_number(r->gc, 1.0);
  }
  double fraction = 1.0;
  for(int i = 1; i < arg_count; i++) {
    if(args[i]->type != NUMBER) {
      printf("Can't call / on ");
      print_obj(args[i]);
      printf("\n");
    }
    fraction /= args[i]->number;
  }
  return gc_make_number(r->gc, fraction);
}

Obj *greater_than(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count == 0) {
    return r->nil;
  }
  double last_value = args[0]->number;
  for(int i = 0; i < arg_count; i++) {
    Obj *o = args[i];
    if(o->type != NUMBER) {
      printf("Can't call < on ");
      print_obj(o);
      printf("\n");
    }
    double current_value = o->number;
    if(last_value > current_value) {
      return r->nil;
    }
    last_value = current_value;
  }
  return r->true_val;
}

Obj *internal_cos(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count != 1) {
    printf("Must call 'cos' with 1 argument.\n");
  }
  return gc_make_number(r->gc, cos(args[0]->number));
}

Obj *internal_sin(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count != 1) {
    printf("Must call 'cos' with 1 argument.\n");
  }
  return gc_make_number(r->gc, cos(args[0]->number));
}


Obj *and(Runtime *r, Obj *args[], int arg_count) {
  for(int i = 0; i < arg_count; i++) {
    if(args[i] == r->nil) return r->nil;
  }
  return r->true_val;
}

Obj *or(Runtime *r, Obj *args[], int arg_count) {
  for(int i = 0; i < arg_count; i++) {
    if(args[i] != r->nil) return r->true_val;
  }
  return r->nil;
}

Obj *print(Runtime *r, Obj *args[], int arg_count) {
  for(int i = 0; i < arg_count; i++) {
    printf("%s", obj_to_str(args[i]));
    printf("\n");
  }
  return r->nil;
}

Obj *println(Runtime *r, Obj *args[], int arg_count) {
  print(r, args, arg_count);
  printf("\n");
  return r->nil;
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

Obj *less_than(Runtime *r, Obj *args[], int arg_count) {
  return not_internal(r, greater_than(r, args, arg_count));
}

Obj *cons(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count != 2) {
    printf("Must call 'cons' with 2 args.\n");
    return r->nil;
  }
  Obj *o = args[0];
  Obj *rest = args[1];
  if(rest->type != CONS) {
    printf("Can't cons %s onto object %s.\n", obj_to_str(o), obj_to_str(rest));
    return r->nil;
  }
  /* printf("Consing "); */
  /* print_obj(o); */
  /* printf(" onto "); */
  /* print_obj(rest); */
  /* printf("\n"); */
  Obj *cons = gc_make_cons(r->gc, o, rest);
  return cons;
}

Obj *first(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count != 1) {
    printf("Must call 'first' with 1 arg.\n");
    return r->nil;
  }
  else if(args[0]->type != CONS) {
    printf("Can't call 'first' on non-list: ");
    print_obj(args[0]);
    printf("\n");
  }
  return args[0]->car;
}

Obj *rest(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count != 1) {
    printf("Must call 'rest' with 1 arg.\n");
    return r->nil;
  }
  else if(args[0]->type != CONS) {
    printf("Can't call 'rest' on non-list: ");
    print_obj(args[0]);
    printf("\n");
  }
  return args[0]->cdr;
}

Obj *list(Runtime *r, Obj *args[], int arg_count) {
  // todo: make this a special form instead, to avoid making new conses?
  Obj *l = r->nil;
  for(int i = arg_count - 1; i >= 0; i--) {
    Obj *cons = gc_make_cons(r->gc, args[i], l);
    l = cons;
  }
  return l;
}

Obj *nil_p(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count != 1) {
    printf("Must call 'nil?' with 1 arg.\n");
    return r->nil;
  }
  Obj *o = args[0];
  if(o->car == NULL && o->cdr == NULL) {
    return r->true_val;
  } else {
    return r->nil;
  }
}

Obj *print_code(Runtime *r, Obj *args[], int arg_count) {
  if(arg_count != 1) {
    printf("Must call 'print-code' with 1 arg.\n");
    return r->nil;
  }
  if(args[0]->type != LAMBDA) {
    printf("Can't call 'print-code' on non-lambda.\n");
  }
  else {
    code_print((Code*)GET_CODE(args[0]));
  }
  return r->nil;
}

Obj *help(Runtime *r, Obj *args[], int arg_count) {
  printf("Useful functions\n");
  printf("----------------\n");
  printf("(quit)\n");
  printf("(env)\n");
  printf("(stack)\n");
  printf("(first xs) (rest xs)\n");
  printf("(def var value)\n");
  printf("(fn (x) (* x x)\n");
  printf("(if expr then else)\n");
  return r->nil;
}

double current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    //printf("milliseconds: %lld\n", milliseconds);
    return ((double)milliseconds) / 1000.0;
}

Obj *get_time(Runtime *r, Obj *args[], int arg_count) {
  return gc_make_number(r->gc, (double)current_timestamp()); //  / CLOCKS_PER_SEC
}

#endif
