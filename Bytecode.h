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
  END_OF_CODES = 666,
} Code;

typedef struct {
  Code *codes;
  int size;
  int pos;
} CodeWriter;

const char *code_to_str(Code code);
void code_print(Code *code_block);

CodeWriter *code_writer_init(CodeWriter *writer, int size);

void code_write_push_constant(CodeWriter *writer, Obj *o);
void code_write_lookup_and_push(CodeWriter *writer, Obj *sym);
void code_write_define(CodeWriter *writer, Obj *sym);
void code_write_call(CodeWriter *writer, int arg_count);
void code_write_end(CodeWriter *writer);
void code_write_return(CodeWriter *writer);

#endif
