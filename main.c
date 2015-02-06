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

Obj *parse_list(GC *gc, Parser *p, const char *source);

Obj *parse_form(GC *gc, Parser *p, const char *source) {
  //printf("Parsing form\n");
  if (source[p->pos] == '(') {
    //printf("Found (\n");
    p->pos++;
    return parse_list(gc, p, source);
  }
  else if(isalpha(source[p->pos])) {
    //printf("Found symbol: ");
    char *name = malloc(sizeof(char) * 256); // TODO: free this when the Obj is freed
    char i = 0;
    while(isalpha(source[p->pos])) {
      name[i++] = source[p->pos];
      p->pos++;
    }
    name[i] = '\0';
    //printf("%s\n", name);
    return gc_make_symbol(gc, name);
  }
  p->pos++;

  //printf("No form found, will return NULL\n");
  return NULL;
}

Obj *parse_list(GC *gc, Parser *p, const char *source) {
  Obj *list = gc_make_cons(gc, NULL, NULL);
  Obj *lastCons = list;

  //printf("Starting work on list %p\n", list);

  while(1) {
    Obj *item = parse_form(gc, p, source);
    if(item) {
      //printf("Adding item to list %p: %s\n", list, obj_to_str(item));
      lastCons->car = item;
      Obj *next = gc_make_cons(gc, NULL, NULL);
      lastCons->cdr = next;
      lastCons = next;
    }    

    if(source[p->pos] == ')') {
      //printf("Found )\n");
      p->pos++;
      break;
    }
    
    p->pos++;
  }

  //printf("Ending work on list %p\n", list);  
  return list;
}

Obj *parse(GC *gc, const char *source) {
  Parser* p = parser_make();
  int source_len = strlen(source);
  int parens = 0;

  p->first_form = gc_make_cons(gc, NULL, NULL);
  p->last_form = p->first_form;
  
  while(p->pos < source_len) {
    //printf("Looking for new form at pos %d \n", p->pos);
    Obj *form = parse_form(gc, p, source);
    if(form) {
      //printf("Got top-level form: ");
      //print_obj(form);
      //printf("\n");
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
  Obj *forms = parse(&gc, "a b c (d e) ((f g h) (i j) (k l m))");
  print_obj(forms);
}

