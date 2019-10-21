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
//change to .cpp from .c for the JFRC cluster and also correct several c++ compiling errors. Adding <ctype.h> for isalpha()
//071112


#undef DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <ctype.h>

#include "mg_utilities.h"

#define OPTION_SYMBOL '-'   //  meta-symbol choices for command line spec. types
#define ZERO_OR_MORE  '*'
#define ONE_OR_MORE   '+'

#define RETURN_LENGTH 80    // length over which a usage statement will be split accross lines

static const char *Type_Strings[] = { "int", "double", "string" };  //  command spec. base types

void *Guarded_Malloc(int size, const char *routine)
{ void *p;

  p = malloc(size);
  if (p == NULL)
    { fprintf(stderr,"\nError in %s:\n",routine);
      fprintf(stderr,"   Out of memory\n");
      return 0;
    }
  return (p);
}

void *Guarded_Realloc(void *p, int size, const char *routine)
{ p = realloc(p,size);
  if (p == NULL)
    { fprintf(stderr,"\nError in %s:\n",routine);
      fprintf(stderr,"   Out of memory\n");
      return 0;
    }
  return (p);
}

char *Guarded_Strdup(const char *pp, const char *routine)
{ char* p = strdup(pp);
  if (p == NULL)
    { fprintf(stderr,"\nError in %s:\n",routine);
      fprintf(stderr,"   Out of memory\n");
      return 0;
    }
  return (p);
}

FILE *Guarded_Fopen(char *file_name, char *options, char *routine)
{ FILE *f;

  f = fopen(file_name,options);
  if (f == NULL)
    { fprintf(stderr,"\nError in %s:\n",routine);
      fprintf(stderr,"   Cannot open %s\n",file_name);
      return 0;
    }
  return (f);
}

typedef enum { OPTION     = 1,
               OPT_CMPCT  = 2,
               OPT_ARG    = 3,
               REQUIRED   = 4,
               ITERATOR_0 = 5,
               ITERATOR_1 = 6
             } Arg_Kind;

typedef enum { INT_TYPE = 0, DOUBLE_TYPE = 1, STRING_TYPE = 2 } Arg_Type;

typedef union { int intval; double dblval; char *strval; } Arg_Value;

typedef struct
  { char     *name;
    Arg_Kind  kind;
    Arg_Type  type;      
    int       iterate;      //  can be set mulitiple times?
    int       synonym;      //  index of base which must be a pure option, -1 if is a base
    int       has_default;  //  has a default value (options, always set for pure opts.)
    Arg_Value defval;       //  default value (options only)
    int       set;          //  the argument has had its value set (# of times)
    Arg_Value value;        //  the value of the argument
    int       extension;    //  index of OPT whose name you are a prefix of (OPT's only)
    int       prefix;       //  index of OPT whose name you are an extension of
  } Arg_Details;

static Arg_Details *Parse = NULL;   //  argument table
static int          Arg_Len;        //  length of argument table
static char        *Program;        //  name of program

//  Print an error message and quit

static void spec_error(const char *format, const char *arg)
{ fprintf(stderr,"\nError while processing argument specification:\n   ");
  fprintf(stderr,format,arg);
  fprintf(stderr,"\n");
  return;
}

static int interp_error(const char *format, const char *arg)
{ fprintf(stderr,"\nError while interpreting command line:\n   ");
  fprintf(stderr,format,arg);
  fprintf(stderr,"\n\n   ");
  Print_Argument_Usage(stderr);
  return 1;
}

//  Print the current parser table

static char *print_value(Arg_Value *value, int type, int width)
{ static char Form[100];

  switch (type)
  { case INT_TYPE:
      sprintf(Form,"%*d",width,value->intval);
      break;
    case DOUBLE_TYPE:
      sprintf(Form,"%*g",width,value->dblval);
      break;
    case STRING_TYPE:
      sprintf(Form,"%s",value->strval);
      break;
  }
  return (Form);
}

//  Debug routine: show contents of table records

static void print_arg_table(int values)
{ static const char *Print_Kind[] = { "", "FLAG", "OCMP", "OARG", "REQD", "ITR0", "ITR1" };
  static const char *Print_Type[] = { "INT", "DBL", "STR" };

  int   i;
  /* char *s; */

  printf("\n");
  for (i = 0; i < Arg_Len; i++)
    { printf("  %s  %s",Print_Kind[Parse[i].kind],Print_Type[Parse[i].type]);
      if (Parse[i].iterate)
        printf("*");
      else
        printf(" ");
      printf("  %3d",Parse[i].synonym);
      printf("  %3d,%3d",Parse[i].extension,Parse[i].prefix);
      printf("  %s",Parse[i].name);
      if (Parse[i].has_default)
        printf("  (%s)",print_value(&(Parse[i].defval),Parse[i].type,1));
      if (values && Parse[i].synonym < 0)
        { printf("  <%d>",Parse[i].set);
          if (Parse[i].set)
            printf("= %s",print_value(&(Parse[i].value),Parse[i].type,1));
          else
            printf("= NA");
        }
      printf("\n");
    }
}

//  Lookup w'th instance of name in Parse[0..top-1] or return -1 if does not exist

static int lookup_arg(char *name, int top, int w)
{ int i;

  for (i = 0; i < top; i++)
    if (strcmp(Parse[i].name,name) == 0)
      if (w == 1)
        return (i);
      else
        w -= 1;
  return (-1);
}

//  Lookup first option match to name in Parse[0..top-1] or return -1 if not found

static int find_option(char *name, int top)
{ int i;

  for (i = 0; i < top; i++)
    if (Parse[i].kind == OPT_CMPCT && strncmp(Parse[i].name,name,strlen(Parse[i].name)) == 0)
      return (i);
    else if (Parse[i].kind <= OPT_ARG && strcmp(Parse[i].name,name) == 0)
      return (i);
  return (-1);
}

//  Lookup first option prefix match to name in Parse[0..top-1] or return -1 if not found

static int find_extension(char *name, int top)
{ int i;

  for (i = top-1; i >= 0; i--)
    if (Parse[i].kind <= OPT_ARG && strncmp(Parse[i].name,name,strlen(name)) == 0)
      return (i);
  return (-1);
}

//  Parse constant string bptr with respect to given type and return value

static Arg_Value *parse_value(char *bptr, int type, int in_spec)
{ static Arg_Value rez;
  char *eptr;

  switch (type)
  { case INT_TYPE:
      rez.intval = strtol(bptr,&eptr,10);
      if (eptr == bptr || *eptr != '\0')
        if (in_spec)
          spec_error("'%s' is not an integer value",bptr);
        else
          interp_error("'%s' is not an integer value",bptr);
      break;
    case DOUBLE_TYPE:
      rez.dblval = strtod(bptr,&eptr);
      if (eptr == bptr || *eptr != '\0')
        if (in_spec)
          spec_error("'%s' is not a floating point value",bptr);
        else
          interp_error("'%s' is not a floating point value",bptr);
      break;
    case STRING_TYPE:
      rez.strval = Guarded_Strdup(bptr,"Process_Arguments");
      break;
  }
  return (&rez);
}

//  Process the command line arguments according to spec

void Process_Arguments(int argc, char *argv[], Arg_Spec *spec)
{ int  i;
  int  has_flex_arg;     //  is there an interative arg (and its type)
  int  num_reqd_arg;     //  number of non-iterative required arguments
  int  iterator_card;    //  number of instances of iterator arg on command line

  Program = Guarded_Strdup(argv[0],"Process_Arguments");

  // Process the Specification

  for (i = 0; spec[i].name != NULL; i++)
    ;
  Arg_Len = i; 

//  Parse = Guarded_Realloc(Parse,sizeof(Arg_Details)*(Arg_Len+argc),"Process_Arguments");
//071112
  Parse = (Arg_Details*) Guarded_Realloc(Parse,sizeof(Arg_Details)*(Arg_Len+argc),"Process_Arguments");

  has_flex_arg = 0;
  num_reqd_arg = 0;
  for (i = 0; i < Arg_Len; i++)
    { char *s;

      Parse[i].name = Guarded_Strdup(spec[i].name,"Process_Arguments"); // Duplicate argument name
      if (lookup_arg(Parse[i].name,i,1) >= 0)
        spec_error("Argument '%s' is declared twice",Parse[i].name);

      s = spec[i].type;              // Parse argument kind
      Parse[i].iterate   = 0;
      Parse[i].extension = -1;
      Parse[i].prefix    = -1;
      switch (*s++)
      { case OPTION_SYMBOL:
          { int j = find_option(Parse[i].name,i);
            if (j >= 0)
              spec_error("Compact option %s is a prefix of a later option",Parse[j].name);
          }

          if (*s == ZERO_OR_MORE)
            { s += 1;
              Parse[i].iterate = 1;
            }
          Parse[i].kind = OPT_CMPCT;
          if (*s == ' ')
            { s += 1;
              Parse[i].kind = OPT_ARG;
            }
          { int j = find_extension(Parse[i].name,i);
            if (j >= 0)
              { if (Parse[i].kind == OPT_ARG)
                  spec_error("%s is a prefix of an earlier option but not a pure option",
                             Parse[i].name);
                Parse[i].extension = j;
                Parse[j].prefix    = i;
              }
          }
          break;
        case ONE_OR_MORE:
          if (has_flex_arg)
            spec_error("Two iterative arguments in spec",NULL);
          has_flex_arg = ITERATOR_1; //071112
          Parse[i].kind = ITERATOR_1;
          Parse[i].iterate = 1;
          break;
        case ZERO_OR_MORE:
          if (has_flex_arg)
            spec_error("Two iterative arguments in spec",NULL);
          has_flex_arg = ITERATOR_0;
          Parse[i].kind = ITERATOR_0;
          Parse[i].iterate = 1;
          break;
        default:
          Parse[i].kind = REQUIRED;
          num_reqd_arg += 1;
          s--;
      }

      { /* char c, *t; */                      // Parse type of the argument
        int  j;

        Parse[i].synonym = -1;
        if (strncmp(s,Type_Strings[INT_TYPE],strlen(Type_Strings[INT_TYPE])) == 0)
          Parse[i].type = INT_TYPE;
        else if (strncmp(s,Type_Strings[DOUBLE_TYPE],strlen(Type_Strings[DOUBLE_TYPE])) == 0)
          Parse[i].type = DOUBLE_TYPE;
        else if (strncmp(s,Type_Strings[STRING_TYPE],strlen(Type_Strings[STRING_TYPE])) == 0)
          Parse[i].type = STRING_TYPE;
        else if (Parse[i].kind == OPT_CMPCT)
          { if (*s == '\0')
              { Parse[i].kind = OPTION;
                Parse[i].type = INT_TYPE;
              }
            else if ((j = lookup_arg(s,i,1)) >= 0)
              { if (Parse[j].kind > OPT_ARG)
                  spec_error("Synonym %s is not an option",Parse[j].name);
                if (Parse[j].synonym >= 0)
                  spec_error("A synonym refers to a synonym",NULL);
                Parse[i].kind    = Parse[j].kind;
                Parse[i].iterate = Parse[j].iterate;
                Parse[i].synonym = j;
                Parse[i].type    = Parse[j].type;
              }
            else
              spec_error("'%s' is not a valid type\n",spec[i].type);
            if (Parse[i].kind != OPTION && Parse[i].extension >= 0)
              spec_error("%s is a prefix of an earlier option but not a pure option",Parse[i].name);
          }
        else
          spec_error("'%s' is not a valid type\n",spec[i].type);

        while (isalpha(*s))
          s += 1;

        if (*s != '\0')
          spec_error("Cannot interpret suffix '%s' of argument type",s);
      }

      Parse[i].has_default = 0;             //  Parse default value (options only)
      if (Parse[i].kind <= OPT_ARG)
        { char *bptr /*, *eptr */;

          bptr = spec[i].defval;
          if (bptr == NULL)
            { if (Parse[i].kind == OPTION)
                spec_error("Pure option must have a default setting",NULL);
            }
          else
            { if (Parse[i].synonym >= 0 && Parse[i].kind != OPTION)
                spec_error("Parameterized option synonym %s cannot have a default value",
                           Parse[i].name);
              Parse[i].has_default = 1;
              Parse[i].defval = *parse_value(bptr,Parse[i].type,1);
              if (Parse[i].kind == OPTION)
                { int val = Parse[i].defval.intval;
                  if (val != 0 && val != 1)
                    spec_error("Pure option default is not 0 or 1 (%s)",bptr);
                }
            }
        }
    }

#ifdef DEBUG
  print_arg_table(0);
#endif

  // Process Arguments

  iterator_card = - num_reqd_arg;  //  Determine size of iterator / check count of required args
  for (i = 1; i < argc; i++)
    if (argv[i][0] != '-')
      iterator_card += 1;
    else
      { int j = find_option(argv[i]+1,Arg_Len);
        if (j >= 0 && Parse[j].kind == OPT_ARG)
          i += 1;
      }

  if (iterator_card < 0 || iterator_card == 0 && has_flex_arg == ITERATOR_1
                        || iterator_card >  0 && ! has_flex_arg)
    { Print_Argument_Usage(stderr);
      return;
    }

  for (i = 0; i < Arg_Len; i++)   //  Initialize set status
    Parse[i].set = 0;

  { int  nextr, icard;
    char flag_string[2];

    icard = 0;
    nextr = 0;                      //  For each command line argument do
    flag_string[1] = '\0';
    for (i = 1; i < argc; i++)
      { if (argv[i][0] == '-')          //   The argument is an option
          { int        j, k, m;
  
            j = find_option(argv[i]+1,Arg_Len);   //   Single argument
            if (j >= 0)
              { m = j;
                if (Parse[j].synonym >= 0)
                  m = Parse[j].synonym; 
                if (Parse[m].set && Parse[m].iterate)
                  { Parse[m].set += 1;
                    Parse[Arg_Len] = Parse[m];
                    m = Arg_Len++; 
                  }
                else
                  Parse[m].set = 1;
                if (Parse[j].kind == OPT_ARG)
                  { if (++i >= argc)
                      interp_error("Follow on argument to option %s is missing",argv[i-1]);
                    Parse[m].value = *parse_value(argv[i],Parse[j].type,0);
                  }
                else if (Parse[j].kind == OPT_CMPCT)
                  Parse[m].value = *parse_value(argv[i]+strlen(Parse[j].name)+1,Parse[j].type,0);
                else
                  Parse[m].value.intval = 1 - Parse[j].defval.intval;
              }
            else
              { for (k = 1; argv[i][k] != '\0'; k++)    //  Try interpreting as a string of flags
                  { *flag_string = argv[i][k];
                    j = find_option(flag_string,Arg_Len);
                    if (j < 0)
                      break;
                    m = j;
                    if (Parse[j].synonym >= 0)
                      m = Parse[j].synonym;
                    if (Parse[m].kind != OPTION)
                      break;
                    Parse[m].value.intval = 1 - Parse[j].defval.intval;
                  }
                if (argv[i][k] != 0)
                  interp_error("Illegal option '%s'",argv[i]+1);
              }
          }
  
        else                                    //  Required arguments
          { int        m;

            while (nextr < Arg_Len)
              { if (Parse[nextr].kind == REQUIRED ||
                         Parse[nextr].kind >= ITERATOR_0 && icard < iterator_card)
                  break;
                nextr += 1;
              }
            if (Parse[nextr].set)
              { Parse[nextr].set += 1;
                Parse[Arg_Len] = Parse[nextr];
                m = Arg_Len++;
              }
            else
              { Parse[nextr].set = 1;
                m = nextr;
              }
            Parse[m].value = *parse_value(argv[i],Parse[nextr].type,0);
            if (Parse[nextr].kind >= ITERATOR_0)
              icard += 1;
            else
              nextr += 1;
          }
      }
  }

  for (i = 0; i < Arg_Len; i++)   //  If not set & has a default, then set it
    if (Parse[i].set == 0 && Parse[i].has_default && Parse[i].synonym < 0)
      { Parse[i].set   = 1;
        Parse[i].value = Parse[i].defval;
      }

#ifdef DEBUG
  print_arg_table(1);
#endif
}

int Argument_Cardinality(char *name)
{ int j = lookup_arg(name,Arg_Len,1);
  if (j < 0)
    spec_error("Cannot find argument '%s'",name);
  if (Parse[j].synonym >= 0)
    j = Parse[j].synonym;
  return (Parse[j].set);
}

#define GET_BODY						\
  va_list ap;							\
  int     j, n;							\
								\
  j = lookup_arg(name,Arg_Len,1);				\
  if (j < 0)							\
    spec_error("Cannot find argument '%s'",name);		\
								\
  va_start(ap,name);						\
  if (Parse[j].iterate)						\
    n = va_arg(ap,int);						\
  else								\
    n = 1;							\
  va_end(ap);							\
								\
  if (Parse[j].synonym >= 0)					\
    name = Parse[Parse[j].synonym].name;			\
  j = lookup_arg(name,Arg_Len,n);				\
  if (j < 0)							\
    spec_error("Exceeded argument cardinality of %s",name); 

int Get_Int_Arg(char *name, ...)
{ GET_BODY
  return (Parse[j].value.intval);
}

double Get_Double_Arg(char *name, ...)
{ GET_BODY
  return (Parse[j].value.dblval);
}

char *Get_String_Arg(char *name, ...)
{ GET_BODY
  return (Parse[j].value.strval);
}

char *Program_Name()
{ return (Program); }

//  Print a usage message on stream file

static inline int print_synonym(FILE *file, int i)
{ int k;

  if ((k = Parse[i].synonym) >= 0)
    { char *name = Parse[k].name;
      fprintf(file,"{%s}",name);
      return (2 + strlen(name));
    }
  return (0);
}

void Print_Argument_Usage(FILE *file)
{ int i, j, k;
  int first;
  int idnt, clen;

  clen = idnt = fprintf(file,"Usage: %s",Program);

  first = 1;
  for (i = 0; i < Arg_Len; i++)
    { if (Parse[i].kind == OPTION && strlen(Parse[i].name) == 1 && Parse[i].extension < 0)
        { if (first)
            { first = 0;
              clen += fprintf(file," [-");
            }
          clen += fprintf(file,"%s",Parse[i].name);
          clen += print_synonym(file,i);
        }
    }
  if (!first)
    clen += fprintf(file,"]");

  for (i = 0; i < Arg_Len; i++)
    if (Parse[i].kind == OPTION && strlen(Parse[i].name) > 1 && Parse[i].extension < 0)
      { if (clen > RETURN_LENGTH)
          clen = fprintf(file,"\n%*s",idnt,"") - 1;
        clen += fprintf(file," [-%s",Parse[i].name);
        clen += print_synonym(file,i);
        clen += fprintf(file,"]");
      }
    else if ((Parse[i].kind == OPT_ARG || Parse[i].kind == OPT_CMPCT) && Parse[i].extension < 0)
      { if (clen > RETURN_LENGTH)
          clen = fprintf(file,"\n%*s",idnt,"") - 1;
        clen += fprintf(file," [-");
        j = i;
        while ((k = Parse[j].prefix) >= 0)
          j = k;
        clen += fprintf(file,"%s",Parse[j].name);
        clen += print_synonym(file,j);
        while ((k = Parse[j].extension) >= 0)
          { clen += fprintf(file,"[%.*s",(int)(strlen(Parse[k].name)-strlen(Parse[j].name)),
                                         Parse[k].name+strlen(Parse[j].name));
            clen += print_synonym(file,k);
            j = k;
          }

        if (Parse[i].kind == OPT_ARG)
          clen += fprintf(file," ");
        clen += fprintf(file,"<%s>",Type_Strings[Parse[i].type]);
        if (Parse[i].iterate)
          clen += fprintf(file," ...");
        for (j = i; j >= 0; j = Parse[j].prefix)
          clen += fprintf(file,"]");
      }

  for (i = 0; i < Arg_Len; i++)
    { if (clen > RETURN_LENGTH)
        clen = fprintf(file,"\n%*s",idnt,"") - 1;
      if (Parse[i].kind == REQUIRED)
        clen += fprintf(file," <%s:%s>",Parse[i].name,Type_Strings[Parse[i].type]);
      else if (Parse[i].kind == ITERATOR_0)
        clen += fprintf(file," [<%s:%s> ...]",Parse[i].name,Type_Strings[Parse[i].type]);
      else if (Parse[i].kind == ITERATOR_1)
        clen += fprintf(file," <%s:%s> ...",Parse[i].name,Type_Strings[Parse[i].type]);
    }
   
  fprintf(file,"\n");
}
