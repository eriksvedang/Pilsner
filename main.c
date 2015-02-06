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

Obj *parse(GC *gc, FILE *e) {
  
  
  
 
  Obj *root = NULL;
  return root;
}

void print_obj(Obj *o) {
  if(o == NULL) {
    printf("NULL");
  }
  else if(o->type == CONS) {
    printf("(");
    Obj *curr = o;
    while(curr) {
      if(curr->cdr) {
	print_obj(curr->car);
	if(curr->cdr->car) {
	  // the next cell is not nil
	  printf(", ");
	}
      }
      curr = curr->cdr;
      //printf("Curr set to %p", curr);
    }
    printf(")");
  }
  else if(o->type == SYMBOL) {
    printf("%s", o->name);
  }
}

void test_printing() {
  GC gc;
  gc_init(&gc);

  Obj *cell1 = gc_make_cons(&gc, NULL, NULL);
  Obj *sym1 = gc_make_symbol(&gc, "sym1");
  Obj *cell2 = gc_make_cons(&gc, sym1, cell1);
  Obj *sym2 = gc_make_symbol(&gc, "sym2");
  Obj *cell3 = gc_make_cons(&gc, sym2, cell2);
  Obj *sym3 = gc_make_symbol(&gc, "sym3");
  Obj *cell4 = gc_make_cons(&gc, sym3, cell3);
  
  print_obj(cell1); printf("\n");
  print_obj(cell2); printf("\n");
  print_obj(cell3); printf("\n");
  print_obj(cell4); printf("\n");

  Obj *cell5 = gc_make_cons(&gc, NULL, NULL);
  cell3->car = cell5;

  print_obj(cell4);
}


int main(int argc, char *argv[])
{
  test_gc();
  test_printing();

  
  
  return 0;
}


