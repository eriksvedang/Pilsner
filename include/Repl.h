#ifndef REPL_H
#define REPL_H

#include <assert.h>
#include "GC.h"
#include "Obj.h"
#include "Parser.h"
#include "Runtime.h"

void load(Runtime *r, const char *lib_path, const char *filename) {
  char full_path[2048];
  sprintf(full_path, "%s%s", lib_path, filename);
  runtime_load_file(r, (const char *)full_path, true);
}

void repl() {
  char *lib_path = getenv("PILSNER_LIB");
  if(!lib_path) {
    error("The environment variable PILNSER_LIB is not set.");
  }
    
  Runtime *r = runtime_new(true);

  //load(r, lib_path, "minimal.lisp");
  load(r, lib_path, "core.lisp");
  load(r, lib_path, "misc.lisp");
  load(r, lib_path, "tests.lisp");
  
  printf("\e[33m~ Welcome to the Pilsner REPL ~\e[0m\n");

  const int MAX_INPUT_BUFFER_SIZE = 2048;
  char str[MAX_INPUT_BUFFER_SIZE];

  while(r->mode != RUNTIME_MODE_FINISHED) {
    printf("\e[32m➜\e[0m ");
    fgets(str, MAX_INPUT_BUFFER_SIZE, stdin);
    if(strcmp(str, "§\n") == 0) {
      gc_collect(r->gc);
    } else {
      runtime_eval(r, str);
    }
  }

  //printf("\e[32m\nTHE END\e[0m\n");

  // The global environment should be the only thing on the stack
  assert(r->gc->stackSize == 1);

  runtime_delete(r);

  //fgets(str, MAX_INPUT_BUFFER_SIZE, stdin);
}

#endif

