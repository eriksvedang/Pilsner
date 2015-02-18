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
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "quote") == 0) {
      code_write_push_constant(writer, form->cdr->car);
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "if") == 0) {
      Obj *expression = form->cdr->car;
      if(!expression) {
	printf("No expression in if-statement.\n");
	return;
      }
      
      Obj *true_branch = form->cdr->cdr->car;
      if(!true_branch) {
	printf("No true-branch in if-statement.\n");
	return;
      }
      
      Obj *false_branch = form->cdr->cdr->cdr->car;
      if(!false_branch) {
	printf("No false-branch in if-statement.\n");
	return;
      }
      
      visit(writer, gc, expression); // the result from this will be the branching value

      int true_code_length;
      Code *true_bytecode = compile(gc, true_branch, &true_code_length);
      //printf("True branch length: %d\n", true_code_length);

      int false_code_length;
      Code *false_bytecode = compile(gc, false_branch, &false_code_length);
      //printf("False branch length: %d\n", false_code_length);

      code_write_if(writer); // this code will jump forward one step if value on the stack is true, otherwise it will jump two steps
      
      // Subtract one from the legnth for the end statement that shouldn't go into the code.
      // Add two for the following jump that leads to the merging point.
      false_code_length--;
      code_write_jump(writer, false_code_length + 2);
      //code_write_push_constant(writer, gc_make_string(gc, "FALSE CODE GOES HERE"));
      memcpy(&writer->codes[writer->pos], false_bytecode, sizeof(Code*) * false_code_length);
      writer->pos += false_code_length;

      // Only subtract one for the end statement of the branch, no extra statement to count for.
      true_code_length--;
      code_write_jump(writer, true_code_length);
      //code_write_push_constant(writer, gc_make_string(gc, "TRUE CODE GOES HERE"));
      memcpy(&writer->codes[writer->pos], true_bytecode, sizeof(Code*) * true_code_length);
      writer->pos += true_code_length;

      // TODO: free the memory from the temporary blocks made with compile!
    }
    else if(form->car->type == SYMBOL && (strcmp(form->car->name, "fn") == 0 || strcmp(form->car->name, "λ") == 0)) {
      Obj *args = form->cdr->car;
      assert(args);
      Obj *body = form->cdr->cdr->car;
      assert(body);
      int code_length = 0;
      Code *bytecode = compile(gc, body, &code_length);
      /* printf("Compiled code for lambda "); */
      /* print_obj(form); */
      /* printf("\n"); */
      /* code_print(bytecode); */
      code_write_push_lambda(writer, args, body, bytecode);
    }
    else {
      // TODO: look up function here already and check arg count etc (catches some errors much earlier)
      Obj *f = form->car;
      Obj *arg = form->cdr;
      int caller_arg_count = 0;
      while(arg && arg->car) {
	visit(writer, gc, arg->car);
	caller_arg_count++;
	arg = arg->cdr;
      }
      visit(writer, gc, f);
      code_write_call(writer, caller_arg_count);
    }
  }
  else {
    printf("Can't visit form:\n");
    print_obj(form);
    printf("\n");
    exit(1);
  }
}

Code *compile(GC *gc, Obj *form, int *OUT_code_length) {
  CodeWriter writer;
  code_writer_init(&writer, 1024);
  visit(&writer, gc, form);
  code_write_end(&writer);
  *OUT_code_length = writer.pos;
  return writer.codes;
}

void compile_and_print(const char *source) {
  GC gc;
  gc_init(&gc);
  Obj *forms = parse(&gc, source);
  Obj *form_cons = forms;
  while(form_cons && form_cons->car) {
    Obj *form = form_cons->car;
    int code_length = 0;
    Code *code = compile(&gc, form, &code_length);
    printf("Generating code for ");
    print_obj(form);
    printf("\n");
    code_print(code);
    printf("Length: %d\n", code_length);
    form_cons = form_cons->cdr;
  }
}

