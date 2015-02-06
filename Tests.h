#ifndef TESTS_H
#define TESTS_H

#include <assert.h>

void test1() {
  GC gc;
  gc_init(&gc);

  Obj *sym1 = gc_make_symbol(&gc, "sym1");
  Obj *sym2 = gc_make_symbol(&gc, "sym2");
  Obj *sym3 = gc_make_symbol(&gc, "sym3");
  Obj *sym4 = gc_make_symbol(&gc, "sym4");
  
  Obj *cell1 = gc_make_cons(&gc, NULL, NULL);
  Obj *cell2 = gc_make_cons(&gc, NULL, NULL);
  Obj *cell3 = gc_make_cons(&gc, NULL, NULL);
  Obj *cell4 = gc_make_cons(&gc, NULL, NULL);

  cell1->car = sym1;
  cell1->cdr = cell2;
  cell2->car = sym2;

  // lone loop
  cell3->car = cell4;
  cell4->cdr = cell3;

  gc_stack_push(&gc, cell1);

  GCResult r1 = gc_collect(&gc);
  assert(r1.alive == 4);
  assert(r1.freed == 4);
	 
  GCResult r2 = gc_collect(&gc);
  assert(r2.alive == 4);
  assert(r2.freed == 0);

  gc_stack_pop(&gc);
  
  GCResult r3 = gc_collect(&gc);
  assert(r3.alive == 0);
  assert(r3.freed == 4);
}

#endif
