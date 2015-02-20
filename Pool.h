#ifndef POOL_H
#define POOL_H

#import "Obj.h"

typedef struct {
  Obj **objs;
  int top_obj;
  int total_count;
} Pool;

Pool *pool_new();

Obj *pool_obj_get(Pool *p);
void pool_obj_return(Pool *p, Obj *o);

#endif
