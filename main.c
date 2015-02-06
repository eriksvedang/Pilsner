#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "GC.h"
#include "Obj.h"
#include "Tests.h"
#include "Parser.h"

void tests() {
  test_gc();
  //test_printing();
  //test_parsing();
}

int main()
{
  tests();

  
}

