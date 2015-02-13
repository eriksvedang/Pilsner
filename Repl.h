#ifndef REPL_H
#define REPL_H

#include "GC.h"
#include "Obj.h"
#include "Parser.h"
#include "Runtime.h"

void repl() {
  const int BUFFER_SIZE = 2048;
  char str[BUFFER_SIZE];
  
  Runtime *r = runtime_new();

  runtime_eval(r, "(load \"core.lisp\")");
  runtime_eval(r, "(load \"misc.lisp\")");
  
  while(r->mode != RUNTIME_MODE_FINISHED) {
    printf("> ");
    fgets(str, BUFFER_SIZE, stdin);
    runtime_eval(r, str);
  }
  
  //runtime_inspect_env(r);
  gc_collect(r->gc);
  runtime_delete(r);
}

#endif

