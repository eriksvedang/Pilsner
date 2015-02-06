#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "GC.h"
#include "Obj.h"
#include "Tests.h"
#include "Parser.h"

void tests() {
  test_gc();
  //test_printing();
  //test_parsing();
}

void repl() {
  GC gc;
  gc_init(&gc);
  char str[256];
  //char *str = "hej\n";

  while(1) {
    printf("> ");
    gets(str);
    Obj *forms = parse(&gc, str);
    printf("\n");
    print_obj(forms);
    printf("\n");
    gc_collect(&gc);
  }
}

int main()
{
  tests();
  repl();
}

