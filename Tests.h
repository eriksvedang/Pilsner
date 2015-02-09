#ifndef TESTS_H
#define TESTS_H

#include <assert.h>
#include "GC.h"
#include "Obj.h"
#include "Tests.h"
#include "Parser.h"
#include "Runtime.h"

void test_gc() {
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

void test_printing() {
  GC gc;
  gc_init(&gc);

  Obj *cell1 = gc_make_cons(&gc, NULL, NULL);
  Obj *sym1 = gc_make_symbol(&gc, "sym1");
  Obj *cell2 = gc_make_cons(&gc, sym1, cell1);
  Obj *sym2 = gc_make_symbol(&gc, "sym2");
  Obj *cell3 = gc_make_cons(&gc, sym2, cell2);
  Obj *sym3 = gc_make_symbol(&gc, "sym3");
  Obj *cell4 = gc_make_cons(&gc, sym3, cell3);
  
  print_obj(cell1); printf("\n");
  print_obj(cell2); printf("\n");
  print_obj(cell3); printf("\n");
  print_obj(cell4); printf("\n");

  // Add a list in the middle of the list
  Obj *cell5 = gc_make_cons(&gc, NULL, NULL);
  Obj *cell6 = gc_make_cons(&gc, gc_make_symbol(&gc, "sym10"), cell5);
  Obj *cell7 = gc_make_cons(&gc, gc_make_symbol(&gc, "sym20"), cell6);
  cell3->car = cell7;
  print_obj(cell4); printf("\n");

  // A weird cell with head but no tail
  cell1->car = sym1;
  cell1->cdr = NULL;
  print_obj(cell1); printf("\n");

  gc_collect(&gc);
}

void test_parsing() {
  GC gc;
  gc_init(&gc);
  
  Obj *forms = parse(&gc, "() a b c (d e) ((f g h () ()) (() i j) (k (() l ()) m))");
  print_obj(forms);
}

void test_runtime() {
  Runtime *r = runtime_new();
  //runtime_eval(r, "(def a (quote b)) a bleh (bleh)");
  //runtime_eval(r, "(bleh) (print-sym (quote apa)) (print-two-syms (quote erik) (quote svedang))");
  //runtime_eval(r, "(+ 2 3)");
  //runtime_eval(r, "\"erik\"");
  runtime_eval(r, "(break) 3 4 5 (+ 2 3) (break) 10 20");
  //runtime_inspect_env(r);
  runtime_delete(r);
}

#endif
