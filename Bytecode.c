#include "Bytecode.h"
#include <stdlib.h>
#include <stdio.h>

const char *code_to_str(Code code) {
  if(code == END_OF_CODES) return "END_OF_CODES";
  else if(code == PUSH_CONSTANT) return "PUSH_CONSTANT";
  else if(code == UNINITIALIZED) return "UNINITIALIZED";
  else if(code == RETURN) return "RETURN";
  else return "UNKNOWN_CODE";
}

CodeWriter *code_writer_init(CodeWriter *writer, int size) {
  writer->codes = malloc(size);
  writer->codes[0] = UNINITIALIZED;
  writer->size = size;
  writer->pos = 0;
  return writer;
}

void code_write(CodeWriter *writer, Code code) {
  if(writer->pos >= writer->size) {
    error("Can't write Code, block is full.");
  }
  writer->codes[writer->pos] = code;
  writer->pos++;
}

void obj_write(CodeWriter *writer, Obj *o) {
  if(writer->pos >= writer->size) {
    error("Can't write Obj*, code block is full.");
  }
  /* Code *c = &writer->codes[writer->pos]; */
  /* Obj **co = (Obj**)c; */
  /* *co = o; */
  /* writer->pos += 2; */

  Obj **p = (Obj**)&(writer->codes[writer->pos]);
  *p = o;
}

void code_write_push_constant(CodeWriter *writer, Obj *o) {
  code_write(writer, PUSH_CONSTANT);
  obj_write(writer, o);
}

void code_write_end(CodeWriter *writer) {
  code_write(writer, END_OF_CODES);
}

void code_write_return(CodeWriter *writer) {
  code_write(writer, RETURN);
}
