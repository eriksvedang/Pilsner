#include "Runtime.h"
#include "Parser.h"
#include "BuiltinFuncs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define LOG_EVAL 0
#define LOG_ENV 0
#define LOG_LAMBDA_EVAL 0
#define LOG_VALUE_STACK 0
#define LOG_FUNC_CALL 0

void runtime_eval_internal(Runtime *r, Obj *env, const char *source, int top_frame_index, int break_frame_index, bool print_result);

// The environments root is a cons cell where the car contains the a-list and the cdr contains the parent env.

Obj *runtime_env_find_pair(Obj *env, Obj *key, bool allow_parent_search) {
  Obj *current = env->car; // get the a-list for this env
  while(current->car) {
    if(eq(current->car->car, key)) {
      return current->car;
    }
    current = current->cdr;
  }
  if(allow_parent_search && env->cdr) {
    return runtime_env_find_pair(env->cdr, key, true);
  } else {
    return NULL;
  }
}

void runtime_env_assoc(Runtime *r, Obj *env, Obj *key, Obj *value) {
  /* printf("Will register %s in env %p with value\n", key->name, env); print_obj(value); printf("\n"); */
  Obj *pair = runtime_env_find_pair(env, key, false);
  if(pair) {
    pair->cdr = value;
  }
  else {
    Obj *new_pair = gc_make_cons(r->gc, key, value);
    Obj *new_cons = gc_make_cons(r->gc, new_pair, env->car);
    env->car = new_cons; // cons pair to the a-list
  }
}

Obj *runtime_env_lookup(Obj *env, Obj *key) {
  Obj *pair = runtime_env_find_pair(env, key, true);
  if(pair) {
    return pair->cdr;
  }
  else if(env->cdr) {
    return runtime_env_lookup(env->cdr, key);
  }
  else {
    return NULL;
  }
}

Obj *runtime_env_make_local(Runtime *r, Obj *parent_env) {
  Obj *empty_alist = gc_make_cons(r->gc, NULL, NULL);
  Obj *env = gc_make_cons(r->gc, empty_alist, parent_env);
  return env;
}

void register_var(Runtime *r, const char *name, Obj *value) {
  Obj *var_name = gc_make_symbol(r->gc, name);
  runtime_env_assoc(r, r->global_env, var_name, value);
}

void register_func(Runtime *r, const char *name, void *f) {
  Obj *function_name = gc_make_symbol(r->gc, name);
  Obj *function_ptr = gc_make_func(r->gc, name, f);
  runtime_env_assoc(r, r->global_env, function_name, function_ptr);
}

Obj *runtime_break(Runtime *r, Obj *args) {
  r->mode = RUNTIME_MODE_BREAK;
  return r->nil;
}

Obj *runtime_quit(Runtime *r, Obj *args) {
  r->mode = RUNTIME_MODE_FINISHED;
  return r->nil;
}

Obj *runtime_env(Runtime *r, Obj *args) {
  runtime_inspect_env(r);
  return r->nil;
}

Obj *runtime_print_stack(Runtime *r, Obj *args) {
  gc_stack_print(r->gc);
  return r->nil;
}

void runtime_print_frames(Runtime *r) {
  printf("\n\e[35m");
  printf("----------- FRAMES ----------- \n");
  for(int i = r->top_frame; i >= 0; i--) {
    printf("%d\t%s\n", i, r->frames[i].name);
  }
  printf("------------------------------ \n");
  printf("\e[0m\n");
}

Obj *runtime_gc_collect(Runtime *r, Obj *args) {
  gc_collect(r->gc);
  return r->nil;
}

bool runtime_load_file(Runtime *r, const char *filename, bool silent) {
  if(!silent) {
    printf("Loading '%s' - ", filename);
  }
  
  char * buffer = 0;
  long length;
  FILE * f = fopen (filename, "rb");

  if(f) {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = malloc (length);
    if (buffer)	{
      fread (buffer, 1, length, f);
    }
    fclose (f);
  } else {
    printf("Failed to open file: %s\n", filename);
    return false;
  }

  if (buffer) {
    runtime_eval_internal(r, r->global_env, buffer, r->top_frame + 1, r->top_frame + 1, false);
    return true;
  } else {
    printf("Failed to open buffer from file: %s\n", filename);
    return false;
  }
}

Obj *runtime_load(Runtime *r, Obj *args) {
  const char *filename = args->car->name;
  if(runtime_load_file(r, filename, false)) {
    Obj *done = gc_make_symbol(r->gc, "DONE");
    return done;
  } else {
    return r->nil;
  }
}

void register_builtin_funcs(Runtime *r) {
  register_func(r, "=", &equal);
  register_func(r, "+", &plus);
  register_func(r, "-", &minus);
  register_func(r, "*", &multiply);
  register_func(r, "/", &divide);
  register_func(r, "<", &greater_than);
  register_func(r, ">", &less_than);
  register_func(r, "cons", &cons);
  register_func(r, "first", &first);
  register_func(r, "rest", &rest);
  register_func(r, "list", &list);
  register_func(r, "nil?", &nil_p);
  register_func(r, "not", &not);
  register_func(r, "println", &println);
  register_func(r, "help", &help);
  
  register_func(r, "break", &runtime_break);
  register_func(r, "quit", &runtime_quit);
  register_func(r, "env", &runtime_env);
  register_func(r, "load", &runtime_load);
  register_func(r, "stack", &runtime_print_stack);
  register_func(r, "gc", &runtime_gc_collect);
}

void register_builtin_vars(Runtime *r) {
  register_var(r, "nil", r->nil);
  register_var(r, "false", r->nil);
  register_var(r, "true", r->true_val);
}

Runtime *runtime_new() {
  GC *gc = malloc(sizeof(GC)); // TODO: call gc_new() instead
  gc_init(gc);
  Runtime *r = malloc(sizeof(Runtime));
  r->gc = gc;
  r->global_env = runtime_env_make_local(r, NULL);
  r->nil = gc_make_cons(gc, NULL, NULL);
  r->true_val = gc_make_symbol(r->gc, "true");
  r->top_frame = -1;
  r->mode = RUNTIME_MODE_RUN;
  gc_stack_push(r->gc, r->global_env); // root the global env so it won't get GC:d
  register_builtin_funcs(r);
  register_builtin_vars(r);
  return r;
}

void runtime_delete(Runtime *r) {
  free(r->gc); // TODO: call gc_delete() instead
  free(r);
}

Frame *frame_push(Runtime *r, Obj *env, Obj *start_pos, const char *name) {
  r->top_frame++;
  if(r->top_frame >= MAX_ACTIVATION_FRAMES) {
    printf("Can't push more frames, reached max limit: %d.\n", MAX_ACTIVATION_FRAMES);
    exit(0);
  }
  Frame *frame = &r->frames[r->top_frame];
  frame->p = start_pos;
  frame->depth = r->top_frame;
  frame->mode = MODE_NORMAL;
  frame->arg_count = 0;
  frame->form_count = 0;
  frame->env = env;
  strcpy(frame->name, name);
  return frame;
}

void frame_pop(Runtime *r) {
  r->top_frame--;
}

const char *frame_mode_to_str(FrameMode frame_mode) {
  if(frame_mode == MODE_NORMAL) return "MODE_NORMAL";
  else if(frame_mode == MODE_DEF) return "MODE_DEF";
  else if(frame_mode == MODE_FUNC_CALL) return "MODE_FUNC_CALL";
  else if(frame_mode == MODE_LAMBDA_RETURN) return "MODE_LAMBDA_RETURN";
  else if(frame_mode == MODE_IMMEDIATE_RETURN) return "MODE_IMMEDIATE_RETURN";
  else if(frame_mode == MODE_DO_BLOCK_RETURN) return "MODE_DO_BLOCK_RETURN";
  else return "UNKNOWN_FRAME_MODE";
}

void eval(Runtime *r) {
  Frame *frame = &r->frames[r->top_frame];
  Obj *form = frame->p;

  #if LOG_EVAL
  printf("Eval in frame %d (%s), p is: ", frame->depth, frame_mode_to_str(frame->mode));
  print_obj(form);
  printf("\n");
  #endif
  #if LOG_VALUE_STACK
  gc_stack_print(r->gc);
  #endif
  #if LOG_ENV
  printf("Env: ");
  print_obj(frame->env);
  printf("\n");
  #endif

  if(frame->mode == MODE_LAMBDA_RETURN) {
    frame_pop(r);
  }
  else if(frame->mode == MODE_IMMEDIATE_RETURN) {
    frame_pop(r);
  }
  else if(frame->mode == MODE_DO_BLOCK_RETURN) {
    // popping superfluous values from the stack after evaling a do-form
    //printf("Will pop %d superfluous values from stack: \n", frame->form_count - 1);
    //gc_stack_print(r->gc);
    Obj *top_value = gc_stack_pop(r->gc);
    for(int i = 0; i < frame->form_count - 1; i++) {
      gc_stack_pop(r->gc);
    }
    gc_stack_push(r->gc, top_value);
    frame_pop(r);
  }
  else if(form->type == NUMBER || form->type == STRING) {
    gc_stack_push(r->gc, form);
    frame_pop(r);
  }
  else if(form->type == SYMBOL) {
    //printf("Looking up symbol %s\n", obj_to_str(form));
    Obj *result = runtime_env_lookup(frame->env, form);
    if(result) {
      gc_stack_push(r->gc, result);
      frame_pop(r);
    }
    else {
      printf("Can't find a symbol named '%s'.\n", form->name);
      gc_stack_push(r->gc, r->nil);
      frame_pop(r);
    }
  }
  else if(form->type == CONS) {
    if(form->car == NULL) {
      //printf("Found ()\n");
      gc_stack_push(r->gc, r->nil); // if car is NULL it should be the magical nil value (empty list)
      frame_pop(r);
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "def") == 0) {
      if(frame->mode == MODE_NORMAL) {
	// Enter a new frame where the value of the def will be determined
	Obj *value_expr = form->cdr->cdr->car;
	frame->mode = MODE_DEF; // this frame is now in special mode, waiting only to set the def
	frame_push(r, frame->env, value_expr, "def");
      }
      else if(frame->mode == MODE_DEF) {
	Obj *key = form->cdr->car;
	Obj *value = gc_stack_pop(r->gc);
	if(value) {
	  runtime_env_assoc(r, frame->env, key, value);
	  //Obj *ok = gc_make_symbol(r->gc, "OKAY");
	  //gc_stack_push(r->gc, ok);
	  gc_stack_push(r->gc, key);
	  //runtime_inspect_env(r);
	} else {
	  printf("Can't define %s to NULL.\n", obj_to_str(key));
	  gc_stack_push(r->gc, r->nil);
	}
	frame_pop(r);
      }	
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "quote") == 0) {
      gc_stack_push(r->gc, form->cdr->car);
      frame_pop(r);
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "if") == 0) {
      if(frame->mode == MODE_NORMAL) {
	frame->mode = MODE_IF_BRANCH;
	frame_push(r, frame->env, form->cdr->car, "eval_if_condition");
      }
      else if(frame->mode == MODE_IF_BRANCH) {
	frame->mode = MODE_IF_RETURN;
	Obj *condition = gc_stack_pop(r->gc);
	if(!eq(condition, r->nil)) {
	  frame_push(r, frame->env, form->cdr->cdr->car, "eval_true_branch");
	} else {
	  frame_push(r, frame->env, form->cdr->cdr->cdr->car, "eval_false_branch");
	}
      }
      else if(frame->mode == MODE_IF_RETURN) {
	frame_pop(r);
      }
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "do") == 0) {
      Obj *subform = form->cdr;
      const int MAX_NUMBER_OF_SUBFORMS = 1024;
      Obj *subforms[MAX_NUMBER_OF_SUBFORMS];
      int i = 0;
      while(subform && subform->car) {
	if(i > MAX_NUMBER_OF_SUBFORMS) {
	  error("Can't have more subforms in do statement.");
	}
	subforms[i++] = subform->car;
	subform = subform->cdr;
	frame->form_count++;
      }
      // Push in backwards order
      i--;
      for(; i >= 0; i--) {
	/* printf("Pushing subform "); print_obj(subforms[i]); printf("\n"); */
	frame_push(r, frame->env, subforms[i], "do_subform");
      }
      /* printf("Number of subforms: %d\n", frame->form_count); */
      frame->mode = MODE_DO_BLOCK_RETURN;
    }
    else if(form->car->type == SYMBOL &&
	    (strcmp(form->car->name, "fn") == 0 ||
	     strcmp(form->car->name, "λ") == 0)) {
      Obj *arg_names = form->cdr->car; // list item 1
      Obj *body = form->cdr->cdr->car; // list item 2
      Obj *lambda = gc_make_lambda(r->gc, frame->env, arg_names, body);
      gc_stack_push(r->gc, lambda);
      frame_pop(r);
    }    
    else {
      if(frame->mode == MODE_NORMAL) {
	frame_push(r, frame->env, form->car, "eval_first_pos"); // push a frame that will evaluate the first position of the list
	Obj *arg = form->cdr;
	while(arg && arg->car != NULL) {
	  //printf("Arg to eval: %s\n", obj_to_str(arg->car));
	  if(arg->car) {
	    frame_push(r, frame->env, arg->car, "eval_arg"); // push arg to evaluate
	    arg = arg->cdr;
	    frame->arg_count++;
	  }
	}
	//printf("Going into MODE_FUNC_CALL with %d args.\n", frame->arg_count);
	frame->mode = MODE_FUNC_CALL;
      }
      else if(frame->mode == MODE_FUNC_CALL) {
	//gc_stack_print(r->gc);
	Obj *f = gc_stack_pop(r->gc);
	#if LOG_FUNC_CALL
	printf("Got lambda or function from the value stack, f = ");
	print_obj(f);
	printf("\n");
	#endif
	if(f == NULL) {
	  printf("f == NULL\n");
	  exit(1);
	}
	else if(f->type == FUNC || f->type == LAMBDA) {	  
	  if(f->type == FUNC) {
	    //printf("Calling func %s with %d args.\n", f->name, frame->arg_count);
	    Obj *args = gc_make_cons(r->gc, NULL, NULL);
	    Obj *last_arg = args;
	    for(int i = 0; i < frame->arg_count; i++) {
	      Obj *value = gc_stack_pop(r->gc);
	      last_arg->car = value;
	      Obj *new_arg = gc_make_cons(r->gc, NULL, NULL);
	      last_arg->cdr = new_arg;
	      last_arg = new_arg;
	    }
	    Obj *result = ((Obj*(*)(Runtime*,Obj*))f->func)(r, args);
	    gc_stack_push(r->gc, result);
	    frame_pop(r);
	  } else {
	    #if LOG_LAMBDA_EVAL
	    printf("Will eval lambda body: ");
	    print_obj(f->cdr);
	    printf("\n");
	    #endif

	    int lambda_arg_count = count(f->car->cdr);
	    //printf("lambda_arg_count = %d\n", lambda_arg_count);
	    if(lambda_arg_count != frame->arg_count) {
	      printf("ERROR! λ with %d arg(s) was called with %d arg(s).\n", lambda_arg_count, frame->arg_count);
	      // Remove the args on the stack
	      for(int i = 0; i < frame->arg_count; i++) {
		gc_stack_pop(r->gc);
	      }
	      frame_pop(r);
	      gc_stack_push(r->gc, r->nil);
	      return;
	    }

	    Obj *parent_env = f->car->car;
	    assert(parent_env);
	    Obj *local_env = runtime_env_make_local(r, parent_env);	    
	    
	    Obj *arg_symbol_cons = f->car->cdr;
	    while(arg_symbol_cons && arg_symbol_cons->car) {
	      Obj *arg_symbol = arg_symbol_cons->car;
	      if(arg_symbol->type != SYMBOL) {
		printf("Must bind symbols as args, found: ");
		print_obj(arg_symbol); printf("\n");
	      }
	      Obj *arg_value = gc_stack_pop(r->gc);
	      #if LOG_LAMBDA_EVAL
	      printf("Binding arg_symbol '%s' to value ", arg_symbol->name);
	      print_obj(arg_value);
	      printf("\n");
	      #endif
	      runtime_env_assoc(r, local_env, arg_symbol, arg_value);
	      arg_symbol_cons = arg_symbol_cons->cdr;
	    }

	    if(f->name) {
	      frame_push(r, local_env, f->cdr, f->name);
	    } else {
	      frame_push(r, local_env, f->cdr, "eval_lambda_body");
	    }
	    frame->mode = MODE_LAMBDA_RETURN;
	  }
	}
	else {
	  printf("Can't call non-function '%s'.\n", obj_to_str(f));
	  gc_stack_push(r->gc, r->nil);
	  frame_pop(r);
	}
      }
    }
  }
  else {
    printf("Can't eval this form:\n");
    print_obj(form);
    printf("\n");
    exit(1);
    gc_stack_push(r->gc, r->nil);
  }
}

void eval_top_form(Runtime *r, Obj *env, Obj *form, int top_frame_index, int break_frame_index) {
  frame_push(r, env, form, "eval_top_form");
  while(1) {
    if(r->mode == RUNTIME_MODE_RUN) {
      eval(r);
      if(r->top_frame < top_frame_index) {
	//printf("Returning from top form\n");
	return;
      }
      else if(r->top_frame < break_frame_index) {
	// Back at the frame position where the break happened
	//printf("Re-breaking\n");
	r->mode = RUNTIME_MODE_BREAK;
	return;
      }
    }
    else if(r->mode == RUNTIME_MODE_BREAK) {
      runtime_print_frames(r);
      printf("Debug REPL, press return to continue execution.\n");
      printf("➜ ");
      const int BUFFER_SIZE = 256;
      char str[BUFFER_SIZE];
      fgets(str, BUFFER_SIZE, stdin);
      r->mode = RUNTIME_MODE_RUN;
      if(strlen(str) > 0) {
	Frame *top = &r->frames[r->top_frame];
	runtime_eval_internal(r, top->env, str, 0, r->top_frame + 1, true);
      } else {
	// just run
      }
    }
    else if(r->mode == RUNTIME_MODE_FINISHED) {
      printf("\n");
      return;
    }
  }
  printf("MAX EXECUTIONS REACHED, WILL INTERRUPT\n");
}

void runtime_eval(Runtime *r, const char *source) {
  runtime_eval_internal(r, r->global_env, source, 0, -1, true);
}

void runtime_eval_internal(Runtime *r, Obj *env, const char *source, int top_frame_index, int break_frame_index, bool print_result) {
  Obj *top_level_forms = parse(r->gc, source);
  Obj *current_form = top_level_forms;
  // WHEN DOES current_form BECOME NULL?
  while(current_form && current_form->car) {
    eval_top_form(r, env, current_form->car, top_frame_index, break_frame_index);
    Obj *result = gc_stack_pop(r->gc);
    if(print_result && result) {
      print_obj(result);
      printf("\n");
    }
    current_form = current_form->cdr;
  }
}

void runtime_inspect_env(Runtime *r) {
  //printf("Global env: ");
  print_obj(r->global_env);
  printf("\n");
}
