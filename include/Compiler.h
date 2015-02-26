#ifndef COMPILER_H
#define COMPILER_H

#include "Obj.h"
#include "Bytecode.h"
#include "GC.h"
#include "Runtime.h"

Code *compile(Runtime *r, bool tail_position, Obj *form, int *OUT_code_length, Obj *args);
void compile_and_print(const char *source);

#endif
