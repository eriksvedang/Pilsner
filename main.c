#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "GC.h"
#include "Obj.h"
#include "Tests.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* bool is_alpha(char c) { */
/*   return 'a' < c && c < 'Z'; */
/* } */

Obj *parse(GC *gc, int pos, const char *source) {
  int len = strlen(source);
  Obj *root = NULL;
  
  if(source[pos] == '(') {
    Obj *cons = gc_make_cons(gc, NULL, NULL);
    root = cons;
    while(source[pos] != ')' && pos < len) {
      pos++;
      Obj *child = parse(gc, pos, source);
      if(child) {
	cons->car = child;
	Obj *newCons = gc_make_cons(gc, NULL, NULL);
	cons->cdr = newCons;
	cons = newCons;
      }
    }
  }
  else if(isalpha(source[pos])) {
    const char *name = "sym";
    Obj *symbol = gc_make_symbol(gc, name);
    while(isalpha(source[pos]) && pos < len) {
      pos++;
    }
    root = symbol;
  }

  return root;
}

void print_obj(Obj *o) {
  if(o->type == CONS) {
    printf("(");
    Obj *curr = o;
    while(curr) {
      print_obj(o->car);
      if(o->cdr) {
	printf(", ");
	curr = o->cdr;
      } else {
	printf(")\n");
	break;
      }
    }         
  }
  else if(o->type == SYMBOL) {
    printf("%s", o->name);
  }
}


int main(int argc, char *argv[])
{
  test1();

  GC gc;
  gc_init(&gc);

  printf("---\n");
  print_obj(parse(&gc, 0, "foo"));
  printf("---\n");
  print_obj(parse(&gc, 0, "(foo)"));
  printf("---\n");
  
  //printf("Obj: %s\n", );
  //printf("Obj: %s\n", );
  
  return 0;
}


