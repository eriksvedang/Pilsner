#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "GC.h"
#include "Obj.h"
#include "Parser.h"
#include "Repl.h"
#include "Runtime.h"
#include "Tests.h"

void tests() {
  test_gc();
  //test_printing();
  //test_parsing();
  test_runtime();
}

int main()
{
  //printf("Size of int: %lu\nSize of Obj:%lu", sizeof(int), sizeof(Obj));
  //tests();
  repl();
}

