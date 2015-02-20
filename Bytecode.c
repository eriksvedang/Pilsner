#include "Bytecode.h"
#include <stdlib.h>
#include <stdio.h>

const char *code_to_str(Code code) {
  if(code == END_OF_CODES)          return "END     ";
  else if(code == PUSH_CONSTANT)    return "PUSH    ";
  else if(code == RETURN)           return "RETURN  ";
  else if(code == LOOKUP_AND_PUSH)  return "LOOKUP  ";
  else if(code == DEFINE)           return "DEFINE  ";
  else if(code == CALL)             return "CALL!   ";
  else if(code == PUSH_LAMBDA)      return "LAMBDA  ";
  else if(code == JUMP)             return "JUMP -> ";
  else if(code == IF)               return "IF      ";
  else if(code == POP_AND_DISCARD)  return "POP     ";
  else if(code == ADD)              return "ADD     ";
  else if(code == MUL)              return "MUL     ";
  else if(code == SUB)              return "SUB     ";
  else if(code == DIV)              return "DIV     ";
  else if(code == DIRECT_LOOKUP_VAR)return "DIRECT  ";
  else if(code == TAIL_CALL)        return "TAILCALL";
  else if(code == UNINITIALIZED) return "UNINITIALIZED";
  else return "UNKNOWN_CODE";
}

bool pushes_obj(Code code) {
  return (code == PUSH_CONSTANT ||
	  code == LOOKUP_AND_PUSH ||
	  code == DEFINE ||
	  code == DIRECT_LOOKUP_VAR);
}

void print_code_as_obj(Code *code) {
  Code *cp = code;
  Obj **oo = (Obj**)cp;
  Obj *o = *oo;
  print_obj(o);
}

Code *code_print_single(Code *code) {
  printf("%s", code_to_str(*code));
  if(pushes_obj(*code)) {
    code += 1;
    printf(" ");
    print_code_as_obj(code);
    code += 2;
  }
  else if(*code == CALL || *code == JUMP) {
    code += 1;
    Code *cp = code;
    int *ip = (int*)cp;
    int i = *ip;
    printf(" %d", i);
    code += 1;
  }
  else if(*code == PUSH_LAMBDA) {
    printf(" <args> <body> <code>");
    code += 7;
  }
  else {
    code++;
  }
  return code;
}

void code_print(Code *code_block) {
  printf("--- CODE BLOCK ---\n");
  while(*code_block != END_OF_CODES) {
    code_block = code_print_single(code_block);
    printf("\n");
  }
  printf("%s\n", code_to_str(*code_block));
  printf("------------------\n");
}

CodeWriter *code_writer_init(CodeWriter *writer, int size) {
  writer->codes = malloc(size); // TODO: Remember to clean this up somewhere!
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

void int_write(CodeWriter *writer, int i) {
  if(writer->pos >= writer->size) {
    error("Can't write int, code block is full.");
  }
  int *ip = (int*)&(writer->codes[writer->pos]);
  *ip = i;
  writer->pos++;
}

int code_write_push_constant(CodeWriter *writer, Obj *o) {
  code_write(writer, PUSH_CONSTANT);
  obj_write(writer, o);
  return 3;
}

int code_write_lookup_and_push(CodeWriter *writer, Obj *sym) {
  if(sym->type != SYMBOL) {
    error("Can't write LOOKUP_AND_PUSH with non-symbol.");
  }
  code_write(writer, LOOKUP_AND_PUSH);
  obj_write(writer, sym);
  return 3;
}

int code_write_define(CodeWriter *writer, Obj *sym) {
  if(sym->type != SYMBOL) {
    error("Can't write DEFINE with non-symbol.");
  }
  code_write(writer, DEFINE);
  obj_write(writer, sym);
  return 3;
}

int code_write_direct_lookup_var(CodeWriter *writer, Obj *binding_pair) {
  if(binding_pair->type != CONS) {
    error("Can't write DIRECT_LOOKUP_VAR with non-cons.");
  }
  else if(binding_pair->car->type != SYMBOL) {
    error("Can't write DIRECT_LOOKUP_VAR with binding pair that hasn't got symbol in car.");
  }
  code_write(writer, DIRECT_LOOKUP_VAR);
  obj_write(writer, binding_pair);
  return 3;
}

int code_write_push_lambda(CodeWriter *writer, Obj *args, Obj *body, Code *code) {
  code_write(writer, PUSH_LAMBDA);
  obj_write(writer, args);
  obj_write(writer, body);
  obj_write(writer, (Obj*)code); // pointers take up the same amount of space
  return 7;
}

int code_write_call(CodeWriter *writer, int arg_count) {
  code_write(writer, CALL);
  int_write(writer, arg_count);
  return 2;
}

int code_write_tail_call(CodeWriter *writer, int arg_count) {
  code_write(writer, TAIL_CALL);
  int_write(writer, arg_count);
  return 2;
}

int code_write_jump(CodeWriter *writer, int jump_length) {
  code_write(writer, JUMP);
  int_write(writer, jump_length);
  return 2;
}

int code_write_if(CodeWriter *writer) {
  code_write(writer, IF);
  return 1;
}

int code_write_end(CodeWriter *writer) {
  code_write(writer, END_OF_CODES);
  return 1;
}

int code_write_return(CodeWriter *writer) {
  code_write(writer, RETURN);
  return 1;
}

int code_write_pop(CodeWriter *writer) {
  code_write(writer, POP_AND_DISCARD);
  return 1;
}

int code_write_code(CodeWriter *writer, Code code) {
  code_write(writer, code);
  return 1;
}
