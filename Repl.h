#ifndef REPL_H
#define REPL_H

#include "GC.h"
#include "Obj.h"
#include "Parser.h"
#include "Runtime.h"

void repl() {
  const int BUFFER_SIZE = 256;
  char str[BUFFER_SIZE];
  
  Runtime *r = runtime_new();

  runtime_eval(r, "(def id (fn (x) x))");
  runtime_eval(r, "(def a (fn (x) (+ x 3)))");
  runtime_eval(r, "(def fact (fn (x) (if (< x 2) 1 (+ x (fact (- x 1))))))");

  // 1 + 2 + 3 + 4 + 5
  // 1 * 2 * 3 * 4 * 5
  
  while(r->mode != RUNTIME_MODE_FINISHED) {
    printf("> ");
    fgets(str, BUFFER_SIZE, stdin);
    //if(strcmp(str, "quit\n") == 0) break;
    runtime_eval(r, str);
  }
  
  //runtime_inspect_env(r);
  gc_collect(r->gc);
  runtime_delete(r);
}

#endif

