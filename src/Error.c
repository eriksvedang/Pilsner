#include "Error.h"

#include <stdio.h>
#include <stdlib.h>

void error(const char *message) {
  printf("ERROR: %s\n", message);
  exit(1);
}
