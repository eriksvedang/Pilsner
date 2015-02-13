#include "Parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

typedef struct {
  int pos;
} Parser;

Parser *parser_make() {
  Parser *p = malloc(sizeof(Parser));
  p->pos = 0;
  return p;
}

Obj *parse_list(GC *gc, Parser *p, const char *source);

bool iswhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}

static const char *specials = "+-*/=%&,_!@#?§<>";

bool isokinsymbol(char c) {
  for (int i = 0; i < strlen(specials); i++) {
    if(specials[i] == c) return true;
  }
  return false;
}

Obj *parse_form(GC *gc, Parser *p, const char *source) {
  //printf("Parsing form\n");
  if (source[p->pos] == '(') {
    //printf("Found (\n");
    return parse_list(gc, p, source);
  }
  else if(source[p->pos] == ';') {
    while(source[p->pos] != '\n' && source[p->pos] != '\0') {
      p->pos++;
    }
  }
  else if(source[p->pos] == '\'') {
    p->pos++;
    Obj *nil = gc_make_cons(gc, NULL, NULL);
    Obj *quote = gc_make_symbol(gc, "quote");
    Obj *quoted_form = parse_form(gc, p, source);
    Obj *rest = gc_make_cons(gc, quoted_form, nil);
    Obj *cons = gc_make_cons(gc, quote, rest);
    return cons;
  }
  else if(isalpha(source[p->pos]) || isokinsymbol(source[p->pos])) {
    //printf("Found symbol: ");
    char *name = malloc(sizeof(char) * 256); // TODO: free this when the Obj is freed
    int i = 0;
    while(!iswhitespace(source[p->pos]) &&
	  source[p->pos] != ')' &&
	  source[p->pos] != '(') {
      name[i++] = source[p->pos];
      p->pos++;
    }
    name[i] = '\0';
    //printf("%s\n", name);
    return gc_make_symbol(gc, name);
  }
  else if(source[p->pos] == '"') {
    char *text = malloc(sizeof(char) * 256); // TODO: free this when the Obj is freed
    int i = 0;
    p->pos++;
    while(source[p->pos] != '"') {
      text[i++] = source[p->pos];
      p->pos++;
    }
    text[i] = '\0';
    return gc_make_string(gc, text);
  }
  else if(isdigit(source[p->pos])) {
    char s[256];
    int i = 0;
    while(isdigit(source[p->pos])) {
      s[i++] = source[p->pos];
      p->pos++;
    }
    s[i] = '\0';
    double num = atof(s);
    return gc_make_number(gc, num);
  }

  //printf("No form found, will return NULL\n");
  return NULL;
}

Obj *parse_list(GC *gc, Parser *p, const char *source) {
  Obj *list = gc_make_cons(gc, NULL, NULL);
  Obj *lastCons = list;

  //printf("Starting work on list %p\n", list);

  p->pos++; // move beyond the first paren
  
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

  Obj *forms = gc_make_cons(gc, NULL, NULL);
  Obj *last_form = forms;
  
  while(p->pos < source_len) {
    //printf("Looking for new form at pos %d \n", p->pos);
    Obj *form = parse_form(gc, p, source);
    if(form) {
      //printf("Got top-level form: ");
      //print_obj(form);
      //printf("\n");
      last_form->car = form;
      last_form->cdr = gc_make_cons(gc, NULL, NULL);
      last_form = last_form->cdr;
    }

    p->pos++;
  }

  //printf("%d parens.\n", parens);
 
  return forms;
}
