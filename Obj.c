#include "Obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char *type_to_str(Type type) {
  if(type == CONS) return "CONS";
  else if(type == FUNC) return "FUNC";
  else if(type == SYMBOL) return "SYMBOL";
  else if(type == STRING) return "STRING";
  else if(type == NUMBER) return "NUMBER";
  else if(type == LAMBDA) return "LAMBDA";
  else if(type == BYTECODE) return "BYTECODE";
  else return "UNKNOWN";
}

const char *obj_to_str(Obj *o) {
  if(o == NULL) {
    return "NULL";
  }
  else if(o->type == CONS) {
    return "CONS";
  }
  else if(o->type == SYMBOL) {
    return o->name;
  }
  else if(o->type == FUNC) {
    char *s = malloc(sizeof(char) * strlen(o->name) + sizeof(char) * 3); // LEAK! SHOULD BE A PROPER OBJ STRING
    s[0] = '<';
    s[strlen(o->func_name) - 2] = '>';
    s[strlen(o->func_name) - 1] = '\0';
    char *s1 = &s[1];
    strcpy(s1, o->func_name);
    return s;
  }
  else if(o->type == NUMBER) {
    const int MAX_STR_LEN = 50;
    char *output = malloc(sizeof(char) * MAX_STR_LEN); // MEMORY LEAK!!!!!
    snprintf(output, MAX_STR_LEN, "%f", o->number);
    return output;
  }
  else if(o->type == STRING) {
    return o->name;
  }
  else if(o->type == LAMBDA) {
    return "λ";
  }
  else if(o->type == BYTECODE) {
    return "BYTECODE";
  }
  else {
    error("Can't print unknown type.");
    return NULL;
  }
}

void print_obj(Obj *o) {
  if(o == NULL) {
    printf("NULL");
  }
  else if(o->type == CONS && o->car == NULL && o->cdr == NULL) {
    printf("nil");
  }
  else if(o->type == CONS && o->cdr != NULL && o->cdr->type == CONS) {
    printf("(");
    Obj *curr = o;
    while(curr) {
      if(curr->cdr) {
	print_obj(curr->car);
	if(curr->cdr->cdr) {
	  // the next cell is not nil
	  printf(" ");
	}
      }
      curr = curr->cdr;
      //printf("Curr set to %p", curr);
    }
    printf(")");
  }
  else if(o->type == CONS) {
    printf("(");
    print_obj(o->car);
    printf(" . ");
    print_obj(o->cdr);
    printf(")");
  }
  else if(o->type == SYMBOL) {
    printf("%s", o->name);
  }
  else if(o->type == FUNC) {
    printf("<%s>", o->func_name);
  }
  else if(o->type == NUMBER) {
    printf("%f", o->number);
  }
  else if(o->type == STRING) {
    printf("\"%s\"", o->name);
  }
  else if(o->type == LAMBDA) {
    printf("λ");
  }
  else if(o->type == BYTECODE) {
    printf("BYTECODE");
  }
}

bool eq(Obj *a, Obj *b) {
  if(a == b) {
    return true;
  }
  else if(a->type != b->type) {
    return false;
  }
  else if(a->type == CONS) {
    return eq(a->car, b->car) && eq(a->cdr, b->cdr);
  }
  else if(a->type == SYMBOL || a->type == STRING) {
    return a->name == b->name || (strcmp(a->name, b->name) == 0);
  }
  else if(a->type == NUMBER) {
    return a->number == b->number; // TODO: this is not a good way to compare doubles, I guess?
  }
  else {
    return false;
  }
}

int count(Obj *list) {
  int i = 0;
  while(list->cdr != NULL) {
    i++;
    list = list->cdr;
  }
  return i;
}
