#ifndef REPL_H
#define REPL_H

#include "GC.h"
#include "Obj.h"
#include "Parser.h"

void repl() {
  const int BUFFER_SIZE = 256;
  
  GC gc;
  gc_init(&gc);
  char str[BUFFER_SIZE];

  while(1) {
    printf("> ");
    fgets(str, BUFFER_SIZE, stdin);
    Obj *forms = parse(&gc, str);
    printf("\n");
    print_obj(forms);
    printf("\n");
    gc_collect(&gc);
  }
}

#endif

