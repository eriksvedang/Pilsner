#include "Bytecode.h"
#include <stdlib.h>
#include <stdio.h>

const char *code_to_str(Code code) {
  if(code == END_OF_CODES) return "END_OF_CODES";
  else if(code == PUSH_CONSTANT) return "PUSH_CONSTANT";
  else if(code == UNINITIALIZED) return "UNINITIALIZED";
  else if(code == RETURN) return "RETURN";
  else if(code == LOOKUP_AND_PUSH) return "LOOKUP_AND_PUSH";
  else if(code == DEFINE) return "DEFINE";
  else return "UNKNOWN_CODE";
}

bool pushes_obj(Code code) {
  return (code == PUSH_CONSTANT ||
	  code == LOOKUP_AND_PUSH ||
	  code == DEFINE);
}

void code_print(Code *code_block) {
  printf("--- CODE BLOCK ---\n");
  while(*code_block != END_OF_CODES) {
    printf("%s", code_to_str(*code_block));
    if(pushes_obj(*code_block)) {
      printf(" <obj>");
      code_block += 2;
    }
    code_block++;
    printf("\n");
  }
  printf("------------------\n");
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
  Obj **p = (Obj**)&(writer->codes[writer->pos]);
  *p = o;
  writer->pos += 2;
}

void code_write_push_constant(CodeWriter *writer, Obj *o) {
  code_write(writer, PUSH_CONSTANT);
  obj_write(writer, o);
}

void code_write_lookup_and_push(CodeWriter *writer, Obj *sym) {
  code_write(writer, LOOKUP_AND_PUSH);
  obj_write(writer, sym);
}

void code_write_define(CodeWriter *writer, Obj *sym) {
  code_write(writer, DEFINE);
  obj_write(writer, sym);
}

void code_write_end(CodeWriter *writer) {
  code_write(writer, END_OF_CODES);
}

void code_write_return(CodeWriter *writer) {
  code_write(writer, RETURN);
}
