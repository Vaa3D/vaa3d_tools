/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




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

void *Guarded_Malloc(int size, const char *routine);
void *Guarded_Realloc(void *array, int size, const char *routine);
char *Guarded_Strdup(const char *string, const char *routine);
FILE *Guarded_Fopen(char *name, char *options, const char *routine);

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
