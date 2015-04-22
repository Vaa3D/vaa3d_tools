

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


// Modified by Hanchuan Peng so that the code can be used for Mac
// 01/26/2006

#ifndef _GNU_GETLINE_H_
# define _GNU_GETLINE_H_ 1

# include <stdio.h>

# ifndef PARAMS
#  if defined (__GNUC__) || __STDC__
#   define PARAMS(args) args
#  else
#   define PARAMS(args) ()
#  endif
# endif

ssize_t gnu_getline (char **lineptr, size_t *n, FILE *stream);
ssize_t gnu_getdelim (char **lineptr, size_t *n, int delim, FILE *stream);


//extern int getline PARAMS ((char **_lineptr, size_t *_n, FILE *_stream));
//extern int getdelim PARAMS ((char **_lineptr, size_t *_n, int _delimiter, FILE *_stream));

#endif /* ! _GETLINE_H_ */


