#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdlib.h>
#include "Obj.h"

typedef enum {
  UNINITIALIZED = 0,
  RETURN = 3,
  PUSH_CONSTANT = 4,
  LOOKUP_AND_PUSH = 5,
  DEFINE = 6,
  CALL = 7,
  PUSH_LAMBDA = 8,
  JUMP = 9,
  IF = 10,
  POP_AND_DISCARD = 11,
  ADD = 12,
  SUB = 13,
  MUL = 14,
  DIV = 15,
  DIRECT_LOOKUP_VAR = 16,
  TAIL_CALL = 17,
  LOOKUP_ARG = 19,
  END_OF_CODES = 666,
} Code;

typedef struct {
  Code *codes;
  int size;
  int pos;
} CodeWriter;

const char *code_to_str(Code code);
Code *code_print_single(Code *code);
void code_print(Code *code_block);

bool pushes_obj(Code code);
bool pushes_int(Code code);

CodeWriter *code_writer_init(CodeWriter *writer, int size);

// All these functions return the number of slots in the code array they take up -- TODO: remove return value!
void code_write_push_constant(CodeWriter *writer, Obj *o);
void code_write_lookup_and_push(CodeWriter *writer, Obj *sym);
void code_write_define(CodeWriter *writer, Obj *sym);
void code_write_call(CodeWriter *writer, int arg_count);
void code_write_tail_call(CodeWriter *writer, int arg_count);
void code_write_end(CodeWriter *writer);
void code_write_return(CodeWriter *writer);
void code_write_push_lambda(CodeWriter *writer, Obj *args, Obj *body, Code *code);
void code_write_jump(CodeWriter *writer, int jump_length);
void code_write_if(CodeWriter *writer);
void code_write_pop(CodeWriter *writer);
void code_write_code(CodeWriter *writer, Code code);
void code_write_direct_lookup_var(CodeWriter *writer, Obj *binding_pair);
void code_write_lookup_arg(CodeWriter *writer, int arg_index);

#endif
