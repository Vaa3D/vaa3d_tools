#include <stdio.h>
#include <ctype.h>
#include "tz_interface.h"

int Input_Dialog(const char *message, const char *format, void *data)
{
  printf("%s", message);
  int c;
  while (isspace(c = getchar())) {
    if (c == '\n') {
      return 0;
    }
  }

  ungetc(c, stdin);
  scanf("%lf", (double *) data);
  while (((c = getchar()) != '\n') && ((c = getchar()) != '\t'));

  return 1;
}
