#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdlib.h>
#include "Obj.h"

typedef enum {
  END_OF_CODES,
  RETURN,
  PUSH_CONSTANT,
  UNINITIALIZED,
} Code;

typedef struct {
  Code *codes;
  int size;
  int pos;
} CodeWriter;

const char *code_to_str(Code code);

CodeWriter *code_writer_init(CodeWriter *writer, int size);

void code_write_push_constant(CodeWriter *writer, Obj *o);
void code_write_end(CodeWriter *writer);
void code_write_return(CodeWriter *writer);

#endif
