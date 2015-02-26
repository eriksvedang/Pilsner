#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdlib.h>
#include "Obj.h"

typedef enum {
  UNINITIALIZED = 0,
  PUSH_CONSTANT,     // Places an Obj on the stack.
  PUSH_LAMBDA,       // Creates an Obj of type LAMBDA and places it on the stack.
  DIRECT_LOOKUP_VAR, // Pointer lookup to a binding in an env.
  LOOKUP_ARG,        // Lookup an arg in the current stack frame.
  DEFINE,            // Set (or create if necessary) the value of a binding in the global scope.
  CALL,              // Calls a function, pushing a new stack frame.
  TAIL_CALL,         // Calls a function by replacing the current stack frame.
  JUMP,              // Move the execution pointer 'p' in the current stack frame a certain index forward.
  IF,                // Skips over the next instruction if top value of the stack is nil (false).
  RETURN,            // Pop the current stack frame.
  POP_AND_DISCARD,   // Remove the top value of the value stack. Used by the (do ...) form to remove unwanted values.
  ADD,               // Fast way of adding two numbers from the top of the stack together, pushing the new result.
  SUB,               // See above.
  MUL,               // See above.
  DIV,               // See above.
  EQ,                // See above.
  END_OF_CODES,      // Marks the end of the code block. Any instructions after this will be ignored.
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

void code_write_push_constant(CodeWriter *writer, Obj *o);
void code_write_define(CodeWriter *writer, Obj *sym);
void code_write_call(CodeWriter *writer, int arg_count);
void code_write_tail_call(CodeWriter *writer, int arg_count);
void code_write_end(CodeWriter *writer);
void code_write_return(CodeWriter *writer);
void code_write_push_lambda(CodeWriter *writer, Obj *args, Obj *body);
void code_write_jump(CodeWriter *writer, int jump_length);
void code_write_if(CodeWriter *writer);
void code_write_pop(CodeWriter *writer);
void code_write_code(CodeWriter *writer, Code code);
void code_write_direct_lookup_var(CodeWriter *writer, Obj *binding_pair);
void code_write_lookup_arg(CodeWriter *writer, int arg_index);

#endif
