#include "Runtime.h"
#include "Parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

Runtime *runtime_new() {
  GC *gc = malloc(sizeof(GC)); // TODO: call gc_new() instead
  gc_init(gc);
  Runtime *r = malloc(sizeof(Runtime));
  r->gc = gc;
  r->global_env = gc_make_cons(gc, NULL, NULL);
  r->nil = gc_make_cons(gc, NULL, NULL);
  r->top_frame = -1;
  return r;
}

void runtime_delete(Runtime *r) {
  free(r->gc); // TODO: call gc_delete() instead
  free(r);
}

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

Obj *runtime_env_assoc(GC *gc, Obj *env, Obj *key, Obj *value) {
  Obj *pair = runtime_env_find_pair(env, key);
  if(pair) {
    pair->cdr = value;
    return env;
  }
  else {
    Obj *new_pair = gc_make_cons(gc, key, value);
    Obj *new_env = gc_make_cons(gc, new_pair, env);
    return new_env;
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

Frame *frame_push(Runtime *r, Obj *start_pos) {
  Frame *frame = &r->frames[++r->top_frame];
  frame->p = start_pos;
  frame->depth = r->top_frame;
  frame->mode = MODE_NORMAL;
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
	    r->global_env = runtime_env_assoc(r->gc, r->global_env, key, value);
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
	error("Not supported.");
	/*
	Obj *f = form->car; // eval(r, form->car);
	if(f == NULL) {
	  gc_stack_push(r->gc, r->nil);
	}
	else if(f->type == FUNC) {
	  printf("Calling function '%s'\n", obj_to_str(f));
	  gc_stack_push(r->gc, f);
	}
	else {
	  printf("Can't call non-function '%s'.\n", obj_to_str(f));
	  gc_stack_push(r->gc, r->nil);
	  }*/
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
