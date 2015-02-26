#include "Compiler.h"
#include "Parser.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

bool is_symbol(Obj *form, const char *name) {
  return form->car->type == SYMBOL && strcmp(form->car->name, name) == 0;
}

bool is_binary_call(Obj *form, const char *name) {
  return is_symbol(form, name) && count(form->cdr) == 2;
}

int find_arg_index_in_arglist(Obj *args, Obj *symbol) {
  assert(symbol->type == SYMBOL);
  int arg_index = -1;
  Obj *arg = args;
  int i = 0;
  while(arg && arg->car) {
    if(eq(arg->car, symbol)) {
      arg_index = i;
      /* printf("Arg '%s' found in position %d.\n", arg->car->name, arg_index); */
      break;
    }
    arg = arg->cdr;
    i++;
  }
  return arg_index;
}

void visit(CodeWriter *writer, Runtime *r, Obj *env, Obj *form, bool tail_position, Obj *args) {
  /* printf("Visiting %s ", tail_position ? "tail position" : ""); */
  /* print_obj(form); */
  /* printf(" with args "); */
  /* print_obj(args); */
  /* printf("\n"); */
  
  if(form->type == SYMBOL) {
    int arg_index = find_arg_index_in_arglist(args, form);
    if(arg_index > -1) {
      // Value is local to innermost function!
      code_write_lookup_arg(writer, arg_index);
    }
    else {
      // Search for an argument in the call stack (not in the global frame though!)
      for(int i = r->top_frame; i > 0; i--) {
      	Frame frame = r->frames[i];
      	int arg_index = find_arg_index_in_arglist(frame.arg_symbols, form);
      	if(arg_index > -1) {
      	  Obj *constant = frame.args[arg_index];
      	  /* printf("Found value for %s in frame %s: ", form->name, frame.name); */
      	  /* print_obj(constant); */
      	  /* printf("\n"); */
      	  code_write_push_constant(writer, constant);
      	  return;
      	}
      }
      
      Obj *binding_pair = runtime_env_find_pair(r->global_env, form, true, NULL);
    
      if(binding_pair) {
	//printf("Found binding called '%s' in env %p: ", form->name, env); print_obj(binding_pair); printf("\n");
	code_write_direct_lookup_var(writer, binding_pair); // Fast lookup of globals
      }
      else {
	printf("Warning: Can't find binding for '%s'.\n", form->name);
	code_write_lookup_and_push(writer, form); // Not found at all!	
      }
    }    
  }
  else if(form->type == NUMBER || form->type == STRING) {
    code_write_push_constant(writer, form);
  }
  else if(form->type == CONS) {
    if(form->car == NULL || form->cdr == NULL) {
      code_write_push_constant(writer, r->nil);
    }
    else if(is_symbol(form, "def")) {
      // Pre-define the binding so that it can be found by recursive function calls etc.
      Obj *symbol = form->cdr->car;
      Obj *value = form->cdr->cdr->car;
      runtime_env_assoc(r, r->global_env, symbol, r->nil);
      visit(writer, r, env, value, tail_position, args);
      code_write_define(writer, symbol);
    }
    else if(is_symbol(form, "set!")) {
      Obj *symbol = SECOND(form);
      Obj *value = THIRD(form);
      //runtime_env_assoc(r, env, symbol, r->nil);
      visit(writer, r, env, value, tail_position, args);
      code_write_set(writer, symbol);
    }
    else if(is_symbol(form, "quote")) {
      code_write_push_constant(writer, form->cdr->car);
    }
    else if(is_binary_call(form, "+")) {
      visit(writer, r, env, form->cdr->car, false, args);
      visit(writer, r, env, form->cdr->cdr->car, false, args);
      code_write_code(writer, ADD);
    }
    else if(is_binary_call(form, "-")) {
      visit(writer, r, env, form->cdr->car, false, args);
      visit(writer, r, env, form->cdr->cdr->car, false, args);
      code_write_code(writer, SUB);
    }
    else if(is_binary_call(form, "*")) {
      visit(writer, r, env, form->cdr->car, false, args);
      visit(writer, r, env, form->cdr->cdr->car, false, args);
      code_write_code(writer, MUL);
    }
    else if(is_binary_call(form, "/")) {
      visit(writer, r, env, form->cdr->car, false, args);
      visit(writer, r, env, form->cdr->cdr->car, false, args);
      code_write_code(writer, DIV);
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "do") == 0) {
      Obj *subform = form->cdr;
      while(subform && subform->car) {
	bool last_form = subform->cdr == NULL || subform->cdr->car == NULL;
	visit(writer, r, env, subform->car, last_form, args);
	if(!last_form) {
	  code_write_pop(writer); // pop value if form is not the last one
	}
	subform = subform->cdr;
      }
    }
    else if(is_symbol(form, "if")) {
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
      
      visit(writer, r, env, expression, false, args); // the result from this will be the branching value

      int true_code_length;
      Code *true_bytecode = compile(r, env, tail_position, true_branch, &true_code_length, args);

      int false_code_length;
      Code *false_bytecode = compile(r, env, tail_position, false_branch, &false_code_length, args);

      code_write_if(writer); // this code will jump forward one step if value on the stack is true, otherwise it will jump two steps
      
      // Subtract one from the length for the END_OF_CODE statement that shouldn't go into the code.
      // Add two for the following jump instruction that leads to the merging point.
      false_code_length--;
      code_write_jump(writer, false_code_length + 2);
      memcpy(&writer->codes[writer->pos], false_bytecode, sizeof(Code*) * false_code_length);
      writer->pos += false_code_length;

      // Only subtract one for the END_OF_CODE statement of the branch, no extra statement to count for.
      true_code_length--;
      code_write_jump(writer, true_code_length);
      memcpy(&writer->codes[writer->pos], true_bytecode, sizeof(Code*) * true_code_length);
      writer->pos += true_code_length;

      // Free the memory from the temporary blocks made with compile
      free(true_bytecode);
      free(false_bytecode);
    }
    else if(form->car->type == SYMBOL && (strcmp(form->car->name, "fn") == 0 || strcmp(form->car->name, "λ") == 0)) {
      Obj *arg_symbols = form->cdr->car;
      Obj *body = form->cdr->cdr->car;
      int arg_count = count(arg_symbols);

      // Create a bunch of fake bindings with the right name but their value set to nil
      /*
      Obj *arg_values = r->nil;
      for(int i = 0; i < arg_count; i++) {
	arg_values = gc_make_cons(r->gc, r->nil, arg_values);
      }
      Obj *compile_time_local_env = bind_args_in_new_env(r, env, arg_symbols, arg_values, arg_count);
      
      int code_length = 0;
      Code *bytecode = compile(r, compile_time_local_env, true, body, &code_length);
      */
      
      code_write_push_lambda(writer, arg_symbols, body, NULL);
    }
    else {
      Obj *f = form->car;
      Obj *arg = form->cdr;
      int caller_arg_count = 0;
      while(arg && arg->car) {
	visit(writer, r, env, arg->car, false, args);
	caller_arg_count++;
	arg = arg->cdr;
      }
      visit(writer, r, env, f, false, args);

      if(tail_position) {
	code_write_tail_call(writer, caller_arg_count);
      } else {
	code_write_call(writer, caller_arg_count);
      }
    }
  }
  else {
    printf("Can't visit form:\n");
    print_obj(form);
    printf("\n");
    exit(1);
  }
}

Code *compile(Runtime *r, Obj *env, bool tail_position, Obj *form, int *OUT_code_length, Obj *args) {
  CodeWriter writer;
  code_writer_init(&writer, 1024);
  visit(&writer, r, env, form, tail_position, args);
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
    Code *code = compile(r, r->global_env, false, form, &code_length, NULL);
    printf("Generating code for ");
    print_obj(form);
    printf("\n");
    code_print(code);
    printf("Length: %d\n", code_length);
    form_cons = form_cons->cdr;
  }
  runtime_delete(r);
}

