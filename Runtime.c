#include "Runtime.h"
#include "Parser.h"
#include "BuiltinFuncs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

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
  Obj *function_ptr = gc_make_func(r->gc, f);
  runtime_env_assoc(r, function_name, function_ptr);
}

void register_builtin_funcs(Runtime *r) {
  register_func(r, "bleh", &bleh);
  register_func(r, "print-sym", &print_sym);
  register_func(r, "print-two-syms", &print_two_syms);
}

Runtime *runtime_new() {
  GC *gc = malloc(sizeof(GC)); // TODO: call gc_new() instead
  gc_init(gc);
  Runtime *r = malloc(sizeof(Runtime));
  r->gc = gc;
  r->global_env = gc_make_cons(gc, NULL, NULL);
  r->nil = gc_make_cons(gc, NULL, NULL);
  r->top_frame = -1;
  gc_stack_push(r->gc, r->global_env); // root the global env so it won't get GC:d
  register_builtin_funcs(r);
  //runtime_inspect_env(r);
  return r;
}

void runtime_delete(Runtime *r) {
  free(r->gc); // TODO: call gc_delete() instead
  free(r);
}

Frame *frame_push(Runtime *r, Obj *start_pos) {
  Frame *frame = &r->frames[++r->top_frame];
  frame->p = start_pos;
  frame->depth = r->top_frame;
  frame->mode = MODE_NORMAL;
  frame->arg_count = 0;
  return frame;
}

void frame_pop(Runtime *r) {
  r->top_frame--;
}

void eval(Runtime *r) {
  Frame *frame = &r->frames[r->top_frame];
  Obj *form = frame->p;

  //printf("Eval in frame %d (mode %d), p is: ", frame->depth, frame->mode);
  //print_obj(form); printf("\n");
  
  if(form->type == CONS) {
    if(form->car == NULL) {
      gc_stack_push(r->gc, r->nil);
    }
    else if(form->car->type == SYMBOL) {
      if(strcmp(form->car->name, "def") == 0) {
	if(frame->mode == MODE_NORMAL) {
	  // Enter a new frame where the value of the def will be determined
	  Obj *value_expr = form->cdr->cdr->car;
	  frame->mode = MODE_DEF; // this frame is now in special mode, waiting only to set the def
	  frame_push(r, value_expr);
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
      else if(strcmp(form->car->name, "quote") == 0) {
	gc_stack_push(r->gc, form->cdr->car);
	frame_pop(r);
      }
      else {
	if(frame->mode == MODE_NORMAL) {
	  frame_push(r, form->car); // push a frame that will evaluate the first position of the list
	  Obj *arg = form->cdr;
	  while(arg && arg->car != NULL) {
	    //printf("Arg to eval: %s\n", obj_to_str(arg->car));
	    if(arg->car) {
	      frame_push(r, arg->car); // push arg to evaluate
	      arg = arg->cdr;
	      frame->arg_count++;
	    }
	  }
	  frame->mode = MODE_FUNC_CALL;
	}
	else if(frame->mode == MODE_FUNC_CALL) {
	  Obj *f = gc_stack_pop(r->gc);
	  if(f == NULL) {
	    printf("f == NULL\n");
	    exit(0);
	  }
	  else if(f->type == FUNC) {
	    printf("Calling func %p with %d args.\n", f->func, frame->arg_count);
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
	  }
	  else {
	    printf("Can't call non-function '%s'.\n", obj_to_str(f));
	    gc_stack_push(r->gc, r->nil);
	  }
	  frame_pop(r);
	}
      }
    }
    else {
      printf("Malformed list:\n");
      print_obj(form);
      printf("\n");
      exit(1);
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
  else {
    printf("Can't eval this form:\n");
    print_obj(form);
    printf("\n");
    exit(1);
    gc_stack_push(r->gc, r->nil);
  }
}

void eval_top_form(Runtime *r, Obj *form) {
  frame_push(r, form);
  for(int i = 0; i < 20; i++) {
    eval(r);
    if(r->top_frame < 0) {
      return;
    }
  }
  printf("MAX EXECUTIONS REACHED, WILL INTERRUPT\n");
}

void runtime_eval(Runtime *r, const char *source) {
  Obj *top_level_forms = parse(r->gc, source);
  Obj *current_form = top_level_forms;
  while(current_form->car) {
    eval_top_form(r, current_form->car);
    Obj *result = gc_stack_pop(r->gc);
    if(result) {
      print_obj(result);
      printf("\n");
    }
    current_form = current_form->cdr;
  }
}

void runtime_inspect_env(Runtime *r) {
  printf("Global env:\n");
  print_obj(r->global_env);
  printf("\n");
}
