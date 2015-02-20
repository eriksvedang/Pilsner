#include "Compiler.h"
#include "Parser.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void visit(CodeWriter *writer, Runtime *r, Obj *form) {
  if(form->type == SYMBOL) {
    code_write_lookup_and_push(writer, form);
  }
  else if(form->type == NUMBER || form->type == STRING) {
    code_write_push_constant(writer, form);
  }
  else if(form->type == CONS) {
    if(form->car == NULL || form->cdr == NULL) {
      // TODO: push special nil value instead or use a specific op for this
      code_write_push_constant(writer, form);
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "def") == 0) {
      visit(writer, r, form->cdr->cdr->car);
      code_write_define(writer, form->cdr->car);
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "quote") == 0) {
      code_write_push_constant(writer, form->cdr->car);
    }
    else if(form->car->type == SYMBOL &&
	    strcmp(form->car->name, "+") == 0 &&
	    count(form->cdr) == 2) {
      visit(writer, r, form->cdr->car);
      visit(writer, r, form->cdr->cdr->car);
      code_write_code(writer, ADD);
    }
    else if(form->car->type == SYMBOL &&
	    strcmp(form->car->name, "-") == 0 &&
	    count(form->cdr) == 2) {
      visit(writer, r, form->cdr->car);
      visit(writer, r, form->cdr->cdr->car);
      code_write_code(writer, SUB);
    }
    else if(form->car->type == SYMBOL &&
	    strcmp(form->car->name, "*") == 0 &&
	    count(form->cdr) == 2) {
      visit(writer, r, form->cdr->car);
      visit(writer, r, form->cdr->cdr->car);
      code_write_code(writer, MUL);
    }
    else if(form->car->type == SYMBOL &&
	    strcmp(form->car->name, "/") == 0 &&
	    count(form->cdr) == 2) {
      visit(writer, r, form->cdr->car);
      visit(writer, r, form->cdr->cdr->car);
      code_write_code(writer, DIV);
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "do") == 0) {
      Obj *subform = form->cdr;
      while(subform && subform->car) {
	visit(writer, r, subform->car);
	if(subform->cdr && subform->cdr->car != NULL) {
	  code_write_pop(writer); // pop value if form is not the last one
	}
	subform = subform->cdr;
      }
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
      
      visit(writer, r, expression); // the result from this will be the branching value

      int true_code_length;
      Code *true_bytecode = compile(r, true_branch, &true_code_length);
      //printf("True branch length: %d\n", true_code_length);

      int false_code_length;
      Code *false_bytecode = compile(r, false_branch, &false_code_length);
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
      Code *bytecode = compile(r, body, &code_length);
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
	visit(writer, r, arg->car);
	caller_arg_count++;
	arg = arg->cdr;
      }
      visit(writer, r, f);
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

Code *compile(Runtime *r, Obj *form, int *OUT_code_length) {
  CodeWriter writer;
  code_writer_init(&writer, 1024);
  visit(&writer, r, form);
  code_write_end(&writer);
  *OUT_code_length = writer.pos;
  return writer.codes;
}

void compile_and_print(const char *source) {
  Runtime *r = runtime_new(true);
  Obj *forms = parse(r->gc, source);
  Obj *form_cons = forms;
  while(form_cons && form_cons->car) {
    Obj *form = form_cons->car;
    int code_length = 0;
    Code *code = compile(r, form, &code_length);
    printf("Generating code for ");
    print_obj(form);
    printf("\n");
    code_print(code);
    printf("Length: %d\n", code_length);
    form_cons = form_cons->cdr;
  }
  runtime_delete(r);
}

