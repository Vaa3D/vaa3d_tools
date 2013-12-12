/*****************************************************************************************\
*                                                                                         *
*  Utilities for allocating memory, opening files, and processing command line arguments  *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  October 2005                                                                  *
*                                                                                         *
\*****************************************************************************************/

#ifndef _SR_UTILITIES

#define _SR_UTILITIES


#include <stdio.h>

#ifdef _MSC_VER

#define strdup(s)  _strdup(s)
#define inline     __inline

#endif

#define ASCII 128

namespace mylib {
void *Guarded_Malloc(size_t size, const char *routine);
void *Guarded_Realloc(void *array, size_t size, const char *routine);
char *Guarded_Strdup(char *string, const char *routine);
FILE *Guarded_Fopen(char *name, char *options, const char *routine);

void Process_Arguments(int argc, char *argv[], char *spec[], int no_escapes);

char  *Program_Name();

int    Get_Repeat_Count(char *name);
int    Is_Arg_Matched(char *name, ... /* [int no] */ );

int    Get_Int_Arg   (char *name, ... /* [int no [int an]] */ );
double Get_Double_Arg(char *name, ... /* [int no [int an]] */ );
char  *Get_String_Arg(char *name, ... /* [int no [int an]] */ );

void Print_Argument_Usage(FILE *file, int no_escapes);
}

#endif
