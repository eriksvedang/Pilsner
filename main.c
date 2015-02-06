#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "GC.h"
#include "Obj.h"
#include "Parser.h"
#include "Repl.h"
#include "Tests.h"

void tests() {
  test_gc();
  //test_printing();
  //test_parsing();
}

typedef struct {
  GC *gc;
  Obj *global_env;
} Runtime;

Runtime *runtime_new() {
  GC *gc = malloc(sizeof(GC));
  gc_init(gc);
  Runtime *r = malloc(sizeof(Runtime));
  r->gc = gc;
  return r;
}

void runtime_delete(Runtime *r) {
  free(r->gc);
  free(r);
}

Obj *eval(Runtime *r, Obj *form) {
  return form;
}

void runtime_eval(Runtime *r, const char *source) {
  Obj *forms = parse(r->gc, source);
  Obj *current_form = forms;
  while(current_form->car) {
    Obj *result = eval(r, current_form->car);
    print_obj(result);
    printf("\n");
    current_form = current_form->cdr;
  }
}

void test_runtime() {
  Runtime *r = runtime_new();
  runtime_eval(r, "aha jaja nej");
  runtime_delete(r);
}

int main()
{
  tests();
  //repl();
  test_runtime();
}

