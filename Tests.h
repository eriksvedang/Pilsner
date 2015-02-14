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
  //runtime_eval(r, "(break) 3 4 5 (+ 2 3) (break) 10 20");
  //runtime_inspect_env(r);
  runtime_delete(r);
}

void test_local_environments() {
  Runtime *r = runtime_new();

  print_obj(runtime_env_lookup(r->global_env, gc_make_symbol(r->gc, "+"))); printf("\n");
  print_obj(runtime_env_lookup(r->global_env, gc_make_symbol(r->gc, "grgdgdger"))); printf("\n");

  runtime_env_assoc(r, r->global_env, gc_make_symbol(r->gc, "HAH"), gc_make_symbol(r->gc, "mhm"));
  print_obj(runtime_env_lookup(r->global_env, gc_make_symbol(r->gc, "HAH"))); printf("\n");

  printf("\nLOCALS\n");
  
  Obj *local_env_1 = runtime_env_make_local(r, r->global_env);
  runtime_env_assoc(r, local_env_1, gc_make_symbol(r->gc, "local!"), gc_make_symbol(r->gc, "123"));
  print_obj(runtime_env_lookup(local_env_1, gc_make_symbol(r->gc, "local!"))); printf("\n");
  print_obj(runtime_env_lookup(local_env_1, gc_make_symbol(r->gc, "+"))); printf("\n");

  printf("\nMORE LOCALS\n");
  Obj *local_env_2 = runtime_env_make_local(r, local_env_1);
  runtime_env_assoc(r, local_env_2, gc_make_symbol(r->gc, "deep"), gc_make_symbol(r->gc, "JOJO"));
  print_obj(runtime_env_lookup(local_env_1, gc_make_symbol(r->gc, "deep"))); printf("\n");
  print_obj(runtime_env_lookup(local_env_2, gc_make_symbol(r->gc, "deep"))); printf("\n");

  printf("\nOVERRIDE\n");
  runtime_env_assoc(r, local_env_2, gc_make_symbol(r->gc, "HAH"), gc_make_symbol(r->gc, "mhm!!!"));
  print_obj(runtime_env_lookup(r->global_env, gc_make_symbol(r->gc, "HAH"))); printf("\n");
  print_obj(runtime_env_lookup(local_env_2, gc_make_symbol(r->gc, "HAH"))); printf("\n");

  /* printf("GLOBAL ENV: \n"); */
  /* print_obj(r->global_env); */
  /* printf("\nLOCAL ENV: \n"); */
  /* print_obj(local_env_1); */
  /* printf("\n"); */
  
  runtime_delete(r);
}

void test_str_allocs() {

  printf("sizeof(char) = %ld\n", sizeof(char));
  
  const char *a = "aha";
  printf("sizeof(a) = %ld\n", sizeof(a));

  const char *b = "booo";
  printf("sizeof(b) = %ld\n", sizeof(b));

  const char *c = "jo men så attehh....";
  printf("sizeof(c) = %ld\n", sizeof(c));

  const char *d = malloc(256);
  printf("sizeof(d) = %ld\n", sizeof(d));
  
}

#endif
