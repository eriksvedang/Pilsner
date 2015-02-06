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
  Obj *first_form;
  Obj *last_form;
} Parser;

Parser *parser_make() {
  Parser *p = malloc(sizeof(Parser));
  p->pos = 0;
  return p;
}

Obj *parse_sexp(GC *gc, Parser *p, const char *source);

Obj *parse_form(GC *gc, Parser *p, const char *source) {
  if (source[p->pos] == '(') {
    p->pos++;
    return parse_sexp(gc, p, source);
  }
  else if(isalpha(source[p->pos])) {
    while(isalpha(source[p->pos])) {
      p->pos++;
    }
    return gc_make_symbol(gc, "?");
  }
  p->pos++;

  return NULL;
}

Obj *parse_sexp(GC *gc, Parser *p, const char *source) {
  Obj *sexp = gc_make_cons(gc, NULL, NULL);
  Obj *lastSexp = sexp;
 again:;
  Obj *inner = parse_form(gc, p, source); 
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
  return sexp;
}

Obj *parse(GC *gc, const char *source) {
  Parser* p = parser_make();
  int source_len = strlen(source);
  int parens = 0;

  p->first_form = gc_make_cons(gc, NULL, NULL);
  p->last_form = p->first_form;
  
  while(p->pos < source_len) {
    Obj *form = parse_form(gc, p, source);
    if(form) {
      printf("Got form: ");
      print_obj(form);
      printf("\n");
      p->last_form->car = form;
      p->last_form->cdr = gc_make_cons(gc, NULL, NULL);
      p->last_form = p->last_form->cdr;
    }
  }

  //printf("%d parens.\n", parens);
 
  return p->first_form;
}

  

int main()
{
  test_gc();
  //test_printing();
  
  GC gc;
  Obj *forms = parse(&gc, "a b c (d e) ((f g g) (h i) (j k))"); //(aha boo cic) (d e) (f) g h i");
  print_obj(forms);
}


