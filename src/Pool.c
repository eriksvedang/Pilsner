#include "Pool.h"
#include <stdlib.h>
#include <stdio.h>

Pool *pool_new(int starting_size) {
  Pool *p = malloc(sizeof(Pool));
  p->objs = malloc(sizeof(Obj*) * starting_size);
  p->total_count = starting_size;
  p->top_obj = -1;
  for(int i = 0; i < starting_size; i++) {
    p->objs[i] = malloc(sizeof(Obj));
  }
  return p;
}

Obj *pool_obj_get(Pool *p) {
  if(p->top_obj >= p->total_count - 1) {
    printf("Out of objects in Pool.\n");
    exit(1);
  }
  return p->objs[++p->top_obj];
}

void pool_obj_return(Pool *p, Obj *o) {
  if(p->top_obj <= 0) {
    printf("Stack underflow in Pool.\n");
    exit(1);
  }
  p->objs[p->top_obj--] = o;
}

