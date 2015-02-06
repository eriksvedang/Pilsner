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
  r->global_env = gc_make_cons(gc, NULL, NULL);
  return r;
}

void runtime_delete(Runtime *r) {
  free(r->gc);
  free(r);
}

bool eq(Obj *a, Obj *b) {
  if(a->type != b->type) return false;
  if(a->type == CONS) {
    return eq(a->car, b->car) && eq(a->cdr, b->cdr);
  }
  else if(a->type == SYMBOL) {
    return a->name == b->name || (strcmp(a->name, b->name) == 0);
  }
  else {
    return false;
  }
}

Obj *runtime_env_find_pair(Obj *env, Obj *key) {
  Obj *current = env;
  while(current->car != NULL) {
    if(eq(current->car->car, key)) {
      return current->car;
    }
    current = current->cdr;
  }
  return NULL;
}

Obj *runtime_env_assoc(GC *gc, Obj *env, Obj *key, Obj *value) {
  Obj *pair = runtime_env_find_pair(env, key);
  if(pair) {
    pair->cdr = value;
    return env;
  }
  else {
    Obj *new_pair = gc_make_cons(gc, key, value);
    Obj *new_env = gc_make_cons(gc, new_pair, env);
    return new_env;
  }
}

Obj *runtime_env_lookup(Obj *env, Obj *key) {
  Obj *pair = runtime_env_find_pair(env, key);
  if(pair) {
    return pair->cdr;
  }
  else {
    return NULL;
  }
}

Obj *eval(Runtime *r, Obj *form) {
  if(form->type == CONS) {
    if(form->car == NULL) {
      return form; // nil
    }
    else if(form->car->type == SYMBOL) {
      if(strcmp(form->car->name, "def") == 0) {
	Obj *key = form->cdr->car;
	Obj *value = form->cdr->cdr->car;
	printf("Defining a var named '%s' with the value ", key->name);
	print_obj(value);
	printf("\n");
	r->global_env = runtime_env_assoc(r->gc, r->global_env, key, value);
	return NULL;
      }
      else {
	printf("Time to call function..?!\n");
	return form;
      }
    }
    else {
      printf("Malformed list:\n");
      print_obj(form);
      printf("\n");
      exit(1);
      return NULL;
    }
  }
  else if(form->type == SYMBOL) {
    //printf("Looking up symbol %s\n", obj_to_str(form));
    Obj *result = runtime_env_lookup(r->global_env, form);
    if(result) {
      return result;
    }
    else {
      printf("Can't find a symbol named '%s'.\n", form->name);
      return NULL;
    }
  }
  else {
    printf("Can't eval this form:\n");
    print_obj(form);
    printf("\n");
    exit(1);
    return NULL;
  }
}

void runtime_eval(Runtime *r, const char *source) {
  Obj *forms = parse(r->gc, source);
  Obj *current_form = forms;
  while(current_form->car) {
    Obj *result = eval(r, current_form->car);
    if(result) {
      print_obj(result);
      printf("\n");
    }
    current_form = current_form->cdr;
  }
}

void runtime_inspect_env(Runtime *r) {
  printf("Global env:\n");
  print_obj(r->global_env);
  printf("\n");
}

void test_runtime() {
  Runtime *r = runtime_new();
  runtime_eval(r, "(def a b) (def c d) a c x");
  runtime_inspect_env(r);
  runtime_delete(r);
}

int main()
{
  tests();
  //repl();
  test_runtime();
}

