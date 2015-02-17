#include "Compiler.h"
#include "Parser.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void visit(CodeWriter *writer, GC *gc, Obj *form) {
  if(form->type == SYMBOL) {
    code_write_lookup_and_push(writer, form);
  }
  else if(form->type == NUMBER || form->type == STRING) {
    code_write_push_constant(writer, form);
  }
  else if(form->type == CONS) {
    if(form->car == NULL || form->cdr == NULL) {
      code_write_push_constant(writer, form); // TODO: push special nil value instead or use a specific op for this
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "def") == 0) {
      visit(writer, gc, form->cdr->cdr->car);
      code_write_define(writer, form->cdr->car);
    }
    else if(form->car->type == SYMBOL && (strcmp(form->car->name, "fn") == 0 || strcmp(form->car->name, "λ") == 0)) {
      Obj *args = form->cdr->car;
      assert(args);
      Obj *body = form->cdr->cdr->car;
      assert(body);
      Code *bytecode = compile(gc, body);
      printf("Compiled code for lambda ");
      print_obj(form);
      printf("\n");
      code_print(bytecode);
      code_write_push_lambda(writer, args, body, bytecode);
    }
    else {
      Obj *f = form->car;
      Obj *arg = form->cdr;
      int arg_count = 0;
      while(arg && arg->car) {
	visit(writer, gc, arg->car);
	arg_count++;
	arg = arg->cdr;
      }
      visit(writer, gc, f);
      code_write_call(writer, arg_count);
    }
  }
  else {
    printf("Can't visit form:\n");
    print_obj(form);
    printf("\n");
    exit(1);
  }
}

Code *compile(GC *gc, Obj *form) {
  CodeWriter writer;
  code_writer_init(&writer, 1024);
  visit(&writer, gc, form);  
  code_write_end(&writer);
  return writer.codes;
}

void compile_and_print(const char *source) {
  GC gc;
  gc_init(&gc);
  Obj *forms = parse(&gc, source);
  Obj *form_cons = forms;
  while(form_cons && form_cons->car) {
    Obj *form = form_cons->car;
    Code *code = compile(&gc, form);
    printf("Generating code for ");
    print_obj(form);
    printf("\n");
    code_print(code);
    form_cons = form_cons->cdr;
  }
}

