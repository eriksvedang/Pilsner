#ifndef REPL_H
#define REPL_H

#include <assert.h>
#include "GC.h"
#include "Obj.h"
#include "Parser.h"
#include "Runtime.h"

void repl() {

  char *lib_path = getenv("PILSNER_LIB");
  if(lib_path) {
    //printf("PILSNER_LIB=%s\n", lib_path);
  } else {
    error("The environment variable PILNSER_LIB is not set.");
  } 
  
  Runtime *r = runtime_new();

  char full_path[1024];
  
  sprintf(full_path, "%s%s", lib_path, "core.lisp");
  runtime_load_file(r, (const char *)full_path, true);

  sprintf(full_path, "%s%s", lib_path, "misc.lisp");
  runtime_load_file(r, (const char *)full_path, true);

  const int BUFFER_SIZE = 2048;
  char str[BUFFER_SIZE];
  
  while(r->mode != RUNTIME_MODE_FINISHED) {
    //runtime_print_frames(r);
    //gc_stack_print(r->gc);
    printf("> ");
    fgets(str, BUFFER_SIZE, stdin);
    runtime_eval(r, str);
  }

  // Pop global environment
  assert(r->gc->stackSize == 1);
  gc_stack_pop(r->gc);
  
  gc_collect(r->gc);
  runtime_delete(r);

  fgets(str, BUFFER_SIZE, stdin);
}

#endif

