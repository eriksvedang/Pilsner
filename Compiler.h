#ifndef COMPILER_H
#define COMPILER_H

#include "Obj.h"
#include "Bytecode.h"
#include "GC.h"

Code *compile(GC *gc, Obj *form, int *OUT_code_length);
void compile_and_print(const char *source);

#endif
