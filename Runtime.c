#include "Runtime.h"
#include "Parser.h"
#include "BuiltinFuncs.h"
#include "Compiler.h"

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

void runtime_eval_internal(Runtime *r, Obj *env, const char *source, bool print_result, int top_frame_index, int break_frame_index);

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
    runtime_eval_internal(r, r->global_env, buffer, false, r->top_frame + 1, -1);
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
  register_func(r, "load", &runtime_load);
  register_func(r, "help", &help);
  register_func(r, "print-code", &print_code);
}

void register_basics(Runtime *r) {
  register_func(r, "gc", &runtime_gc_collect);
  register_func(r, "env", &runtime_env);
  register_func(r, "stack", &runtime_print_stack);
}

void register_builtin_vars(Runtime *r) {
  register_var(r, "nil", r->nil);
  register_var(r, "false", r->nil);
  register_var(r, "true", r->true_val);
}

Runtime *runtime_new(bool builtins) {
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
  register_basics(r);
  if(builtins) {
    register_builtin_funcs(r);
    register_builtin_vars(r);
  }
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
    exit(1);
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

Obj *fetch_args(Runtime *r, int arg_count) {
  // TODO: Use a C-array to pass args instead!
  Obj *args = gc_make_cons(r->gc, NULL, NULL);
  Obj *last_arg = args;
  for(int i = 0; i < arg_count; i++) {
    Obj *value = gc_stack_pop(r->gc);
    Obj *new_arg = gc_make_cons(r->gc, value, last_arg);
    last_arg = new_arg;
  }
  /* printf("Fetched %d args: ", arg_count); */
  /* print_obj(last_arg); */
  /* printf("\n"); */
  return last_arg;
}

void call_func(Runtime *r, Obj *f, int arg_count) {
  Obj *args = fetch_args(r, arg_count);
  Obj *result = ((Obj*(*)(Runtime*,Obj*))f->func)(r, args);
  gc_stack_push(r->gc, result);
}

void call_lambda(Runtime *r, Obj *f, int arg_count) {

  int proper_arg_count = count(GET_ARGS(f));
  if(proper_arg_count != arg_count) {
    printf("Can't call function %s with %d args (should be %d).\n", obj_to_str(f), arg_count, proper_arg_count);
    gc_stack_push(r->gc, r->nil);
    return;
  }
  
  Obj *args = fetch_args(r, arg_count);

  Obj *parent_env = f->car->car;
  assert(parent_env);
  Obj *local_env = runtime_env_make_local(r, parent_env);

  Obj *arg_symbol_cons = f->car->cdr;
  Obj *arg_value_cons = args;
  
  while(arg_symbol_cons && arg_symbol_cons->car &&
	arg_value_cons  && arg_value_cons->car) {
    Obj *arg_symbol = arg_symbol_cons->car;
    if(arg_symbol->type != SYMBOL) {
      printf("Must bind symbols as args, found: ");
      print_obj(arg_symbol); printf("\n");
    }
    Obj *arg_value = arg_value_cons->car;
    /* printf("Binding arg_symbol '%s' to value ", arg_symbol->name); */
    /* print_obj(arg_value); */
    /* printf("\n"); */
    runtime_env_assoc(r, local_env, arg_symbol, arg_value);
    arg_symbol_cons = arg_symbol_cons->cdr;
    arg_value_cons = arg_value_cons->cdr;
  }
  
  Obj *bytecode = f->cdr->cdr;
  assert(bytecode->type == BYTECODE);
  runtime_frame_push(r, local_env, (Code*)bytecode->code, "call_lambda");
}

Obj *read_next_code_as_obj(Frame *frame) {
  Code *cp = frame->p;
  Obj **oo = (Obj**)cp;
  Obj *o = *oo;
  //printf("read o = %p\n", o);
  frame->p += 2;
  return o;
}

int read_next_code_as_int(Frame *frame) {
  Code *cp = frame->p;
  int *ip = (int*)cp;
  int i = *ip;
  //printf("read i = %d\n", i);
  frame->p++;
  return i;
}

void runtime_step_eval(Runtime *r) {
  Frame *frame = &r->frames[r->top_frame];

  Code code = *frame->p;
  
  printf("%s> ", frame->name);
  code_print_single(frame->p);
  printf("\n");
  
  frame->p++;

  int old_obj_count = g_obj_count;

  if(code == RETURN || code == END_OF_CODES) {
    runtime_frame_pop(r);
  }
  else if(code == IF) {
    Obj *value = gc_stack_pop(r->gc);
    if(eq(value, r->nil)) {
      frame->p += 2;
    } else {
      // do nothing
    }
  }
  else if(code == JUMP) {
    int jump_length = read_next_code_as_int(frame);
    frame->p += jump_length;
  }
  else if(code == PUSH_CONSTANT) {
    Obj *o = read_next_code_as_obj(frame);
    gc_stack_push(r->gc, o);
    //printf("Constant: %s\n", obj_to_str(o));
  }
  else if(code == LOOKUP_AND_PUSH) {
    Obj *sym = read_next_code_as_obj(frame);
    Obj *value = runtime_env_lookup(frame->env, sym);
    if(value) {
      gc_stack_push(r->gc, value);
    } else {
      printf("Can't find value '%s' in environment.\n", sym->name);
      gc_stack_push(r->gc, r->nil);
    }
  }
  else if(code == POP_AND_DISCARD) {
    gc_stack_pop(r->gc);
  }
  else if(code == DEFINE) {
    Obj *sym = read_next_code_as_obj(frame);
    Obj *value = gc_stack_pop(r->gc);
    runtime_env_assoc(r, frame->env, sym, value);
    gc_stack_push(r->gc, sym);
  }
  else if(code == PUSH_LAMBDA) {
    Obj *args = read_next_code_as_obj(frame);
    Obj *body = read_next_code_as_obj(frame);
    Code *bytecode = (Code*)read_next_code_as_obj(frame);
    Obj *lambda = gc_make_lambda(r->gc, frame->env, args, body, bytecode);
    gc_stack_push(r->gc, lambda);
  }
  else if(code == CALL) {
    Obj *f = gc_stack_pop(r->gc);
    int arg_count = read_next_code_as_int(frame);
    //printf("Calling %s '%s' with %d args.\n", type_to_str(f->type), f->name ? f->name : "λ", arg_count);
    if(f->type == FUNC) {
      call_func(r, f, arg_count);
    }
    else if(f->type == LAMBDA) {
      call_lambda(r, f, arg_count);
    }
    else {
      printf("Can't call something that's not a lambda or func: ");
      print_obj(f);
      printf("\n");
      gc_stack_push(r->gc, r->nil);
    }
  }
  else {
    printf("runtime_step_eval can't understand code %s\n", code_to_str(code));
  }

  //runtime_print_frames(r);
  //gc_stack_print(r->gc, false);
  printf("+ %d Obj:s\n", g_obj_count - old_obj_count);
}

void eval_top_form(Runtime *r, Obj *env, Obj *form, int top_frame_index, int break_frame_index) {

  int code_length = 0;
  Code *bytecode = compile(r->gc, form, &code_length);
  //code_print(bytecode);
  
  runtime_frame_push(r, env, bytecode, "top-level");
  while(1) {
    if(r->mode == RUNTIME_MODE_RUN) {
      runtime_step_eval(r);
      if(r->top_frame < top_frame_index) {
	return;
      }
    }
    else if(r->mode == RUNTIME_MODE_FINISHED) {
      printf("\n");
      return;
    }
    else {
      printf("Not implemented.\n");
      exit(1);
    }
  }
}

void runtime_eval_internal(Runtime *r, Obj *env, const char *source, bool print_result, int top_frame_index, int break_frame_index) {
  Obj *top_level_forms = parse(r->gc, source);
  Obj *current_form = top_level_forms;
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

void runtime_eval(Runtime *r, const char *source) {
  runtime_eval_internal(r, r->global_env, source, true, 0, -1);
}

