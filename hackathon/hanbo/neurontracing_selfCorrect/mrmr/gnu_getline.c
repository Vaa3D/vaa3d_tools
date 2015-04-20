

/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999, 2000, 2001, 2002 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA  */

/* First implementation by Alain Magloire */

/* Modified by Hanchuan Peng to make the code useable for Max OS.
   last revision: 01/26/2006
*/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gnu_getline.h"

ssize_t
gnu_getline (char **lineptr, size_t *n, FILE *stream)
{
  return gnu_getdelim (lineptr, n, '\n', stream);
}

#ifndef HAVE_GETDELIM

/* Default value for line length.  */
static const int line_size = 128;

ssize_t
gnu_getdelim (char **lineptr, size_t *n, int delim, FILE *stream)
{
  int indx = 0;
  int c;

  /* Sanity checks.  */
  if (lineptr == NULL || n == NULL || stream == NULL)
    return -1;

  /* Allocate the line the first time.  */
  if (*lineptr == NULL)
    {
      *lineptr = (char *)malloc (line_size);
      if (*lineptr == NULL)
	return -1;
      *n = line_size;
    }

  while ((c = getc (stream)) != EOF)
    {
      /* Check if more memory is needed.  */
      if (indx >= *n)
	{
	  *lineptr = (char *)realloc (*lineptr, *n + line_size);
	  if (*lineptr == NULL)
	    return -1;
	  *n += line_size;
	}

      /* Push the result in the line.  */
      (*lineptr)[indx++] = c;

      /* Bail out.  */
      if (c == delim)
	break;
    }

  /* Make room for the null character.  */
  if (indx >= *n)
    {
      *lineptr = (char *)realloc (*lineptr, *n + line_size);
      if (*lineptr == NULL)
       return -1;
      *n += line_size;
    }

  /* Null terminate the buffer.  */
  (*lineptr)[indx++] = 0;

  /* The last line may not have the delimiter, we have to
   * return what we got and the error will be seen on the
   * next iteration.  */
  return (c == EOF && (indx - 1) == 0) ? -1 : indx - 1;
}

#endif /* HAVE_GETDELIM */


#ifdef STANDALONE
int main(void)
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  fp = fopen("/etc/passwd", "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);
  while ((read = gnu_getline(&line, &len, fp)) != -1) {
    printf("Retrieved line of length %zu :\n", read);
    printf("%s", line);
  }
  if (line)
    free(line);
  return EXIT_SUCCESS;
}
#endif


