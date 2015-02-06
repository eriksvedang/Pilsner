#include "Error.h"

#include <stdio.h>
#include <stdlib.h>

void error(const char *message) {
  printf("ERROR: %s", message);
  exit(1);
}
