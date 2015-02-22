#include "Runtime.h"
#include "Parser.h"
#include "BuiltinFuncs.h"
#include "Compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define TAIL_CALLS_ENABLED 1

#define LOG_EVAL 0
#define LOG_OBJ_COUNT 0
#define LOG_BYTECODE 0
#define LOG_FRAMES 0
#define LOG_VALUE_STACK 0

#define HAS_PARENT_ENV(env) (env->cdr != NULL)

void runtime_eval_internal(Runtime *r, Obj *env, const char *source, bool print_result, int top_frame_index, int break_frame_index);

// The environments root is a cons cell where the car
// contains the a-list and the cdr contains the parent env.

Obj *runtime_env_find_pair(Obj *env, Obj *key, bool allow_parent_search, bool *OUT_found_in_local_env) {
  //printf("Looking for pair for %s in env %p (is global: %d)\n", obj_to_str(key), env, !HAS_PARENT_ENV(env));
  Obj *current = env->car; // get the a-list for this env
  while(current->car) {
    if(eq(current->car->car, key)) {
      if(OUT_found_in_local_env) {
	*OUT_found_in_local_env = HAS_PARENT_ENV(env);
      }
      return current->car;
    }
    current = current->cdr;
  }
  if(allow_parent_search && HAS_PARENT_ENV(env)) {
    return runtime_env_find_pair(env->cdr, key, true, OUT_found_in_local_env);
  } else {
    return NULL;
  }
}

void runtime_env_assoc(Runtime *r, Obj *env, Obj *key, Obj *value) {
  /* printf("Will register %s in env %p with value\n", key->name, env); print_obj(value); printf("\n"); */
  Obj *pair = runtime_env_find_pair(env, key, false, NULL);
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
  Obj *pair = runtime_env_find_pair(env, key, true, NULL);
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

Obj *runtime_break(Runtime *r, Obj *args[], int arg_count) {
  r->mode = RUNTIME_MODE_BREAK;
  return r->nil;
}

Obj *runtime_quit(Runtime *r, Obj *args[], int arg_count) {
  r->mode = RUNTIME_MODE_FINISHED;
  return r->nil;
}

Obj *runtime_env(Runtime *r, Obj *args[], int arg_count) {
  runtime_inspect_env(r);
  return r->nil;
}

Obj *runtime_print_stack(Runtime *r, Obj *args[], int arg_count) {
  gc_stack_print(r->gc, false);
  return r->nil;
}

void runtime_inspect_env(Runtime *r) {
  print_obj(r->global_env);
  printf("\n");
}

void runtime_print_frames(Runtime *r) {
  printf("\n\e[35m");
  printf("______ CALL STACK ______ \n");
  for(int i = r->top_frame; i >= 0; i--) {
    printf("%d\t%s\n", i, r->frames[i].name);
  }
  printf("________________________ \n");
  printf("\e[0m\n");
}

Obj *runtime_gc_collect(Runtime *r, Obj *args[], int arg_count) {
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
    buffer = malloc (length + 1);
    if (buffer)	{
      fread (buffer, 1, length, f);
      buffer[length] = '\0';
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

Obj *runtime_load(Runtime *r, Obj *args[], int arg_count) {
  const char *filename = args[0]->name;
  if(runtime_load_file(r, filename, false)) {
    Obj *done = gc_make_symbol(r->gc, "DONE");
    return done;
  } else {
    return r->nil;
  }
}

void register_builtin_funcs(Runtime *r) {
  register_func(r, "+", &plus);
  register_func(r, "=", &equal);
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
  register_func(r, "help", &help);
  register_func(r, "print-code", &print_code);
}

void register_basics(Runtime *r) {
  register_func(r, "load", &runtime_load);
  register_func(r, "env", &runtime_env);
  register_func(r, "stack", &runtime_print_stack);
  register_func(r, "gc", &runtime_gc_collect);
}

void register_builtin_vars(Runtime *r) {
  register_var(r, "nil", r->nil);
  register_var(r, "false", r->nil);
  register_var(r, "true", r->true_val);
}

Runtime *runtime_new(bool builtins) {
  GC *gc = gc_new();
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
  gc_delete(r->gc);
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
  return last_arg;
}

void call_func(Runtime *r, Obj *f, int arg_count) {
  Obj *args[arg_count];
  for(int i = arg_count - 1; i >= 0; i--) {
    args[i] = gc_stack_pop(r->gc);
  }
  Obj *result = ((Obj*(*)(Runtime*,Obj**,int))f->func)(r, args, arg_count);
  gc_stack_push(r->gc, result);
}

Obj *bind_args_in_new_env(Runtime *r, Obj *parent_env, Obj *arg_symbols, Obj *arg_values, int arg_count) {
  
  Obj *local_env = runtime_env_make_local(r, parent_env);

  Obj *arg_symbol_cons = arg_symbols;
  Obj *arg_value_cons = arg_values;
  
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

  return local_env;
}

void call_lambda(Runtime *r, Obj *f, int arg_count, bool tail_call) {
  int proper_arg_count = count(GET_ARGS(f));
  if(proper_arg_count != arg_count) {
    printf("Can't call function %s with %d args (should be %d).\n", obj_to_str(f), arg_count, proper_arg_count);
    gc_stack_push(r->gc, r->nil);
    return;
  }

  Obj *parent_env = f->car->car;
  assert(parent_env);
  Obj *arg_symbols = f->car->cdr;
  Obj *arg_values = fetch_args(r, arg_count);
  Obj *local_env = bind_args_in_new_env(r, parent_env, arg_symbols, arg_values, arg_count);
  
  Obj *bytecode = f->cdr->cdr;
  assert(bytecode->type == BYTECODE);

#if TAIL_CALLS_ENABLED  
  if(tail_call) {
    runtime_frame_replace(r, local_env, (Code*)bytecode->code, "tail_call_lambda");
  } else {
    runtime_frame_push(r, local_env, (Code*)bytecode->code, "call_lambda");
  }
#else
  runtime_frame_push(r, local_env, (Code*)bytecode->code, "call_lambda");
#endif
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

  #if LOG_EVAL
  printf("%s> ", frame->name);
  code_print_single(frame->p);
  printf("\n");
  int old_obj_count = g_obj_count;
  #endif
  
  frame->p++;

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
  else if(code == DIRECT_LOOKUP_VAR) {
    Obj *binding_pair = read_next_code_as_obj(frame);
    gc_stack_push(r->gc, binding_pair->cdr); // the value is stored in the cdr of the binding pair
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
  else if(code == ADD) {
    double a = gc_stack_pop(r->gc)->number;
    double b = gc_stack_pop(r->gc)->number;
    gc_stack_push(r->gc, gc_make_number(r->gc, a + b));
  }
  else if(code == SUB) {
    double a = gc_stack_pop(r->gc)->number;
    double b = gc_stack_pop(r->gc)->number;
    gc_stack_push(r->gc, gc_make_number(r->gc, b - a));
  }
  else if(code == MUL) {
    double a = gc_stack_pop(r->gc)->number;
    double b = gc_stack_pop(r->gc)->number;
    gc_stack_push(r->gc, gc_make_number(r->gc, a * b));
  }
  else if(code == DIV) {
    double a = gc_stack_pop(r->gc)->number;
    double b = gc_stack_pop(r->gc)->number;
    gc_stack_push(r->gc, gc_make_number(r->gc, b / a));
  }
  else if(code == DEFINE) {
    Obj *sym = read_next_code_as_obj(frame);
    Obj *value = gc_stack_pop(r->gc);
    runtime_env_assoc(r, r->global_env, sym, value);
    gc_stack_push(r->gc, sym);
  }
  else if(code == PUSH_LAMBDA) {
    Obj *args = read_next_code_as_obj(frame);
    Obj *body = read_next_code_as_obj(frame);
    Code *bytecode = (Code*)read_next_code_as_obj(frame);
    Obj *lambda = gc_make_lambda(r->gc, frame->env, args, body, bytecode);
    gc_stack_push(r->gc, lambda);
  }
  else if(code == CALL || code == TAIL_CALL) {
    Obj *f = gc_stack_pop(r->gc);
    int arg_count = read_next_code_as_int(frame);
    if(f->type == FUNC) {
      call_func(r, f, arg_count);
    }
    else if(f->type == LAMBDA) {
      call_lambda(r, f, arg_count, (code == TAIL_CALL));
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

#if LOG_FRAMES
  runtime_print_frames(r);
#endif

#if LOG_VALUE_STACK
  gc_stack_print(r->gc, false);
#endif

#if LOG_OBJ_COUNT
  printf("+ %d Obj:s\n", g_obj_count - old_obj_count);
#endif
}

void eval_top_form(Runtime *r, Obj *env, Obj *form, int top_frame_index, int break_frame_index) {

  int code_length = 0;
  Code *bytecode = compile(r, env, false, form, &code_length);
  #if LOG_BYTECODE
  code_print(bytecode);
  #endif
  int old_obj_count = g_obj_count;
  
  runtime_frame_push(r, env, bytecode, "top-level");
  
  while(1) {
    if(r->top_frame <= break_frame_index) {
      r->mode = RUNTIME_MODE_BREAK;
      break;
    }

    if(r->mode == RUNTIME_MODE_RUN) {
      runtime_step_eval(r);
      if(r->top_frame < top_frame_index) {
	//printf("Breaking out of eval top form.\n");
	break;
      }
    }
    else if(r->mode == RUNTIME_MODE_FINISHED) {
      printf("\n");
      break;
    }
    else {
      runtime_print_frames(r);
      printf("Debug REPL, press return to continue execution.\n");
      printf("\e[35m➜\e[0m ");
      const int BUFFER_SIZE = 2048;
      char str[BUFFER_SIZE];
      fgets(str, BUFFER_SIZE, stdin);
      r->mode = RUNTIME_MODE_RUN;
      if(strlen(str) > 0) {
	runtime_eval_internal(r, env, str, true, 0, r->top_frame);
      }
      else {
	// continue normal execution
      }
    }
  }

  //printf("+ %d Obj:s\n", g_obj_count - old_obj_count);
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

