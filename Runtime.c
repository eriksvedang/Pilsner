#include "Runtime.h"
#include "Parser.h"
#include "BuiltinFuncs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

void runtime_eval_internal(Runtime *r, const char *source, int top_frame_index);

Obj *runtime_env_find_pair(Obj *env, Obj *key) {
  Obj *current = env;
  while(current->car != NULL) {
    if(eq(current->car->car, key)) {
      return current->car;
    }
    current = current->cdr;
  }
  return NULL;
}

void runtime_env_assoc(Runtime *r, Obj *key, Obj *value) {
  //printf("Will register %s with val %s\n", key->name, obj_to_str(value));
  Obj *pair = runtime_env_find_pair(r->global_env, key);
  if(pair) {
    pair->cdr = value;
  }
  else {
    Obj *new_pair = gc_make_cons(r->gc, key, value);
    Obj *new_env = gc_make_cons(r->gc, new_pair, r->global_env);
    r->global_env = new_env;
  }
}

Obj *runtime_env_lookup(Obj *env, Obj *key) {
  Obj *pair = runtime_env_find_pair(env, key);
  if(pair) {
    return pair->cdr;
  }
  else {
    return NULL;
  }
}

void register_func(Runtime *r, const char *name, void *f) {
  Obj *function_name = gc_make_symbol(r->gc, name);
  Obj *function_ptr = gc_make_func(r->gc, name, f);
  runtime_env_assoc(r, function_name, function_ptr);
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

/* void frame_pop(Runtime *r); */

/* Obj *runtime_run(Runtime *r, Obj *args) { */
/*   printf("RUN!\n"); */
/*   r->mode = RUNTIME_MODE_RUN; */
/*   frame_pop(r); */
/*   return r->nil; */
/* } */

void register_builtin_funcs(Runtime *r) {
  register_func(r, "+", &plus);
  register_func(r, "*", &multiply);
  register_func(r, "break", &runtime_break);
  register_func(r, "quit", &runtime_quit);
  register_func(r, "env", &runtime_env);
  register_func(r, "=", &equal);
}

Runtime *runtime_new() {
  GC *gc = malloc(sizeof(GC)); // TODO: call gc_new() instead
  gc_init(gc);
  Runtime *r = malloc(sizeof(Runtime));
  r->gc = gc;
  r->global_env = gc_make_cons(gc, NULL, NULL);
  r->nil = gc_make_cons(gc, NULL, NULL);
  r->top_frame = -1;
  r->mode = RUNTIME_MODE_RUN;
  gc_stack_push(r->gc, r->global_env); // root the global env so it won't get GC:d
  register_builtin_funcs(r);
  //runtime_inspect_env(r);
  return r;
}

void runtime_delete(Runtime *r) {
  free(r->gc); // TODO: call gc_delete() instead
  free(r);
}

Frame *frame_push(Runtime *r, Obj *start_pos, const char *name) {
  Frame *frame = &r->frames[++r->top_frame];
  frame->p = start_pos;
  frame->depth = r->top_frame;
  frame->mode = MODE_NORMAL;
  frame->arg_count = 0;
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
  else return "UNKNOWN_FRAME_MODE";
}

void eval(Runtime *r) {
  Frame *frame = &r->frames[r->top_frame];
  Obj *form = frame->p;

  /*
  printf("Eval in frame %d (%s), p is: ", frame->depth, frame_mode_to_str(frame->mode));
  print_obj(form);
  printf("\n");
  */
  
  if(form->type == CONS) {
    if(form->car == NULL) {
      gc_stack_push(r->gc, r->nil); // if car is null it should be the magical nil value (empty list)
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "def") == 0) {
      if(frame->mode == MODE_NORMAL) {
	// Enter a new frame where the value of the def will be determined
	Obj *value_expr = form->cdr->cdr->car;
	frame->mode = MODE_DEF; // this frame is now in special mode, waiting only to set the def
	frame_push(r, value_expr, "def");
      }
      else if(frame->mode == MODE_DEF) {
	Obj *key = form->cdr->car;
	Obj *value = gc_stack_pop(r->gc);
	if(value) {
	  runtime_env_assoc(r, key, value);
	  Obj *ok = gc_make_symbol(r->gc, "OK");
	  gc_stack_push(r->gc, ok);
	  //runtime_inspect_env(r);
	} else {
	  gc_stack_push(r->gc, r->nil);
	}
	frame_pop(r);
      }	
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "quote") == 0) {
      gc_stack_push(r->gc, form->cdr->car);
      frame_pop(r);
    }
    else if(form->car->type == SYMBOL && strcmp(form->car->name, "fn") == 0) {
      // TODO: store some kind of local environment
      Obj *lambda = gc_make_lambda(r->gc, form->cdr->car, form->cdr->cdr->car);
      gc_stack_push(r->gc, lambda);
      frame_pop(r);
    }    
    else {
      if(frame->mode == MODE_NORMAL) {
	frame_push(r, form->car, "eval_first_pos"); // push a frame that will evaluate the first position of the list
	Obj *arg = form->cdr;
	while(arg && arg->car != NULL) {
	  //printf("Arg to eval: %s\n", obj_to_str(arg->car));
	  if(arg->car) {
	    frame_push(r, arg->car, "eval_arg"); // push arg to evaluate
	    arg = arg->cdr;
	    frame->arg_count++;
	  }
	}
	frame->mode = MODE_FUNC_CALL;
      }
      else if(frame->mode == MODE_LAMBDA_RETURN) {
	frame_pop(r);
      }
      else if(frame->mode == MODE_FUNC_CALL) {
	Obj *f = gc_stack_pop(r->gc);
	if(f == NULL) {
	  printf("f == NULL\n");
	  exit(1);
	}
	else if(f->type == FUNC || f->type == LAMBDA) {
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
	  if(f->type == FUNC) {
	    Obj *result = ((Obj*(*)(Runtime*,Obj*))f->func)(r, args);
	    gc_stack_push(r->gc, result);
	    frame_pop(r);
	  } else {
	    /* printf("Will eval lambda body: "); */
	    /* print_obj(f->cdr); */
	    /* printf("\n"); */
	    frame_push(r, f->cdr, "eval_lambda_body");
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
  else if(form->type == SYMBOL) {
    //printf("Looking up symbol %s\n", obj_to_str(form));
    Obj *result = runtime_env_lookup(r->global_env, form);
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
  else if(form->type == NUMBER || form->type == STRING) {
    gc_stack_push(r->gc, form);
    frame_pop(r);
  }
  else {
    printf("Can't eval this form:\n");
    print_obj(form);
    printf("\n");
    exit(1);
    gc_stack_push(r->gc, r->nil);
  }
}

static const int MAX_EXECUTIONS = 100;

void eval_top_form(Runtime *r, Obj *form, int top_frame_index) {
  /* printf("Will eval top form: "); */
  /* print_obj(form); */
  /* printf("\n"); */
  frame_push(r, form, "eval_top_form");
  for(int i = 0; i < MAX_EXECUTIONS; i++) {
    if(r->mode == RUNTIME_MODE_RUN) {
      eval(r);
      if(r->top_frame < 0) {
	//printf("Returning from top form\n");
	return;
      }
      else if(r->top_frame < top_frame_index) {
	// Back at the frame position where the break happened
	//printf("Unbreaking\n");
	r->mode = RUNTIME_MODE_BREAK;
	return;
      }
    }
    else if(r->mode == RUNTIME_MODE_BREAK) {
      printf("----------- FRAMES ----------- \n");
      for(int i = r->top_frame; i >= 0; i--) {
	printf("%d\t%s\n", i, r->frames[i].name);
      }
      printf("------------------------------ \n");
      printf("Debug REPL, press return to continue execution.\n");
      printf("> ");
      const int BUFFER_SIZE = 256;
      char str[BUFFER_SIZE];
      fgets(str, BUFFER_SIZE, stdin);
      r->mode = RUNTIME_MODE_RUN;
      if(strlen(str) > 0) {
	runtime_eval_internal(r, str, r->top_frame + 1);
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
  runtime_eval_internal(r, source, 0);
}

void runtime_eval_internal(Runtime *r, const char *source, int top_frame_index) {
  Obj *top_level_forms = parse(r->gc, source);
  Obj *current_form = top_level_forms;
  while(current_form->car) {
    eval_top_form(r, current_form->car, top_frame_index);
    Obj *result = gc_stack_pop(r->gc);
    if(result) {
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
