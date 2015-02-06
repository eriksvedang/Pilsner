#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "GC.h"
#include "Obj.h"
#include "Tests.h"

typedef struct {
  int pos;
} Parser;

Parser *parser_make() {
  Parser *p = malloc(sizeof(Parser));
  p->pos = 0;
  return p;
}

Obj *parse(GC *gc, const char *source) {
  Parser* p = parser_make();
  int source_len = strlen(source);
  int parens = 0;

  Obj *forms = gc_make_cons(gc, NULL, NULL);
  Obj *lastForm = forms;
  
  while(p->pos < source_len) {
    if (source[p->pos] == '(') {
      p->pos++;
      Obj *sexp = gc_make_cons(gc, NULL, NULL);
      Obj *lastSexp = sexp;
    again:
      while(isalpha(source[p->pos])) {
	p->pos++;
      }
      Obj *inner = gc_make_symbol(gc, "?");
      lastSexp->car = inner;
      Obj *next = gc_make_cons(gc, NULL, NULL);
      lastSexp->cdr = next;
      lastSexp = next;
      if(source[p->pos] == ' ') {
	p->pos++;
	goto again;
      }
      else if(source[p->pos] != ')') {
	printf("Failed to parse %c at %d\n", source[p->pos], p->pos);
	exit(1);
      }
      lastForm->car = sexp;
      Obj* newCell = gc_make_cons(gc, NULL, NULL);
      lastForm->cdr = newCell;
      lastForm = newCell;
    }
    p->pos++;
  }

  //printf("%d parens.\n", parens);
 
  return forms;
}

  

int main()
{
  test_gc();
  //test_printing();
  
  GC gc;
  Obj *forms = parse(&gc, "(aha boo cic) (d e) (f) g h i");
  print_obj(forms);
}


