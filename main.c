#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "GC.h"
#include "Obj.h"
#include "Parser.h"
#include "Repl.h"
#include "Runtime.h"
#include "Tests.h"

int main(int argc, char *argv[]) {
  //tests();
  repl();
}

