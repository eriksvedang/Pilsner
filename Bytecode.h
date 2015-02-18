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

CodeWriter *code_writer_init(CodeWriter *writer, int size);

// All these functions return the number of slots in the code array they take up
int code_write_push_constant(CodeWriter *writer, Obj *o);
int code_write_lookup_and_push(CodeWriter *writer, Obj *sym);
int code_write_define(CodeWriter *writer, Obj *sym);
int code_write_call(CodeWriter *writer, int arg_count);
int code_write_end(CodeWriter *writer);
int code_write_return(CodeWriter *writer);
int code_write_push_lambda(CodeWriter *writer, Obj *args, Obj *body, Code *code);
int code_write_jump(CodeWriter *writer, int jump_length);
int code_write_if(CodeWriter *writer);
int code_write_pop(CodeWriter *writer);

#endif
