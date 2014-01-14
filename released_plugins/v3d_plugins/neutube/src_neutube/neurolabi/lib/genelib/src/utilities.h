/*****************************************************************************************\
*                                                                                         *
*  Utilities for allocating memory, opening files, and processing command line arguments  *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  October 2005                                                                  *
*                                                                                         *
\*****************************************************************************************/

/*****************************************************************************************\
*                                                                                         *
*  Copyright (c) Oct. 1, '05 Dr. Gene Myers and Howard Hughes Medical Institute ("HHMI")  *
*                                                                                         *
*  This software is provided and licensed on the following terms.  By downloading,        *
*  using and/or copying the software, you agree that you have read, understood, and       *
*  will comply with these terms.                                                          *
*                                                                                         *
*  Redistribution and use in source and binary forms, with or without modification,       *
*  are permitted provided that the following conditions are met:                          *
*     1. Redistributions of source code must retain the above copyright notice, this      *
*          list of conditions and the following disclaimer.                               *
*     2. Redistributions in binary form must reproduce the above copyright notice, this   *
*          list of conditions and the following disclaimer in the documentation and/or    *
*          other materials provided with the distribution.                                *
*     3. Neither the name of the Howard Hughes Medical Institute nor the names of its     *
*          contributors may be used to endorse or promote products derived from this      *
*          software without specific prior written permission.                            *
*                                                                                         *
*  For use of this software and documentation for purposes other than those listed above, *
*  contact Dr. Gene Myers at:                                                             *
*                                                                                         *
*    Janelia Farms Research Campus                                                        *
*    19700 Helix Drive                                                                    *
*    Ashburn, VA  20147-2408                                                              *
*    Tel:   571.209.4153                                                                  *
*    Fax:   571.209.4083                                                                  *
*    Email: myersg@janelia.hhmi.org                                                       *
*                                                                                         *
*  For any issues regarding HHMI or use of its name, contact:                             *
*                                                                                         *
*    Howard Hughes Medical Institute                                                      *
*    4000 Jones Bridge Road                                                               *
*    Chevy Chase, MD 20815-6789                                                           *
*    (301) 215-8500                                                                       *
*    Email: webmaster@hhmi.org                                                            *
*                                                                                         *
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY    *
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED WARRANTIES   *
*  OF MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR A PARTICULAR PURPOSE ARE          *
*  DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY   *
*  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, *
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR *
*  PROFITS; REASONABLE ROYALTIES; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING         *
*  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF   *
*  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                             *
*                                                                                         *
*  The names of the authors and copyright holders may not be used in advertising or       *
*  publicity pertaining to the software without specific, written prior permission.       *
*  Title to copyright in this software will remain with copyright holders.                *
*                                                                                         *
\*****************************************************************************************/

#ifndef SR_UTILITIES

#define SR_UTILITIES

#include <stdio.h>

#define ASCII 128

/* The usual protected allocation and file opening routines.   */

void *Guarded_Malloc(size_t size, const char *routine);
void *Guarded_Realloc(void *array, size_t size, char *routine);
char *Guarded_Strdup(char *string, char *routine);
FILE *Guarded_Fopen(const char *name, const char *options, const char *routine);

/* Process the command line according to 'spec' (See Appendix A for details).  Any
   failure results in an error message and the end of execution.  One tricky thing
   here is that 'spec' itself is interpreted, so an invalid spec will also produce
   an error message.  Be sure to debug the spec!  If no_escapes is non-zero then any
   escaping quotes in the specification will not be seen in a usage statement, should
   one be printed out with an error.                                                  */ 

void Process_Arguments(int argc, char *argv[], char *spec[], int no_escapes);

/* Once the command line has been parsed you can get the value of any argument by calling
   the appropriately typed 'Get' routine with the name of the argument.  You presumably
   know the type since you wrote the spec.  If an argument is an iterator then you must
   further suppply the 'Get' call with the index of the instance you want, where the numbering
   starts at 1.  To know how many instances there are call Get_Repeat_Count. If an argument
   is a multi-value option then you must also specify which value you want.  Is_Arg_Matched
   will tell you if any particular argument has been matched on the command line or not.
   Finally, you can get the program name with Program_Name.                             */

char  *Program_Name();

int    Get_Repeat_Count(char *name);
int    Is_Arg_Matched(char *name, ...);

int    Get_Int_Arg   (char *name, ...);
double Get_Double_Arg(char *name, ...);
char  *Get_String_Arg(char *name, ...);

/* There may be constraints among the arguments that are not captured by the spec that
   you explictly check after the call to Process_Args.  If you detect an error and wish
   to print out a usage message, a call to Print_Argument_Usage will do so on the file
   'file'.  As for Processs_Arguments, passing in a non-zero no_escapes value suppresses
   the printing of escape chars in the statement.                                       */

void Print_Argument_Usage(FILE *file, int no_escapes);

#endif
