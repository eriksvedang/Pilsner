#include "Runtime.h"
#include "Parser.h"
#include "BuiltinFuncs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define LOG_EVAL 1
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
  gc_stack_print(r->gc, false);
  return r->nil;
}

void runtime_inspect_env(Runtime *r) {
  //printf("Global env: ");
  print_obj(r->global_env);
  printf("\n");
}

void runtime_print_frames(Runtime *r) {
  //printf("\n\e[35m");
  printf("----------- FRAMES ----------- \n");
  for(int i = r->top_frame; i >= 0; i--) {
    printf("%d\t%s\n", i, r->frames[i].name);
  }
  printf("------------------------------ \n");
  //printf("\e[0m\n");
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
    //runtime_eval(r, r->global_env, buffer, r->top_frame + 1, r->top_frame + 1, false);
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
  register_func(r, "time", &get_time);
  
  register_func(r, "break", &runtime_break);
  register_func(r, "quit", &runtime_quit);
  register_func(r, "env", &runtime_env);
  register_func(r, "load", &runtime_load);
  register_func(r, "stack", &runtime_print_stack);
  register_func(r, "gc", &runtime_gc_collect);
  register_func(r, "help", &help);
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

Frame *runtime_frame_init(Runtime *r, Obj *env, Code *code, const char *name) {
  Frame *frame = &r->frames[r->top_frame];
  frame->p = code;
  frame->env = env;
  strcpy(frame->name, name);
  return frame;
}

Frame *runtime_frame_push(Runtime *r, Obj *env, Code *code, const char *name) {
  r->top_frame++;
  if(r->top_frame >= MAX_FRAMES) {
    printf("Can't push more stack frames, reached max limit: %d.\n", MAX_FRAMES);
    exit(0);
  }
  return runtime_frame_init(r, env, code, name);
}

void runtime_frame_pop(Runtime *r) {
  r->top_frame--;
}

// Changes the current frame, just as if popping and then pushing a new one.
Frame *runtime_frame_replace(Runtime *r, Obj *env, Code *code, const char *name) {
  if(r->top_frame < 0) {
    error("Can't replace top frame because there are no frames.\n");
  }
  return runtime_frame_init(r, env, code, name);
}

void call_func(Runtime *r, Obj *f, int arg_count) {
  // TODO: Use a C-array to pass args instead!
  Obj *args = gc_make_cons(r->gc, NULL, NULL);
  Obj *last_arg = args;
  for(int i = 0; i < arg_count; i++) {
    Obj *value = gc_stack_pop(r->gc);
    last_arg->car = value;
    Obj *new_arg = gc_make_cons(r->gc, NULL, NULL);
    last_arg->cdr = new_arg;
    last_arg = new_arg;
  }
  Obj *result = ((Obj*(*)(Runtime*,Obj*))f->func)(r, args);
  gc_stack_push(r->gc, result);
}

Obj *read_next_code_as_obj(Frame *frame) {
  Code *cp = frame->p;
  Obj **oo = (Obj**)cp;
  Obj *o = *oo;
  //printf("read o = %p\n", o);
  frame->p += 2;
  return o;
}

void runtime_step_eval(Runtime *r) {
  Frame *frame = &r->frames[r->top_frame];

  Code code = *frame->p;
  printf("> %s\n", code_to_str(code));

  if(code == END_OF_CODES) {
    r->mode = RUNTIME_MODE_FINISHED;
    return;
  }
  
  frame->p++;

  if(code == PUSH_CONSTANT) {
    Obj *o = read_next_code_as_obj(frame);
    gc_stack_push(r->gc, o);
    //printf("Constant: %s\n", obj_to_str(o));
  }
  else if(code == LOOKUP_AND_PUSH) {
    Obj *sym = read_next_code_as_obj(frame);
    Obj *value = runtime_env_lookup(frame->env, sym);
    gc_stack_push(r->gc, value);
  }
  else if(code == DEFINE) {
    Obj *sym = read_next_code_as_obj(frame);
    Obj *value = gc_stack_pop(r->gc);
    runtime_env_assoc(r, frame->env, sym, value);
  }
  else if(code == CALL) {
    //gc_stack_print(r->gc, false);
    Obj *f = gc_stack_pop(r->gc);
    int arg_count = 2;
    printf("Calling %s '%s' with %d args.\n", type_to_str(f->type), f->name, arg_count);
    if(f->type == FUNC) {
      call_func(r, f, arg_count);
    }
    else if(f->type == LAMBDA) {
      error("Can't handle lambda yet");
    }
    else {
      error("Can't call something that's not a lambda or func.\n");
      exit(1);
    }
  }
  else if(code == RETURN) {
    
  }
  else {
    printf("Can't understand code %s\n", code_to_str(code));
  }
}

void runtime_eval(Runtime *r, const char *source) {
  
}

