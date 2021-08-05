/*****************************************************************************************\
*                                                                                         *
*  Utilities for allocating memory, opening files, and processing command line arguments  *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  October 2005                                                                  *
*                                                                                         *
\*****************************************************************************************/

/* changed the include file name by PHC 060816*/

#ifndef MG_SR_UTILITIES

#define MG_SR_UTILITIES

#include <stdio.h>

#define ASCII 128

/* The usual protected allocation and file opening routines.   */

void *Guarded_Malloc(int size, char *routine);
void *Guarded_Realloc(void *array, int size, char *routine);
char *Guarded_Strdup(char *string, char *routine);
FILE *Guarded_Fopen(char *name, char *options, char *routine);

/* The structure of the command line to be interpreted is specified in an array of
   'Arg_Spec' records the last of which is {NULL,NULL,NULL} signalling the end of the
   spec.  An arg spec consists of three string fields that are described in detail in
   Appendix A.                                                                         */

typedef struct
  { char *name;    //  lookup name and command line string if an option
    char *type;    //  syntax and form of the argument
    char *defval;  //  default value (if any)
  } Arg_Spec;

/* Process the command line according to 'spec'.  All arguments are parsed and checked.
   All required argument must be present.  Any failure results in an error message and
   the end of execution.  One tricky thing here is that 'spec' itself is interpreted,
   so an invalid spec will also produce an error message.  Be sure to debug the spec!  */ 

void  Process_Arguments(int argc, char *argv[], Arg_Spec *spec);

/* Once the command line has been parsed you can get the value of any argument by calling
   the appropriately typed 'Get' routine with the name of the argument.  You presumably
   know the type since you wrote the spec.  If an argument is an iterator then you must
   further suppply the 'Get' call with the index of the instance you want, where the numbering
   starts at 1.  To know how many instances there are call Argument_Cardinality.  For non-iterative
   arguments this routine returns 1 if the argument has a value, and 0 if it does not.
   Finally, you can get the program name with Program_Name.                             */

char  *Program_Name();

int    Argument_Cardinality(char *name);

int    Get_Int_Arg   (char *name, ...);
double Get_Double_Arg(char *name, ...);
char  *Get_String_Arg(char *name, ...);

/* There may be constraints among the arguments that are not captured by the spec that
   you explictly check after the call to Process_Args.  If you detect an error and wish
   to print out a usage message, a call to Print_Argument_Usage will do so on the file 'file'.  */

void Print_Argument_Usage(FILE *file);

#endif
