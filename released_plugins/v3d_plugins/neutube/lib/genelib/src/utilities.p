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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "utilities.h"

#define OPTION_CHAR  '-'
#define FLAG_CHAR    '!'
#define ESCAPE_CHAR  '\''
#define VALUE_CHAR   '@'

#undef  DEBUG
#undef  DEBUG_TREE
#undef  DEBUG_FA
#undef  DEBUG_MATCH
#undef  DEBUG_SEARCH
#undef  DEBUG_RESULT
#undef  DEBUG_LOOKUP

/*****************************************************************************************/
/*   GUARDED SYSTEM CALLS                                                                */
/*****************************************************************************************/

void *Guarded_Malloc(size_t size, const char *routine)
{ void *p;

  p = malloc(size);
  if (p == NULL)
    { fprintf(stderr,"\nError in %s:\n",routine);
      fprintf(stderr,"   Out of memory\n");
      exit (1);
    }
  return (p);
}

void *Guarded_Realloc(void *p, size_t size, char *routine)
{ p = realloc(p,size);
  if (p == NULL)
    { fprintf(stderr,"\nError in %s:\n",routine);
      fprintf(stderr,"   Out of memory\n");
      exit (1);
    }
  return (p);
}

char *Guarded_Strdup(char *p, char *routine)
{ p = strdup(p);
  if (p == NULL)
    { fprintf(stderr,"\nError in %s:\n",routine);
      fprintf(stderr,"   Out of memory\n");
      exit (1);
    }
  return (p);
}

FILE *Guarded_Fopen(const char *file_name, const char *options, const char *routine)
{ FILE *f;

  f = fopen(file_name,options);
  if (f == NULL)
    { fprintf(stderr,"\nError in %s:\n",routine);
      fprintf(stderr,"   Cannot open %s\n",file_name);
      exit (1);
    }
  return (f);
}


/*****************************************************************************************/
/*   COMMAND-LINE PROCESSOR LONG-TERM STATIC VALUES                                      */
/*****************************************************************************************/

static char  *Program;        //  name of program
static char **Master_Spec;    //  specification copy (persistent)

static int    ArgC;           //  global references to argc, argv of command-line
static char **ArgV;      

#define INT_TYPE    0         //  value types
#define DOUBLE_TYPE 1
#define STRING_TYPE 2

static char *Type_Strings[] = { "int", "double", "string" };  //  command spec. base types

typedef struct                         // a base type value
  { int type;
    union
      { int    intval;
        double dblval;
        char  *strval;
      } value;
  } Value;

typedef unsigned int Alpha_Set[4];    //  a bit-vector encoding of a set of chars

#ifdef DEBUG

static void print_value(FILE *file, Value *value)
{ switch (value->type)
  { case INT_TYPE:
      fprintf(file,"%d",value->value.intval);
      break;
    case DOUBLE_TYPE:
      fprintf(file,"%g",value->value.dblval);
      break;
    case STRING_TYPE:
      fprintf(file,"%s",value->value.strval);
      break;
  }
}

#endif


/*****************************************************************************************/
/*   COMMAND-LINE PARSE TREE DEFS & UTLITIES                                             */
/*****************************************************************************************/

  // Node labels

#define TYPE    1   //  un-named type associated with an option -- <type>
#define NAME    2   //  named required argument -- <name:type>
#define TEXT    3   //  text segment of an option -- text
#define LIST    4   //  1st text segment of a flag list -- !text
#define DASH    5   //  1st text segment of an option -- -text
#define WHITE   6   //  contiguous block of white space

#define SYN_L   7   //  {, in automata only
#define SYN_R   8   //  }, in automata only

#define SYN     7   //  interior node labels
#define OPT     8
#define PLUS    9
#define CAT    10
#define OR     11

  // Interior nodes of the parse tree for the specification

typedef struct _node
  { int           label;  // Node type     (>= SYN)
    short         empty;  // Can the subtree match the emtpy string?
    short         loops;  // Does the subtree contain a loop?
    struct _node *left;   // Left subtree
    struct _node *right;  // Right subtree (>= CAT only)
  } Node;

  // Leaf nodes of the parse tree, corresponding to "atoms" in the documentation

typedef struct
  { int           label;      // Node type (< SYN)
    short         empty;      // Can the subtree match the emtpy string?
    short         loops;      // Does the subtree contain a loop?
    char         *name;       // Name of argument (NAME), text of constant (TEXT,LIST,DASH),
                              //   and token position (name-1 pts. at '<' for NAME)
    short         nlen;       // Length of name or text (NAME,LIST,TEXT,DASH)
    short         slen;       // Length of name or text not including escape chars
    short         line;       // Spec line containing token
    short         has_def;    // Does the TYPE have a default?
    Value         def_val;    // Type (NAME,TYPE), and default value if there is one (TYPE)
  } Atom;

static Node *new_node(int label, Node *left, Node *right)
{ Node *p;

  p = (Node *) Guarded_Malloc(sizeof(Node),"Process_Arguments");
  p->label = label;
  p->left  = left;
  p->right = right;
  switch (p->label)
  { case CAT:
      p->empty = left->empty && right->empty;
      p->loops = left->loops || right->loops;
      break;
    case OR:
      p->empty = left->empty || right->empty;
      p->loops = left->loops || right->loops;
      break;
    case OPT:
      p->empty = 1;
      p->loops = left->loops;
      break;
    case PLUS:
      p->empty = left->empty;
      p->loops = 1;
      break;
    case SYN:
      p->empty = left->empty;
      p->loops = left->loops;
      break;
  }
  return (p);
}

static Node *new_atom(int label, char *name, int len, Value *type_def, int line, int hasd)
{ Atom *p;

  p = (Atom *) Guarded_Malloc(sizeof(Atom),"Process_Arguments");
  p->label   = label;
  p->empty   = (label == WHITE);
  p->loops   = 0;
  p->name    = name;
  p->nlen    = len;
  p->line    = line;
  p->has_def = hasd;
  if (type_def != NULL)
    p->def_val = *type_def;
  if (label == TEXT || label == LIST || label == DASH)
    { int i, j;

      for (i = j = 0; i < len; i++, j++)
        if (name[i] == ESCAPE_CHAR)
          i += 1;
      p->slen = j;
    }
  else
    p->slen = 0;
  return ((Node *) p);
}


/*****************************************************************************************/
/*   PARSE TREE PRINTER                                                                  */
/*****************************************************************************************/

#ifdef DEBUG

static char *Label_String[] = { "EMTY", "TYPE", "NAME", "TEXT", "LIST", "DASH",
                                "WHTE", "_SYN", "_OPT", "PLUS", "_CAT", "__OR" };

static char *Type_String[] = { "INT", "DBL", "STR" };

static void print_type(FILE *file, Atom *q)
{ fprintf(file,"<%s",Type_String[q->def_val.type]);
  if (q->has_def)
    { fprintf(file,"(");
      print_value(file,&(q->def_val));
      fprintf(file,")");
    }
  fprintf(file,">");
}

#endif

#ifdef DEBUG_TREE

static char Print_Buffer[1000];

static void _print_tree(FILE *file, int ind, int sec, Node *p)
{ Atom *q;

  fprintf(file,"%s_%s %d %d",Print_Buffer,Label_String[p->label],p->empty,p->loops);
  q = (Atom *) p;
  if (p->label == TYPE)
    print_type(file,q);
  else if (p->label == NAME)
    fprintf(file,"<%.*s:%s>",q->nlen,q->name,Type_String[q->def_val.type]);
  else if (p->label == LIST)
    fprintf(file,"%c'%.*s'",FLAG_CHAR,q->nlen-1,q->name+1);
  else if (p->label == TEXT || p->label == DASH)
    fprintf(file,"'%.*s'",q->nlen,q->name);
  fprintf(file,"\n");
  if (sec) Print_Buffer[ind] = ' ';
  strcat(Print_Buffer," |");
  if (p->label >= SYN)
    { if (p->right != NULL)
        _print_tree(file,ind+2,0,p->right);
      if (p->left != NULL)
        _print_tree(file,ind+2,1,p->left);
    }
  Print_Buffer[ind+1] = '\0';
}

static void print_tree(FILE *file, Node *p)
{ fprintf(file,"\nSPECIFICATION PARSE TREE:\n");
  fprintf(file,"\n");
  strcpy(Print_Buffer,"|");
  _print_tree(file,0,1,p);
  fprintf(file,"\n");
  fflush(file);
}

#endif


/*****************************************************************************************/
/*   COMMAND-LINE SPECIFICATION PARSER                                                   */
/*****************************************************************************************/

  /* Global communication between parser routines */

static char *Scan;        /* Current scan pointer                       */
static char *Specbeg;     /* Pointer to first char of current spec line */
static int   Specline;    /* Current spec line #                        */
static int   In_Curly;    /* Currently within curly braces ?            */

  /* Syntax_error and semantic_error print error and location and terminate */

static char *C2_ERROR = "Can call Process_Arguments only once!";

static char *RP_ERROR = "Missing right paren, ), in command-line specification";
static char *RC_ERROR = "Missing right curly brace, }, in command-line specification";
static char *RB_ERROR = "Missing right bracket, ], in command-line specification";
static char *RA_ERROR = "Missing right angle bracket, >, in command-line specification";
static char *SP_ERROR = "White space inside argument angle brackets <>";
static char *UT_ERROR = "Invalid type in command-line specification";
static char *PT_ERROR = "Prematurely terminated command-line specification";
static char *CC_ERROR = "Cannot have {} within another {}";
static char *UO_ERROR = "Unterminated <> token";
static char *UD_ERROR = "Unterminated default in ()'s";
static char *UB_ERROR = "End of line immediately after escape symbol";
static char *EI_ERROR = "Expecting integer default value";
static char *ED_ERROR = "Expecting floating point default value";
static char *EN_ERROR = "Cannot have ... within the scope of another ...";

static char *TO_ERROR = "Type is not part of an option";
static char *TT_ERROR = "Type must be separated from a preceding type by white space or text";
static char *TL_ERROR = "Type cannot be part of an option flag list";
static char *NR_ERROR = "Name should have white space or nothing to its right";
static char *NL_ERROR = "Name should have white space or nothing to its left";
static char *TX_ERROR = "Text follows a type chain";
static char *OP_ERROR = "Start of option flag list should have white space or nothing to its left";
static char *BO_ERROR = "Start of option should begin with a -";
static char *CW_ERROR = "Cannot have white space within {}";
static char *CT_ERROR = "Cannot have a type within {}";
static char *CP_ERROR = "Curly braces can only enclose option text";
static char *PL_ERROR = "Cannot have only part of a unit within the scope of a ...";
static char *OL_ERROR = "Cannot have a pure option within the scope of a ...";

static char *DP_ERROR = "Unit occurs twice on an expansion of the specification";
static char *DF_ERROR = "Two instances of the same unit have different default values";


static void syntax_error(char *msg)
{ fprintf(stderr,"\nError in Process_Arguments:\n  %s\n",msg);
  fprintf(stderr,"\n        %s",Master_Spec[Specline]);  //  Has a terminal '\n'!
  fprintf(stderr,"        %*s^\n",(int)(Scan-Specbeg),"");
  exit (1);
}

static int atom_location(Atom *a)
{ if (a->label == NAME)
    return ((a->name - Master_Spec[a->line]) - 1);
  else
    return (a->name - Master_Spec[a->line]);
}

static void semantic_error(char *msg, Atom *s)
{ fprintf(stderr,"\nError in Process_Arguments:\n  %s\n",msg);
  fprintf(stderr,"\n        %s",Master_Spec[s->line]);  //  Has a terminal '\n'!
  fprintf(stderr,"        %*s^\n",atom_location(s),"");
  exit (1);
}

static void dual_error(char *msg, Atom *a, Atom *b)
{ fprintf(stderr,"\nError in Process_Arguments:\n  %s\n",msg);
  fprintf(stderr,"\n        %s",Master_Spec[a->line]);
  fprintf(stderr,"        %*s^",atom_location(a),"");
  if (a->line == b->line)
    fprintf(stderr,"%*s^\n",(atom_location(b)-atom_location(a))-1,"");
  else
    { fprintf(stderr,"\n\n        %s",Master_Spec[b->line]);
      fprintf(stderr,"        %*s^\n",atom_location(b),"");
    }
  exit (1);
}

// If at the end of current spec. line, setup to scan the next

static void check_eol()
{ while (*Scan == '\0' && Master_Spec[Specline+1] != NULL)
    { Specbeg = Master_Spec[++Specline];
      Scan    = Specbeg;
    }
}

static int parse_type()   //  match a type string and return the code for the type
{ char *p;

  if (strncmp("int",Scan,3) == 0)
    { Scan += 3;
      return (INT_TYPE);
    }
  if (strncmp("double",Scan,6) == 0)
    { Scan += 6;
      return (DOUBLE_TYPE);
    }
  p = Scan;
  while (*Scan != '\0' && (isalnum(*Scan) || *Scan == '_'))
    Scan += 1;
  if (*Scan == '\0' || p == Scan)
    { Scan = p;
      return (-1);
    }
  return (STRING_TYPE);
}

static Node *or();

//  <fact> <- '(' <or> ')' | '{' <or> '}' | '[' <or> ']' | '<'<name>':'<type>'>'
//          | '<'<type>['('<value>')']'>' | <white> | <text> | <dash> | <flag>

static Node *fact()
{ Node *p;

  switch (*Scan)

  { case '(':
      Scan += 1;
      check_eol();
      p = or();
      if (*Scan != ')')
        syntax_error(RP_ERROR);
      Scan += 1;
      break;

    case '{':
      if (In_Curly)
        syntax_error(CC_ERROR);   //  cannot nest {}'s
      In_Curly = 1;
      Scan += 1;
      check_eol();
      p = or();
      if (*Scan != '}')
        syntax_error(RC_ERROR);
      Scan += 1;
      p = new_node(SYN,p,NULL);
      In_Curly = 0;
      break;

    case '[':
      Scan += 1;
      check_eol();
      p = or();
      if (*Scan != ']')
        syntax_error(RB_ERROR);
      Scan += 1;
      p = new_node(OPT,p,NULL);
      break;

    case '<':
      { char *name;
        int   len, hasd;
        Value type_def;

        Scan += 1;
        name  = Scan;
        while (*Scan != ':' && *Scan != '>' && *Scan != '(')
          { if (isspace(*Scan))
              syntax_error(SP_ERROR);
            if (*Scan == '\0')
              { Scan = name-1;
                syntax_error(UO_ERROR);
              }
            Scan += 1;
          }
        if (*Scan == ':')
          { len = Scan - name;
            Scan += 1;
          }
        else
          { len = -1;
            Scan = name;
          }

        type_def.type = parse_type();   // Get type
        if (type_def.type < 0)
          syntax_error(UT_ERROR);

        if (len < 0 && *Scan == '(')   //  Get default value (if any)
          { char *valptr, *e;

            Scan  += 1;
            hasd   = 1;
            valptr = Scan;
            while (*Scan != ')')
              { if (*Scan == ESCAPE_CHAR)
                  Scan += 1;
                if (*Scan == '\0')
                  { Scan = valptr-1;
                    syntax_error(UD_ERROR);
                  }
                Scan += 1;
              }
            *Scan = '\0';
            if (type_def.type == INT_TYPE)
              { type_def.value.intval = strtol(valptr,&e,10);
                if (*valptr == '\0' || *e != '\0')
                  { Scan = valptr;
                    syntax_error(EI_ERROR);
                  }
              }
            else if (type_def.type == DOUBLE_TYPE)
              { type_def.value.dblval = strtod(valptr,&e);
                if (*valptr == '\0' || *e != '\0')
                  { Scan = valptr;
                    syntax_error(ED_ERROR);
                  }
              }
            else
              type_def.value.strval = Guarded_Strdup(valptr,"Process_Arguments");
            *Scan++ = ')';
          }
        else
          hasd = 0;
        if (*Scan != '>')
          syntax_error(RA_ERROR);
        Scan += 1;
        if (len < 0)
          p = new_atom(TYPE,name-1,0,&type_def,Specline,hasd);
        else
          p = new_atom(NAME,name,len,&type_def,Specline,0);
      }
      break;

    case '\0':
      syntax_error(PT_ERROR);

    default:
      if (isspace(*Scan))                              // White space
        { p = new_atom(WHITE,Scan,0,NULL,Specline,0);
          while (isspace(*Scan))
            Scan += 1;
        }
      else                                             // One of the text types (LIST,DASH,TEXT)
        { int   kind;
          char *string;

          if (*Scan == FLAG_CHAR)
            { kind = LIST; *Scan = OPTION_CHAR; }  //  Change ! to - for subsequent print purposes
          else if (*Scan == OPTION_CHAR)
            kind = DASH;
          else
            kind = TEXT;
          string = Scan;
          while (!isspace(*Scan) && *Scan != ')' && *Scan != '(' && *Scan != '|' &&
                                    *Scan != ']' && *Scan != '[' && *Scan != '<' &&
                                    *Scan != '}' && *Scan != '{' && *Scan != '\0' &&
                                   (*Scan != '.' || Scan[1] != '.' || Scan[2] != '.'))
            { if (*Scan == ESCAPE_CHAR)
                Scan += 1;
              if (*Scan == '\0')
                syntax_error(UB_ERROR);
              Scan += 1;
            }
          p = new_atom(kind,string,Scan-string,NULL,Specline,0);
        }
      break;
  }

  check_eol();
  return (p);
}

//  <rep> <- <fact> (<white> '...')*

static Node *rep()
{ Node *p;

  p = fact();
  while (1)
    { char *tp = Scan;
      while (isspace(*Scan)) 
        Scan += 1;
      if (Scan[0] == '.' && Scan[1] == '.' && Scan[2] == '.')
        { if (p->loops)
            syntax_error(EN_ERROR);   //  loop cannot contain a loop
          Scan += 3;
          p = new_node(PLUS,p,NULL);
        }
      else
        { Scan = tp;
          break;
        }
    }
  return (p);
}

//  <cat> <- <rep> +

static Node *cat()
{ Node *p;

  p = rep();
  while (*Scan != ')' && *Scan != ']' && *Scan != '}' && *Scan != '|' && *Scan != '\0')
    p = new_node(CAT,p,rep());
  return (p);
}

//  <or> <- <cat> ( '|' <cat> ) *

static Node *or()
{ Node *p;

  p = cat();
  while (*Scan == '|')
    { Scan += 1;
      check_eol();
      p = new_node(OR,p,cat());
    }
  return (p);
}

// Transform the tree so that the interior of any repetition cannot match the empty string.
//   Don't worry about freeing nodes as we're leaving a global structure in place regardless.

static Node *remove_empty_loops(Node *p, int elim)
{ if (p == NULL)
    return (NULL);
  if (p->label < SYN)
    { if (p->empty && elim)
        return (NULL);
      else
        return (p);
    }
  if (elim)
    { p->empty = 0;
      if (p->label == OPT)
        p = remove_empty_loops(p->left,p->left->empty);
      else
        { p->left  = remove_empty_loops(p->left,1);
          p->right = remove_empty_loops(p->right,1);
          if (p->label == CAT)
            p->label = OR;
        }
    }
  else
    if (p->label == PLUS && p->empty)
      { p->empty = 0;
        p->left  = remove_empty_loops(p->left,1);
        p = new_node(OPT,p,NULL);
      }
    else
      { p->left  = remove_empty_loops(p->left,0);
        p->right = remove_empty_loops(p->right,0);
      }

  if (p->left == NULL)
    if (p->right == NULL)
      return (NULL);
    else
      return (p->right);
  else
    if (p->label == OR && p->right == NULL)
      return (p->left);
    else
      return (p);
}


/*****************************************************************************************/
/*   BUILD FINITE AUTOMATON EQUIVALENT TO COMMAND-LINE PARSE                             */
/*****************************************************************************************/

  // Automaton State

#define LOOP_START  0x01  //  Target of backedge?
#define LOOP_FINISH 0x02  //  2nd edge is a backedge?

#define START_FLAG  0x10  //  First state of a unit
#define MORE0_FLAG  0x20  //  1st successor edge continues a unit
#define MORE1_FLAG  0x40  //  2nd successor edge continues a unit
#define FINISH_FLAG 0x80  //  Last state of a unit

typedef struct _state
  { struct _state *succ[2];    // Automaton transitions
    struct _state *pred[2];
    struct _state *loop;       // Entry state of loop containing this state (if its in a loop)
    int            mark;       // Mark field for traversals and checking
    int            flags;      // Some union of the 6 defined flags above
    short          minlen;     // Min # of unit starts on a path to the final state
    short          maxlen;     // Max # of unit starts on a path to the start state
    Atom          *atom;       // Atom for the state
    void          *ulist;      // Linked list of units that start in this state
#ifdef DEBUG
    int            number;     // Preorder number of diagnostic output
#endif
  } State;

  // Automaton (that corresponds to the specification)

typedef struct
  { State *start;     // start state of automaton
    State *final;     // final state of automaton
    int    longest;   // longest sequence in automaton
  } Automaton;

static State *new_state(Atom *atom, State *loop_start)
{ State *s;

  s = (State *) Guarded_Malloc(sizeof(State),"Process_Arguments");
  s->succ[0] = s->succ[1] = NULL;
  s->pred[0] = s->pred[1] = NULL;
  s->loop    = loop_start;
  s->mark    = 0;
  s->flags   = 0;
  s->minlen  = 0;
  s->maxlen  = 0;
  s->atom    = atom;
  s->ulist   = NULL;
#ifdef DEBUG
  s->number  = 0;
#endif
  return ((State *) s);
}

static Automaton *build_enfa(Node *p, State *loop_start)
{ static Automaton enfa;

  Automaton  mach1, mach2;

  switch (p->label)
  { case OR:

      mach1 = *build_enfa(p->left,loop_start);
      mach2 = *build_enfa(p->right,loop_start);

      enfa.start = new_state(NULL,loop_start);
      enfa.final = new_state(NULL,loop_start);
      if (mach1.longest > mach2.longest)
        enfa.longest = mach1.longest;
      else
        enfa.longest = mach2.longest;

      enfa.start->succ[0] = mach1.start;
      enfa.start->succ[1] = mach2.start;
      mach1.final->succ[0] = enfa.final;
      mach2.final->succ[0] = enfa.final;

      mach1.start->pred[0] = enfa.start;
      mach2.start->pred[1] = enfa.start;
      enfa.final->pred[0] = mach1.final;
      enfa.final->pred[1] = mach2.final;

      return (&enfa);

    case CAT:
      mach1 = *build_enfa(p->left,loop_start);
      mach2 = *build_enfa(p->right,loop_start);

      enfa.start   = mach1.start;
      enfa.final   = mach2.final;
      enfa.longest = mach1.longest + mach2.longest;

      mach1.final->succ[0] = mach2.start;
      mach2.start->pred[0] = mach1.final;
      return (&enfa);

    case OPT:
    case SYN:
    case PLUS:
      mach2.start  = new_state(NULL,loop_start);
      if (p->label == PLUS)
        mach1 = *build_enfa(p->left,mach2.start);
      else
        mach1 = *build_enfa(p->left,loop_start);

      enfa.start   = mach2.start;
      enfa.final   = new_state(NULL,loop_start);
      enfa.longest = mach1.longest;

      enfa.start->succ[0]  = mach1.start;
      mach1.start->pred[0] = enfa.start;

      mach1.final->succ[0] = enfa.final;
      enfa.final->pred[0]  = mach1.final;

      if (p->label == PLUS)
        { mach1.final->succ[1] = mach1.start;
          mach1.start->pred[1] = mach1.final;
          mach1.start->flags |= LOOP_START;
          mach1.final->flags |= LOOP_FINISH;
        }
      else if (p->label == OPT)
        { enfa.start->succ[1] = enfa.final;
          enfa.final->pred[1] = enfa.start;
        }
      else
        { enfa.start->atom = (Atom *) new_atom(SYN_L,NULL,0,NULL,0,0);
          enfa.final->atom = (Atom *) new_atom(SYN_R,NULL,0,NULL,0,0);
          enfa.longest    += 2;
        }

      return (&enfa);

    default:
      enfa.start = enfa.final = new_state((Atom *) p,loop_start);
      enfa.longest = 1;

      return (&enfa);
  }
}

static void reset_marks(State *s)   // Resets mark bits after a traversal of automaton
{ s->mark = 0;
  if (s->succ[0] != NULL && s->succ[0]->mark != 0)
    reset_marks(s->succ[0]);
  if (s->succ[1] != NULL && s->succ[1]->mark != 0)
    reset_marks(s->succ[1]);
}

#ifdef DEBUG

static int preorder_number(State *s, int *n)  // Preorder number the states of the automaton
{ s->number = ++(*n);
  if (s->succ[0] != NULL && s->succ[0]->number == 0)
    preorder_number(s->succ[0],n);
  if (s->succ[1] != NULL && s->succ[1]->number == 0)
    preorder_number(s->succ[1],n);
}

#endif

static Automaton *build_automaton(Node *root)
{ static Automaton enfa;

  Automaton  mach;

  mach = *build_enfa(root,NULL);
  enfa.start = new_state(NULL,NULL);
  enfa.final = mach.final;

  enfa.start->succ[0] = mach.start;
  mach.start->pred[0] = enfa.start;
  enfa.longest        = mach.longest;

#ifdef DEBUG
  { int pnumber;

    pnumber = 0;
    preorder_number(enfa.start,&pnumber);
  }
#endif

  return (&enfa);
}


/*****************************************************************************************/
/*   PRINT A FINITE AUTOMATON                                                            */
/*****************************************************************************************/

#ifdef DEBUG_FA

static void print_instance(FILE *, void *, int, int);

static char *State_String[] = { "EMTY", "TYPE", "NAME", "TEXT",
                                "LIST", "DASH", "WHTE", "{", "}" };

static void print_state(FILE *file, State *s, State *f)
{ fprintf(file,"s%d",s->number);
  if (s->number == 1)
    { fprintf(file," -");
      if (s == f)
        fprintf(file,"+");
    }
  else if (s == f)
    fprintf(file," +");

  { Atom *p = s->atom;

    if (p == NULL)
      fprintf(file," EMTY");
    else
      { fprintf(file," %s",State_String[p->label]);
        if (p->label == TYPE)
          print_type(file,p);
        else if (p->label == NAME)
          fprintf(file," <%.*s:%s>",p->nlen,p->name,Type_String[p->def_val.type]);
        else if (p->label == TEXT || p->label == DASH)
          fprintf(file," '%.*s'",p->nlen,p->name);
        else if (p->label == LIST)
          fprintf(file," !'%.*s'",p->nlen-1,p->name+1);
      }
    if ((s->flags & START_FLAG) != 0)
      fprintf(file," >");
    if ((s->flags & MORE0_FLAG) != 0)
      fprintf(file," 0");
    if ((s->flags & MORE1_FLAG) != 0)
      fprintf(file," 1");
    if ((s->flags & FINISH_FLAG) != 0)
      fprintf(file," <");
    if ((s->flags & (LOOP_START|LOOP_FINISH)) != 0)
      fprintf(file," *");
    if (s->loop != NULL)
      fprintf(file," [%d]",s->loop->number);
    fprintf(file," <%d,%d>\n",s->minlen,s->maxlen);
  }

  print_instance(file,s->ulist,1,1);

  if (s->succ[0] != NULL)
    fprintf(file,"  -> s%d\n",s->succ[0]->number);
  if (s->succ[1] != NULL)
    fprintf(file,"  -> s%d\n",s->succ[1]->number);

  s->mark = 1;
  if (s->succ[0] != NULL && s->succ[0]->mark == 0)
    print_state(file,s->succ[0],f);
  if (s->succ[1] != NULL && s->succ[1]->mark == 0)
    print_state(file,s->succ[1],f);
}

static void print_automaton(FILE *file, Automaton *a)
{ State *start = a->start;

  print_state(file,start,a->final);
  reset_marks(start);
  fflush(file);
}

#endif


/*****************************************************************************************/
/*   SEMANTIC CHECK AND UNIT MARKING WITHIN FINITE AUTOMATON                             */
/*****************************************************************************************/

/* Check that there are no illegal paths through the automaton.  Each path should spell
   a sequence of required args, flag lists, and option chains separated by white space.
   Each of these segments we call a "unit" and the three types of units are defined by:

      <unit> <-- <required arg> | <flag list> | <option chain>
        <required arg> <-- NAME
        <flag list>    <-- LIST <text> *
        <option chain> <-- DASH <text>* (TYPE <text>+)* TYPE? (WHITE TYPE)*
          <text> <-- DASH | LIST | TEXT

   A path is legal if it matches: WHITE* ( <unit> WHITE+ ) <unit> WHITE*.  We also want
   to ensure that {} braces only surround text in an option.  This translates to checking
   that the string spelled on each path in the automaton is accepted by the following
   eight state finite automaton with transtions as follows:

      1 - NAME  -> 5   2 - TYPE   -> 7   3 - TYPE  -> 4    4 - WHITE -> 3   6 - <text> -> 6
          DASH  -> 2       <text> -> 2       NAME  -> 5                         WHITE  -> 1
          LIST  -> 6       WHITE  -> 3       DASH  -> 2    5 - WHITE -> 1
          WHITE -> 1       SYN_LR -> 2       LIST  -> 6                     7 - <text> -> 2
          SYN_L -> 8                         WHITE -> 3                     7 - WHITE  -> 3
                                             SYN_L -> 8    8 - DASH  -> 2

    In addition we figure out where units start and finish and what edges are on paths
    spelling units marking said in the "flags" field of each state.

    This code is tricky and proceeds in 3 scans of the automaton -- compute_unit_lookaheads,
    compute_automaton_units, and compute_unit_finishes -- that set bits in the 'mark' field
    of each state in order to keep track of a variety of things as described for each
    routine below.
*/

#define SEE_MARK    0x100  // mark bit for compute_unit_lookaheads traversal
#define SEE_TEXT    0x200  // exists a path from node spelling 'epsilon* <text>'
#define SEE_TYPE    0x400  // exists a path from node spelling '(epsilon|WHITE)* TYPE'

#define CLEAR_MARK 0x1000  // mark bit for compute_unit_finishes traversal
#define SEE_CLEAR  0x2000  // exists a path from node spelling '(epsilon|WHITE)* non-unit_edge'


/* In a reverse topo. order scan we first determine whether one can see <text> ('SEE_TEXT')
   or WHITE* TYPE ('SEE_TYPE') on some path from a state (including the state).  This sets
   mark bits: SEE_MARK (all), SEE_TEXT, and SEE_TYPE.
*/

static void compute_unit_lookaheads(State *s)
{ s->mark |= SEE_MARK;

  if (s->succ[0] != NULL && (s->succ[0]->mark & SEE_MARK) == 0)
    compute_unit_lookaheads(s->succ[0]);

  if (s->succ[1] != NULL && (s->succ[1]->mark & SEE_MARK) == 0)
    compute_unit_lookaheads(s->succ[1]);

  if (s->succ[0] != NULL)
    { s->mark |= s->succ[0]->mark;
      if (s->succ[1] != NULL && (s->flags & LOOP_FINISH) == 0)
        s->mark |= s->succ[1]->mark;
    }

  { Atom *p;

    p = s->atom;
    if (p != NULL)
      { if (p->label != WHITE)
          s->mark = 0;
        else
          s->mark &= ~SEE_TEXT;
        if (p->label == TYPE)
          s->mark |= SEE_TYPE;
        if (TEXT <= p->label && p->label != WHITE)
          s->mark |= SEE_TEXT;
      }
  }
}

/* In a forward topo. order scan we check that every path matches the automaton above,
   reporting an error if it does not.  In addition we make sure {} bracket only <text>.
   We also mark the start of units and with the aid of the SEE_TEXT and SEE_TYPE flags we
   can determine which edges are in a unit.  This sets bits 1-8 corresponding to the 8 states
   of the automaton above.
*/

  // Anciliary routine to find first required argument preceding state s (when bit 5(0x10) is set)

static Atom *find_previous_name(State *s)
{ while (s->atom == NULL || s->atom->label != NAME)
    if (s->pred[0] != NULL && (s->pred[0]->mark & 0x10) != 0)
      s = s->pred[0];
    else
      s = s->pred[1];
  return (s->atom);
}

  // Anciliary routine to mark edge from s to t if it is within a unit

static void classify_edge(State *s, State *t, int flag)
{ if ((t->mark & SEE_TYPE) != 0 && (s->mark & 0x4e) != 0)
    s->flags |= flag;
  if ((t->mark & SEE_TEXT) != 0 && (s->mark & 0x62) != 0)
    s->flags |= flag;
}

static void check_automaton_units(State *s)
{ s->mark |= 1;

  if (s->pred[0] != NULL && (s->pred[0]->mark & 0xff) == 0)
    check_automaton_units(s->pred[0]);

  if (s->pred[1] != NULL && (s->pred[1]->mark & 0xff) == 0)
    check_automaton_units(s->pred[1]);

  { unsigned int in, out;
    Atom        *p;
    State       *t, *u;

    t  = s->pred[0];
    u  = s->pred[1];
    in = 0;
    if (t == NULL && u == NULL)
      in = 0x1;
    if (t != NULL)
      in |= (t->mark & 0xff);
    if (u != NULL && (s->flags & LOOP_START) == 0)
      in |= (u->mark & 0xff);

    p   = s->atom;
    out = 0;
    if (p == NULL)
      out = in;
    else
      switch (p->label)
      { case SYN_L:
          if ((in & 0x07) != 0)
            out |= 0x2;
          if ((in & 0xf8) != 0)    // back up to atom just after {
            { do
                s = s->succ[0];
              while (s->atom == NULL);
              semantic_error(CP_ERROR,s->atom);
            }
          if ((in & 0x05) != 0)
            s->flags |= START_FLAG;
          break;
        case SYN_R:
          if ((in & 0x02) != 0)
            out |= 0x2;
          if ((in & 0xfd) != 0)    // back up to atom just before }
            { do
                { if (s->pred[1] != NULL)
                    s = s->pred[1];
                  else
                    s = s->pred[0];
                }
              while (s->atom == NULL);
              semantic_error(CP_ERROR,s->atom);
            }
           break;
        case TYPE:
          if ((in & 0x2) != 0)
            out |= 0x40;
          if ((in & 0x4) != 0)
            out |= 0x08;
          if ((in & 0x01) != 0)
            semantic_error(TO_ERROR,p);
          if ((in & 0x48) != 0)
            semantic_error(TT_ERROR,p);
          if ((in & 0x10) != 0)
            semantic_error(NR_ERROR,find_previous_name(s));
          if ((in & 0x20) != 0)
            semantic_error(TL_ERROR,p);
          if ((in & 0x80) != 0)
            semantic_error(CT_ERROR,p);
          break;
        case NAME:
          if ((in & 0x05) != 0)
            out |= 0x10;
          if ((in & 0xfa) != 0)
            semantic_error(NL_ERROR,p);
          s->flags |= START_FLAG;
          break;
        case TEXT:
          if ((in & 0x42) != 0)
            out |= 0x02;
          if ((in & 0x20) != 0)
            out |= 0x20;
          if ((in & 0x85) != 0)
            semantic_error(BO_ERROR,p);
          if ((in & 0x10) != 0)
            semantic_error(NR_ERROR,find_previous_name(s));
          else if ((in & 0x08) != 0)
            semantic_error(TX_ERROR,p);
          break;
        case DASH:
          if ((in & 0xc7) != 0)
            out |= 0x2;
          if ((in & 0x20) != 0)
            out |= 0x20;
          if ((in & 0x10) != 0)
            semantic_error(NR_ERROR,find_previous_name(s));
          else if ((in & 0x8) != 0)
            semantic_error(TX_ERROR,p);
          if ((in & 0x5) != 0)
            s->flags |= START_FLAG;
          break;
        case LIST:
          if ((in & 0x5) != 0)
            { out |= 0x20;
              s->flags |= START_FLAG;
            }
          if ((in & 0x42) != 0)
            out |= 0x2;
          if ((in & 0x20) != 0)
            out |= 0x20;
          if ((in & 0x18) != 0)
            semantic_error(OP_ERROR,p);
          if ((in & 0x80) != 0)
            semantic_error(BO_ERROR,p);
          break;
        case WHITE:
          if ((in & 0x31) != 0)
            out |= 0x1;
          if ((in & 0x4e) != 0)
            out |= 0x4;
          if ((in & 0x80) != 0)
            semantic_error(CW_ERROR,p);
          break;
      }

    s->mark = out | (s->mark & ~0xff);

    t = s->succ[0];
    u = s->succ[1];
    if (t != NULL)
      classify_edge(s,t,MORE0_FLAG);
    if (u != NULL && (s->flags & LOOP_FINISH) == 0)
      classify_edge(s,u,MORE1_FLAG);
  }
} 

/* Finally, in a reverse topo. order scan we determine the ends of every unit by finding
   those states that are in any one of automaton states 2, 4, 5, 6, or 7 (0x7a), and see
   a non-unit edge through epsilon or WHITE nodes ('SEE_CLEAR').  We also check here that
   units are wholly within the scope of a ... and that a pure option is not within a loop.
   In addition the minimum and maximum number of required arguments on a path from each
   state is computed.  This routine uses mark bits CLEAR_MARK and SEE_CLEAR.
*/

  // Anciliary routine: s is within a unit, find first non-epsilon part following s

static Atom *unit_split(State *s)
{ while (s->atom == NULL)
    { if ((s->flags & MORE0_FLAG) != 0)
        s = s->succ[0];
      else
        s = s->succ[1];
    }
  return (s->atom);
}

  // Anciliary routine: return start atom of pure option through s (or NULL if !exists)

static Atom *pure_option(State *s)
{ if (s->atom != NULL)
    { if (s->atom->label < TEXT || s->atom->label == WHITE)
        return (NULL);
    }
  if ((s->flags & START_FLAG) != 0)
    return (s->atom);
  if (s->pred[0] != NULL && (s->pred[0]->mark & 0xa2) != 0)
    { Atom *p = pure_option(s->pred[0]);
      if (p != NULL)
        return (p);
    }
  if (s->pred[1] != NULL && (s->pred[1]->mark & 0xa2) != 0)
    { Atom *p = pure_option(s->pred[1]);
      if (p != NULL)
        return (p);
    }
  return (NULL);
}

static void compute_unit_finishes(State *s)
{ s->mark |= CLEAR_MARK;

  if (s->succ[0] != NULL && (s->succ[0]->mark & CLEAR_MARK) == 0)
    compute_unit_finishes(s->succ[0]);

  if (s->succ[1] != NULL && (s->succ[1]->mark & CLEAR_MARK) == 0)
    compute_unit_finishes(s->succ[1]);

  if (s->succ[0] == NULL && s->succ[1] == NULL)
    s->mark |= SEE_CLEAR;
  if (s->succ[0] != NULL)
    { if ((s->succ[0]->mark & SEE_CLEAR) != 0 || (s->flags & MORE0_FLAG) == 0)
        s->mark |= SEE_CLEAR;
    }
  if (s->succ[1] != NULL)
    { if ((s->succ[1]->mark & SEE_CLEAR) != 0 || (s->flags & MORE1_FLAG) == 0)
        s->mark |= SEE_CLEAR;
    }

  if (s->atom != NULL)
    { if ((s->mark & SEE_CLEAR) != 0 && (s->mark & 0x7a) != 0)
        s->flags |= FINISH_FLAG;
      if (s->atom->label != WHITE)
        s->mark &= ~SEE_CLEAR;
    }

  if ((s->flags & LOOP_START) != 0 && (s->pred[0]->flags & MORE0_FLAG) != 0)
    semantic_error(PL_ERROR,unit_split(s));
  if ((s->flags & LOOP_FINISH) != 0 && (s->flags & MORE0_FLAG) != 0)
    semantic_error(PL_ERROR,unit_split(s->succ[0]));

  if ((s->flags & FINISH_FLAG) != 0 && s->loop != NULL)  // Cannot have a pure option inside ...
    { Atom *p = pure_option(s);
      if (p != NULL)
        semantic_error(OL_ERROR,p);
    }

  if (s->succ[0] == NULL)
    s->minlen = s->maxlen = 0;
  else
    { s->minlen = s->succ[0]->minlen;
      s->maxlen = s->succ[0]->maxlen;
      if (s->succ[1] != NULL && (s->flags & LOOP_FINISH) == 0)
        { if (s->succ[1]->minlen < s->minlen) 
            s->minlen = s->succ[1]->minlen;
          if (s->succ[1]->maxlen > s->maxlen)
            s->maxlen = s->succ[1]->maxlen;
        }
    }
  if ((s->flags & START_FLAG) != 0 && s->atom->label == NAME)
    { s->minlen += 1;
      s->maxlen += 1;
    }
}


/*****************************************************************************************/
/*   UNIT TABLE DEFINITIONS AND GLOBALS                                                  */
/*****************************************************************************************/

  /* A unit matches a contiguous sequence of arguments on the command line and are of
       three types: options, flag lists, and required args.  The exact state sequences
       that constitute a valid instance of each are defined with the code for checking and
       identifying them above.
  */

#define OPTION   0  // The three types of units and ...
#define REQUIRED 1
#define FLAGS    2
#define OFLAG    3  // ... a flag that is specified as an option and so may have synonyms!

  // A possible match to a unit

typedef struct _Candidate
  { struct _Candidate *next;   //  Linked list of possible matches to a unit
    int                argp;   //  index of argument matched
    Alpha_Set          set;    //  if a FLAG|OFLAG, the set of chars in in the matching arg
  } Candidate;

  // Values and iteration count of a unit in a final match

typedef struct _Match
  { struct _Match *next;   // linked list headed at unit definition
    Value         *value;  // array of values for unit
    int            iter;   // loop iteration (if in a loop), 1 otherwise
  } Match;

  // A tabled unit record

typedef struct _unit
  { struct _unit *inext;  // linked list of instances starting at state beg
    State        *beg;    // first state in the automaton of the instance
    State        *end;    // last state in the automaton of the instance

    Atom        **unit;   // unit[0..len-1] is the sequence of atoms defining the unit
    int           len;    //   (see the comments for SEMANTIC CHECK OF FINITE AUTOMATON)
    int           nvals;  // number of value templates in the unit
    int           span;   // number of command line arguments spanned by the unit (options only)
    int           kind;   // type of unit (OPTION, REQUIRED, FLAGS, OFLAG)
    Candidate    *clist;  // list of command line arguments that could match this unit
    int           equiv;  // Circular list (initially singly-linked) of synonyms
    int           equal;  // Circular list (initially singly-linked) of equal units

    int           used;   // instance is used in the reported match
    int           maxi;   // max-loop iteration (if match is in loop)
    Match        *alist;  // linked list of Match'es for the unit

    int           mlist;  // linked list of matches to a query
    short         hit;    // name matches this entry
    short         prefix; // name match is a prefix of the full name
  } Unit;

static Unit   *Unit_Table = NULL;   // Unit_Table[0..Num_of_Units-1] is a
static int     Num_of_Units;        //     table of all distinct unit instances
static int     Unit_Table_Max = 0;

static Value *Vtop;                 //  Value allocation pointer
static char  *Ttop;                 //  Text value allocation pointer
static int    Fill_Values;          //  record values while matching?

static Alpha_Set Flag_Vector;          //  flags actually matched
static Alpha_Set Flag_Space;           //  all possible flags in the spec

#ifdef DEBUG

static void print_unit(FILE *file, Unit *def)
{ int i, curly_odd;

  curly_odd = 1;
  fprintf(file,"Unit: .");
  for (i = 0; i < def->len; i++)
    { Atom *s = def->unit[i];
      switch (s->label)
      { case TYPE:
          print_type(file,s);
          break;
        case NAME:
          fprintf(file,"<%.*s:%s>",s->nlen,s->name,Type_String[s->def_val.type]);
          break;
        case TEXT:
        case LIST:
        case DASH:
          if (def->kind == FLAGS && i == 0)
            fprintf(file,"!%.*s",s->nlen-1,s->name+1);
          else
            fprintf(file,"%.*s",s->nlen,s->name);
          break;
        case WHITE:
          fprintf(file," ");
          break;
        case SYN_L:
          fprintf(file,"{");
          break;
        case SYN_R:
          fprintf(file,"}");
          break;
      }
    }
  fprintf(file,". <>%d |%d| ~%d =%d ...%d",def->span,def->nvals,def->equiv,def->equal,def->maxi);
}

static void print_instance(FILE *file, void *list, int show_matches, int walk_list)
{ Unit      *u = (Unit *) list;
  Candidate *m;

  while (u != NULL)
    { fprintf(file,"       %2d %2d ",u->beg->number,u->end->number);
      print_unit(file,u);
      fprintf(file,"\n");
      if (show_matches)
        for (m = u->clist; m != NULL; m = m->next)
          fprintf(file,"               %2d: %s\n",m->argp,ArgV[m->argp]);
      if (!walk_list) break;
      u = u->inext;
    }
}

#endif


/*****************************************************************************************/
/*   BUILD A TABLE OF ALL UNIT OCCURENCES                                                */
/*****************************************************************************************/

static Atom **AtStk;       //  Stack of atoms currently traversed by the recursive travel of a unit
static int    Stop;        //  Top of said stack
static State *Start_State; //  First state of unit currently being traversed
static int    Unit_Span;   //  Current span (# of WHITE's) in current unit

//  For the option currently on Stack, find the option with the largest index that is
//    a synonym (matches ignoring internals of {} pairs)
 
static int equivalent_option()
{ int    k;

  for (k = Num_of_Units-2; k >= 0; k--)
    if (Unit_Table[k].kind != FLAGS && Unit_Table[k].kind != REQUIRED)
      { Atom **unit = Unit_Table[k].unit;
        int    ulen = Unit_Table[k].len;
        int    i, j, vi, vj;
  
        vi = vj = 0;
        for (i = 0; i < ulen; i++)
          if (unit[i]->label == TYPE)
            { if (vi >= Stop) goto nomatch;
              if (AtStk[vi]->label != TYPE || AtStk[vi]->def_val.type != unit[i]->def_val.type)
                goto nomatch;
              vi += 1;
            }
          else if (unit[i]->label == WHITE)
            { if (vi >= Stop) goto nomatch;
              if (AtStk[vi]->label != WHITE)
                goto nomatch;
              vi += 1;
            }
          else if (unit[i]->label == SYN_L)
            { if (vi >= Stop) goto nomatch;
              if (AtStk[vi] != unit[i])
                goto nomatch;
              vi += 1;
              while (AtStk[vi]->label != SYN_R)
                vi += 1;
              i += 1;
              while (unit[i]->label != SYN_R)
                i += 1;
              vi += 1;
            }
          else // unit[i]->label == TEXT|LIST|DASH 
            { char *name = unit[i]->name;
              int   nlen = unit[i]->nlen;
  
              for (j = 0; j < nlen; j++)
                { if (vi >= Stop) goto nomatch;
                  if (AtStk[vi]->label < TEXT || AtStk[vi]->label > DASH) goto nomatch;
                  if (name[j] != AtStk[vi]->name[vj]) goto nomatch;
                  vj += 1;
                  if (vj >= AtStk[vi]->nlen)
                    { vi += 1; vj = 0; }
                }
            } 
        if (vi == Stop)
          return (k);
      nomatch:
        continue;
      } 
  return (-1);
}

//  For the unit currently on Stack, find the unit with the largest index that is
//    equal to it (if any).
 
static int equal_unit()
{ int    k;

  for (k = Num_of_Units-2; k >= 0; k--)
    if (Unit_Table[k].kind != FLAGS)
      { Atom **unit = Unit_Table[k].unit;
        int    ulen = Unit_Table[k].len;
        int    i, j, vi, vj;
        int    defmat;
  
        defmat = 1;
        vi = vj = 0;
        for (i = 0; i < ulen; i++)
          { while (vi < Stop && (AtStk[vi]->label == SYN_L || AtStk[vi]->label == SYN_R))
              vi += 1;
            if (unit[i]->label == TYPE)
              { if (vi >= Stop) goto nomatch;
                if (AtStk[vi]->label != TYPE || AtStk[vi]->def_val.type != unit[i]->def_val.type)
                  goto nomatch;
                if (AtStk[vi]->has_def != unit[i]->has_def)
                  defmat = 0;
                else if (AtStk[vi]->has_def)
                  switch (AtStk[vi]->def_val.type)
                  { case INT_TYPE:
                      if (AtStk[vi]->def_val.value.intval != unit[i]->def_val.value.intval)
                        defmat = 0;
                      break;
                    case DOUBLE_TYPE:
                      if (AtStk[vi]->def_val.value.dblval != unit[i]->def_val.value.dblval)
                        defmat = 0;
                      break;
                    case STRING_TYPE:
                      if (AtStk[vi]->def_val.value.strval != unit[i]->def_val.value.strval)
                        defmat = 0;
                      break;
                  }
                vi += 1;
              }
            else if (unit[i]->label == WHITE)
              { if (vi >= Stop) goto nomatch;
                if (AtStk[vi]->label != WHITE)
                  goto nomatch;
                vi += 1;
              }
            else if (unit[i]->label == SYN_L || unit[i]->label == SYN_R)
              continue;
            else if (unit[i]->label == NAME)
              { if (vi >= Stop) goto nomatch;
                if (unit[i]->nlen != AtStk[vi]->nlen)
                  goto nomatch;
                if (strncmp(unit[i]->name,AtStk[vi]->name,unit[i]->nlen))
                  goto nomatch;
                if (AtStk[vi]->def_val.type != unit[i]->def_val.type)
                  goto nomatch;
                vi += 1;
              }
            else // unit[i]->label == TEXT|LIST|DASH 
              { char *name = unit[i]->name;
                int   nlen = unit[i]->nlen;
    
                for (j = 0; j < nlen; j++)
                  { if (vi >= Stop) goto nomatch;
                    if (AtStk[vi]->label < TEXT || AtStk[vi]->label > DASH) goto nomatch;
                    if (name[j] != AtStk[vi]->name[vj]) goto nomatch;
                    vj += 1;
                    if (vj >= AtStk[vi]->nlen)
                      { vi += 1;
                        vj = 0;
                        while (vi < Stop && (AtStk[vi]->label==SYN_L || AtStk[vi]->label==SYN_R))
                          vi += 1;
                      }
                  }
              } 
          }
        while (vi < Stop && (AtStk[vi]->label == SYN_L || AtStk[vi]->label == SYN_R))
          vi += 1;
        if (vi == Stop)
          { if (!defmat)
              dual_error(DF_ERROR,AtStk[0],unit[0]);
            return (k); 
          }
      nomatch:
        continue;
      } 
  return (-1);
}

//  Add the unit currently in AtStk[0..Stop-1] of type 'kind' and ending at state 'end'
//    to the unit table.

static void add_unit_instance(int kind, State *end)
{ int   has_curly, k;
  Unit *def;

  has_curly = 0;
  if (kind == OPTION || kind == OFLAG)
    { for (k = 0; k < Stop; k++)
        if (AtStk[k]->label == SYN_L)
          has_curly = 1;
    }

  if (Num_of_Units >= Unit_Table_Max)
    { Unit_Table_Max = 1.2*Num_of_Units + 100;
      Unit_Table = (Unit *)
        Guarded_Realloc(Unit_Table,sizeof(Unit)*Unit_Table_Max,"Process_Arguments");
    }

  def = Unit_Table + Num_of_Units++;

  def->unit = (Atom **) Guarded_Malloc(sizeof(Atom *)*Stop,"Process_Arguments");
  memcpy(def->unit,AtStk,sizeof(Atom *)*Stop);
  def->len    = Stop;
  def->span   = Unit_Span;
  def->kind   = kind;
  def->clist  = NULL;
  def->maxi   = 0;
  def->used   = 0;
  def->alist  = NULL;

  if (kind == OPTION)
    { def->nvals = 0;
      for (k = 0; k < Stop; k++)
        if (AtStk[k]->label == TYPE)
          def->nvals += 1;
    }
  else if (kind == REQUIRED)
    def->nvals = 1;
  else
    def->nvals = 0;

  if (has_curly)
    def->equiv = equivalent_option();
  else
    def->equiv = -1;
  if (kind != FLAGS)
    def->equal = equal_unit();
  else
    def->equal = -1;

  def->beg   = Start_State;
  def->end   = end;
  def->inext = (Unit*)Start_State->ulist;
  Start_State->ulist = def;
}

// Recursively traverse a unit pushing its atoms onto "Stack" as you go and when you
//   get to the end of it add the unit (sequence of atoms in AtStk[0..Stop-1]) to the
//   unit table by calling "add_unit_instance".

static void traverse_unit(State *s)
{ int pushed;

  pushed = 0;
  if (s->atom != NULL)
    if (s->atom->label != WHITE || Stop == 0 || AtStk[Stop-1]->label != WHITE)
      { pushed = 1;
        AtStk[Stop++] = s->atom;
        if (s->atom->label == WHITE) Unit_Span += 1;
      }

  if ((s->flags & FINISH_FLAG) != 0)
    { int kind, olen, opos;

      //  If option unit is a flag, code it as such

      if (AtStk[0]->label == DASH)
        { olen = 1; opos = 0; }
      else
        { olen = 3; opos = 1; }
      if (AtStk[0]->label == DASH || AtStk[0]->label == SYN_L)
        { if (AtStk[opos]->nlen == 2 && Stop == olen)
            kind = OFLAG;
          else if (AtStk[opos]->nlen == 1 && Stop == olen+1 &&
                   AtStk[opos+1]->label == TEXT && AtStk[opos+1]->nlen == 1)
            kind = OFLAG;
          else
            kind = OPTION;
        }
      else if (AtStk[0]->label == NAME)
        kind = REQUIRED;
      else
        kind = FLAGS;

      add_unit_instance(kind,s);
    }

  if ((s->flags & MORE0_FLAG) != 0)
    traverse_unit(s->succ[0]);
  if ((s->flags & MORE1_FLAG) != 0)
    traverse_unit(s->succ[1]);

  if (pushed)
    { Stop--;
      if (s->atom->label == WHITE) Unit_Span -= 1;
    }
}

// Recursively perform a topo. search of the automaton looking for the starts of units
//   and whenever you find one call "traverse_unit" on it.

static void find_unit_starts(State *s)
{ s->mark = 1;

  if (s->succ[0] != NULL && s->succ[0]->mark == 0)
    find_unit_starts(s->succ[0]);
  if (s->succ[1] != NULL && s->succ[1]->mark == 0)
    find_unit_starts(s->succ[1]);

  if ((s->flags & START_FLAG) != 0)
    { Start_State = s;
      traverse_unit(s);
    }
}

//  Convert singly linked ".equiv" lists in the unit table into circularly linked lists

static void circularize_equiv_lists()
{ int  i, x, y, z;

  for (i = 0; i < Num_of_Units; i++)     // Trick: scan chain in reverse order making
    if ((x = Unit_Table[i].equiv) >= 0)  //   the end of the chain the 2nd element of
      { y = Unit_Table[x].equiv;         //   the currently scanned part ...
        z = Unit_Table[y].equiv;
        Unit_Table[x].equiv = z;
        Unit_Table[y].equiv = x;
        Unit_Table[i].equiv = y;
      }
    else
      Unit_Table[i].equiv = i;

  for (i = 0; i < Num_of_Units; i++)    // ... then move the 2nd element to the end
    { x = Unit_Table[i].equiv;          //   effectively circularizing
      y = Unit_Table[x].equiv;
      if (i > y && y >= x)
        { Unit_Table[i].equiv = y;
          Unit_Table[x].equiv = i;
        }
    }
}

//  Convert singly linked ".equal" lists in the unit table into circularly linked lists

static void circularize_equal_lists()
{ int  i, x, y, z;

  for (i = 0; i < Num_of_Units; i++)     // Trick: scan chain in reverse order making
    if ((x = Unit_Table[i].equal) >= 0)  //   the end of the chain the 2nd element of
      { y = Unit_Table[x].equal;         //   the currently scanned part ...
        z = Unit_Table[y].equal;
        Unit_Table[x].equal = z;
        Unit_Table[y].equal = x;
        Unit_Table[i].equal = y;
      }
    else
      Unit_Table[i].equal = i;

  for (i = 0; i < Num_of_Units; i++)    // ... then move the 2nd element to the end
    { x = Unit_Table[i].equal;          //   effectively circularizing
      y = Unit_Table[x].equal;
      if (i > y && y >= x)
        { Unit_Table[i].equal = y;
          Unit_Table[x].equal = i;
        }
    }
}

static void check_dups_back(State *s, Unit *d)   // traverse backwards looking for a member of
{ Unit *u;                                       // d's class on a path back to the start state

  if ((s->flags & START_FLAG) != 0)
    { for (u = (Unit *) (s->ulist); u != NULL; u = u->inext)
        if (u->used)
          dual_error(DP_ERROR,u->unit[0],d->unit[0]);
    }
  if (s->pred[0] != NULL)
    check_dups_back(s->pred[0],d);
  if (s->pred[1] != NULL && (s->flags & LOOP_FINISH) == 0)
    check_dups_back(s->pred[1],d);
}

static void check_for_duplicates()
{ int i, k;
 
  for (i = 0; i < Num_of_Units; i++)    // for each unit with a non-trivial equal list
    if (Unit_Table[i].equal > i)        // mark all the class & see if one proceeds another
      { Unit_Table[i].used = 1;
        for (k = Unit_Table[i].equal; k != i; k = Unit_Table[k].equal)
          Unit_Table[k].used = 1;

        k = i;
        do
          { State *s = Unit_Table[k].beg;
            if (s->pred[0] != NULL)
              check_dups_back(s->pred[0],Unit_Table+k);
            if (s->pred[1] != NULL && (s->flags & LOOP_FINISH) == 0)
              check_dups_back(s->pred[1],Unit_Table+k);
            k = Unit_Table[k].equal;
          }
        while (k != i);

        Unit_Table[i].used = 0;
        for (k = Unit_Table[i].equal; k != i; k = Unit_Table[k].equal)  // clean up: unmark
          Unit_Table[k].used = 0;
      }
}

//  Build a table of all the units in the automaton in Unit_Table[0..Num_of_Units-1].
//    Note that the number of units can be exponential in automaton size in the worst case.

static void build_unit_table(Automaton *mach)
{ AtStk     = (Atom **) Guarded_Malloc(sizeof(Atom *)*mach->longest,"Process_Arguments");
  Stop      = 0;
  Unit_Span = 0;

  { int k;

    for (k = 0; k < 4; k++)
      Flag_Space[k] = 0;
    Num_of_Units = 0;
    Fill_Values  = 0;
  }

#ifdef DEBUG_MATCH
  printf("\n");
#endif

  find_unit_starts(mach->start);
  reset_marks(mach->start);
  circularize_equiv_lists();
  circularize_equal_lists();

  free(AtStk);

  check_for_duplicates();

#ifdef DEBUG_MATCH
  { int k;

    printf("\nTabled Unit Instances %d\n",Num_of_Units);
    for (k = 0; k < Num_of_Units; k++)
      print_instance(stdout,Unit_Table+k,0,0);
  }
#endif
}


/*****************************************************************************************/
/*   FIND CANDIDATE MATCHES TO EACH UNIT                                                 */
/*****************************************************************************************/

// Add a match to the argp'th argument to unit def's list of matches

static inline Candidate *add_match(Unit *def, int argp)
{ Candidate *mat = (Candidate  *) Guarded_Malloc(sizeof(Candidate),"Process_Argument");
  mat->next  = def->clist;
  mat->argp  = argp;
  def->clist = mat;
  return (mat);
}

// Compare *flag* unit def against all arguments on the command line

static void match_flags(Unit *def)
{ int set[128]; 
  int first;
  int i, j, k;
                
  for (i = 0; i < 128; i++)
    set[i] = 0;     
 
  first = 1;
  for (i = 0; i < def->len; i++)
    { char *name = def->unit[i]->name;
      int   nlen = def->unit[i]->nlen;
 
      if (def->unit[i]->label == SYN_R || def->unit[i]->label == SYN_L)
        continue;
      for (j = (first ? 1 : 0); j < nlen; j++)
        { int v = name[j];
          set[v] = 1;
          Flag_Space[v>>5] |= (1 << (v&0x1f));
        }
      first = 0;
    }               
                      
  for (i = 1; i < ArgC; i++)
    if (ArgV[i][0] == OPTION_CHAR)
      { for (j = 1; ArgV[i][j] != '\0'; j++)
          if (set[(int) ArgV[i][j]])
            break;
        if (ArgV[i][j] != '\0')
          { Candidate *mat = add_match(def,i);
            for (k = 0; k < 4; k++)
              mat->set[k] = 0;
            while (ArgV[i][j] != '\0')
              { k = ArgV[i][j];
                if (set[k])
                  mat->set[k>>5] |= (1 << (k&0x1f));
                j += 1;
              }
#ifdef DEBUG_MATCH
            print_unit(stdout,def);
            printf(" matches %s\n",ArgV[i]);
#endif
          }
      }   
}

// Return non-zero if val[0..len-1] matches an integer constant.  Also push the value
//   onto the array pointed at by Vtop if Fill_Values is on.

static int match_int(char *val, int len)
{ int   c, m, v;
  char *e;

  c = val[len];
  val[len] = '\0';
  v = strtol(val,&e,10);
  m = (*val != '\0' && *e == '\0');
  val[len] = c;
  if (m && Fill_Values)
    { Vtop->type = INT_TYPE;
      Vtop->value.intval = v;
      Vtop += 1;
    }
  return (m);
}

// Return non-zero if val[0..len-1] matches a floating-point constant.  Also push the value
//   onto the array pointed at by Vtop if Fill_Values is on.

static int match_double(char *val, int len)
{ int    c, m;
  char  *e;
  double d;

  c = val[len];
  val[len] = '\0';
  d = strtod(val,&e);
  m = (*val != '\0' && *e == '\0');
  val[len] = c;
  if (m && Fill_Values)
    { Vtop->type = DOUBLE_TYPE;
      Vtop->value.dblval = d;
      Vtop += 1;
    }
  return (m);
}

// If Fill_Values is non-zero push val[0..len-1] onto the result stack (Vtop + Ttop)

static void push_string(char *val, int len)
{ if (!Fill_Values) return;
  Vtop->type = STRING_TYPE;
  Vtop->value.strval = Ttop;
  Vtop += 1;
  strncpy(Ttop,val,len);
  Ttop += len;
  *Ttop++ = '\0';
}

// Pop the last result placed on the results stack (iff Fill_Value is non-zero)

static void pop_type_match()
{ if (!Fill_Values) return;
  Vtop -= 1;
  if (Vtop->type == STRING_TYPE)
    Ttop = Vtop->value.strval;
}

// Compare *required* unit def against arg

static int match_required_arg(Unit *def, char *arg)
{ int m;

  switch (def->unit[0]->def_val.type)
  { case INT_TYPE:
      m = match_int(arg,strlen(arg));
      break;
    case DOUBLE_TYPE:
      m = match_double(arg,strlen(arg));
      break;
    default:
      m = (arg[0] != OPTION_CHAR);
      if (m)
        push_string(arg,strlen(arg));
      break;
  }
  return (m);
}

// Compare *required* unit def against all arguments on the command line

static void match_required(Unit *def)
{ int i;

  for (i = ArgC-1; i >= 1; i--)      //  Loop order is important!
    if (match_required_arg(def,ArgV[i]))
      { add_match(def,i);
#ifdef DEBUG_MATCH
        print_unit(stdout,def);
        printf(" matches %s\n",ArgV[i]);
#endif
      }
}

// Return non-zero only if name[0..len] matches text where quoted characters can
//   occur in name (but not in text).

int match_text(char *name, char *text, int len)
{ int i, j;

  for (i = j = 0; i < len; i++, j++)
    { if (name[i] == ESCAPE_CHAR)
        i += 1;
      if (name[i] != text[j])
        return (0);
    }
  return (1);
}

// Return non-zero only if unit def from the i'th atom onwards matches the command
//   line starting at the character pointed at by arg, which is the first string pointed
//   at by argv (the rest of the command line following it in order).

int match_option_suffix(Unit *def, int i, char *arg, char **argv)
{ if (i >= def->len) 
    { if (*arg == '\0')
        return (1);
      else
        return (0);
    }

  switch (def->unit[i]->label)

  { case TEXT:
    case LIST:
    case DASH:
      if (match_text(def->unit[i]->name,arg,def->unit[i]->nlen))
        return (match_option_suffix(def,i+1,arg+def->unit[i]->slen,argv));
      else
        return (0);

    case SYN_L:
    case SYN_R:
      return (match_option_suffix(def,i+1,arg,argv));

    case TYPE:
      if (i+1 >= def->len || def->unit[i+1]->label == WHITE)
        { switch (def->unit[i]->def_val.type)
          { case STRING_TYPE:
              push_string(arg,strlen(arg));
              if (match_option_suffix(def,i+1,arg+strlen(arg),argv))
                return (1);
              pop_type_match();
              return (0);
            case INT_TYPE:
              if (match_int(arg,strlen(arg)))
                { if (match_option_suffix(def,i+1,arg+strlen(arg),argv))
                    return (1);
                  pop_type_match();
                }
              return (0);
            case DOUBLE_TYPE:
              if (match_double(arg,strlen(arg)))
                { if (match_option_suffix(def,i+1,arg+strlen(arg),argv))
                    return (1);
                  pop_type_match();
                }
              return (0);
          }
        }

      else  // def->unit[i+1]->label == TEXT|LIST|DASH
        { Atom *state = def->unit[i+1];
          char *sname = state->name;
          int   nlen  = state->nlen;
          int   slen  = state->slen;
          int   off;

          off = 0; 
          for (off = 1; off <= (int) strlen(arg) - slen; off++)
            { if (match_text(sname,arg+off,nlen))
                { switch (def->unit[i]->def_val.type)
                  { case STRING_TYPE:
                      push_string(arg,off);
                      if (match_option_suffix(def,i+2,arg+off+slen,argv))
                        return (1);
                      pop_type_match();
                      break;
                    case INT_TYPE:
                      if (match_int(arg,off))
                        { if (match_option_suffix(def,i+2,arg+off+slen,argv))
                            return (1);
                          pop_type_match();
                        }
                      break;
                    case DOUBLE_TYPE:
                      if (match_double(arg,off))
                        { if (match_option_suffix(def,i+2,arg+off+slen,argv))
                            return (1);
                          pop_type_match();
                        }
                      break;
                  }
                }
            }
          return (0);
        }

    case WHITE:
      if (*arg != '\0')
        return (0);
      argv += 1;
      return (match_option_suffix(def,i+1,*argv,argv));
  }

  return (0);
}

// Compare *option* unit def against all arguments on the command line

static void match_option(Unit *def)
{ int i;

  for (i = 1; i < ArgC - def->span; i++)
    if (match_option_suffix(def,0,ArgV[i],ArgV+i))
      { add_match(def,i);
#ifdef DEBUG_MATCH
        print_unit(stdout,def);
        printf(" matches %s\n",ArgV[i]);
#endif
      }
}

// Find all possible argument matches to all units

static void find_candidate_matches()
{ int k;

  for (k = 0; k < Num_of_Units; k++)
    switch (Unit_Table[k].kind)
    { case FLAGS:
      case OFLAG:
        match_flags(Unit_Table+k);
        break;
      case OPTION:
        match_option(Unit_Table+k);
        break;
      case REQUIRED:
        match_required(Unit_Table+k);
        break;
    }
}


/*****************************************************************************************/
/*   CHECK THAT A MATCH IS PLAUSIBLE                                                     */
/*****************************************************************************************/

static void is_match_possible(Automaton *mach, Node *root, int no_escapes)
{ int       *has_a_match = (int *) Guarded_Malloc(sizeof(int)*2*ArgC,"Process_Arguments");
  int       *chain_cover = has_a_match + ArgC;
  Alpha_Set *all_flags   = (Alpha_Set *) Guarded_Malloc(sizeof(Alpha_Set)*ArgC,"Process_Arguments");

//  Scan the Unit_Table to determine:
//      has_a_match[i] is 1 iff a non-flag unit matches arg i
//      chain_cover[i] is 1 iff an option type matches arg i
//      all_flags[i] = the union of all flag matches to arg i

  { int        i, k;
    Candidate *m;

    for (i = 1; i < ArgC; i++)
      { has_a_match[i] = 0;
        chain_cover[i] = 0;
        for (k = 0; k < 4; k++)
          all_flags[i][k] = 0;
      }

    for (i = 0; i < Num_of_Units; i++)
      for (m = Unit_Table[i].clist; m != NULL; m = m->next)
        switch (Unit_Table[i].kind)
        { case OPTION:
            for (k = 1; k <= Unit_Table[i].span; k++)
              { has_a_match[m->argp+k] = 1;
                chain_cover[m->argp+k] = 1;
              }
          case REQUIRED:
            has_a_match[m->argp] = 1;
            break;
          case OFLAG:
          case FLAGS:
            for (k = 0; k < 4; k++)
              all_flags[m->argp][k] |= m->set[k];
            break;
        }
  }

// Compute the maximum and minimum number of required arguments on the command line
//   (nreq,oreq) and make sure they do not preclude a match.  Also make sure that
//   every command line argument is matched by at least unit.

  { int i, k, nreq, oreq;
    int rmin, rmax;
    
    rmin = mach->start->minlen;
    rmax = mach->start->maxlen;
    if (root->loops && ArgC > rmax)
      rmax = ArgC;

    nreq = oreq = 0;
    for (i = 1; i < ArgC; i++)
      { if (ArgV[i][0] != OPTION_CHAR)
          { nreq += 1;
            if (!chain_cover[i])
              oreq += 1;
          }
        if (has_a_match[i])
          continue;
        if (ArgV[i][0] == OPTION_CHAR)
          { for (k = 1; ArgV[i][k] != '\0'; k++)
              { int v = ArgV[i][k];
                if ((all_flags[i][v>>5] & (1 << (v&0x1f))) == 0)
                  break;
              }
            if (ArgV[i][k] == '\0')
              continue;
          }
        fprintf(stderr,"%s: Can not recognize argument \"%s\"\n\n",Program,ArgV[i]);
        Print_Argument_Usage(stderr,no_escapes);
        exit (1);
      }
#ifdef DEBUG_MATCH
    printf("\nRequired Arg Range [%d,%d], actual = %d(%d)\n",rmin,rmax,nreq,oreq);
#endif
    if (nreq < rmin)
      { fprintf(stderr,"%s: Too few required arguments\n\n",Program);
        Print_Argument_Usage(stderr,no_escapes);
        exit (1);
      }
    if (oreq > rmax)
      { fprintf(stderr,"%s: Too many required arguments\n\n",Program);
        Print_Argument_Usage(stderr,no_escapes);
        exit (1);
      }
  }

  free(all_flags);
  free(has_a_match);
}


/*****************************************************************************************/
/*   FIND BEST MATCH TO COMMAND LINE                                                     */
/*****************************************************************************************/

#define UN_MATCHED 0
#define FULL_MATCH 1
#define PART_MATCH 2   //  A partial match til the end

  // Assignment of an arg to a unit while searching for a match

typedef struct
  { int              matched;   // Match state (one of 3 constants above)
    Unit            *inst;      // If matched, unit instance matched by this arg
    int              iter;      // Iteration of loop (if in one) for the match
    Alpha_Set        matflag;   // Flags matched so far (PART_MATCH only)
    Alpha_Set        argflag;   // All flags in the argument (starts with OPTION_CHAR only)
  } Match_State;

static int             Last_Req;     // Last required argument matched
static Match_State    *Match_Vector; // Match[1..ArgC] contains the current state of the match
static Unit          **Fstack;       // Fstack[0..Ftop-1] is the sequence of units traversed so far
static int             Ftop;

static int             Num_Matches;  // Number of equally specific matched found
static int             Num_Numbers;  // Number of numbers in the matches
static Match_State    *A_Match;      // A_Match[1..ArgC] is a best match found so far and ...
static Unit          **F_Path;       // F_Path[0..Flen-1] is the sequence of units giving the match
static int             Flen;

/*  We engage in a backtracking search through the automaton seeking a path that matches
      the command line.  What makes this tricky is that opional arguments and flags can
      be in any order with respect to their order on an accepting automaton path.  So
      we keep a vector of whether or not each argument has been matched, but allow option
      matches to occur in any order.
*/

static void match_instance(Unit *, Candidate *);
static void traverse_edge(State *);

// Traverse state s

static void traverse_state(State *s)
{ if ((s->flags & LOOP_START) != 0)
    s->loop->mark += 1;

  if (s->atom == NULL || s->atom->label == WHITE)
    traverse_edge(s);
  else
    { Candidate *mat;   // For all units that start at s, try all match possibilities
      Unit      *ins;

#ifdef DEBUG_SEARCH
      printf("Doing I @ %d(%d)\n",s->number,Last_Req);
#endif
      for (ins = (Unit *) (s->ulist); ins != NULL; ins = ins->inext)
        for (mat = ins->clist; mat != NULL; mat = mat->next)
          if (ins->kind == REQUIRED)
            { if (mat->argp > Last_Req && Match_Vector[mat->argp].matched == UN_MATCHED)
                match_instance(ins,mat);
            }
          else
            match_instance(ins,mat);
#ifdef DEBUG_SEARCH
      printf("Undoing I @ %d\n",s->number);
#endif
    }

  if ((s->flags & LOOP_START) != 0)
    s->loop->mark -= 1;
}

// Traverse edges out of state s

static void traverse_edge(State *s)
{
#ifdef DEBUG_SEARCH
  printf("Doing E @ %d\n",s->number);
#endif
  if (s->succ[0] == NULL)   //  s is the final state, check if have a match, and if so and
    { int i, k, nos;        //    it is best, then record it in A_Match/F_Path.

      nos = 0;
      for (i = 1; i < ArgC; i++)
        { if (Match_Vector[i].matched == UN_MATCHED)
            break;
          else if (Match_Vector[i].matched == FULL_MATCH)
            { Unit *u = Match_Vector[i].inst;
              if (u != NULL)
                { Atom *f = u->unit[0];
                  if (f->label == NAME && f->def_val.type <= DOUBLE_TYPE)
                    nos += 1;
                }
            }
          else  // Match_Vector[i].matched == PART_MATCH
            { for (k = 0; k < 4; k++)
                if (Match_Vector[i].matflag[k] != Match_Vector[i].argflag[k])
                  break;
              if (k < 4)
                break;
            }
        }
      if (i >= ArgC)
        { if (Num_Matches == 0 || nos > Num_Numbers)
            {
#ifdef DEBUG_SEARCH
              if (Num_Matches == 0)
                printf("First match found (%d)\n",nos);
              else
                printf("Better match found (%d)\n",nos);
#endif
              Num_Matches = 1;
              Num_Numbers = nos;
              for (i = 1; i < ArgC; i++)
                A_Match[i] = Match_Vector[i];
              for (i = 0; i < Ftop; i++)
                F_Path[i] = Fstack[i];
              Flen = Ftop;
            } 
          else if (nos == Num_Numbers)
            { for (i = 1; i < ArgC; i++)
                if (A_Match[i].matched == PART_MATCH)
                  { if (Match_Vector[i].matched != PART_MATCH)
                      break;
                  }
                else
                  { if (Match_Vector[i].inst != A_Match[i].inst)
                      break;
                  } 
              if (i < ArgC)
                { Num_Matches += 1;

#ifdef DEBUG_SEARCH
                  printf("Found additional match (%d)\n",nos);
#endif
                }
#ifdef DEBUG_SEARCH
              else
                printf("Found same match\n");
#endif
            }
#ifdef DEBUG_SEARCH
          else
            printf("Found match but inferior (%d)\n",nos);

         printf("\nA Match (%d,%d):\n",Num_Matches,Num_Numbers);
         for (i = 1; i < ArgC; i++)
           if (A_Match[i].matched == PART_MATCH)
             { printf(" %2d: Flags:",i);
               for (k = 0; k < 4; k++)
                 printf(" %08x",A_Match[i].matflag[k]);
               printf("\n");
             }
           else if (A_Match[i].inst != NULL)
             { printf(" %2d: ",i);
               printf(" [%d]",A_Match[i].iter);
               print_instance(stdout,A_Match[i].inst,0,0);
             }
#endif
        }
    }
  else
    { traverse_state(s->succ[0]);
      if (s->succ[1] != NULL)
        traverse_state(s->succ[1]);
    }
#ifdef DEBUG_SEARCH
  printf("Undoing E @ %d\n",s->number);
#endif
}

//  Match mat of unit ins if possible and if so continue search of edges out of end of the unit

static void match_instance(Unit *ins, Candidate *mat)
{ static int o, k;

  Fstack[Ftop] = ins;
  Ftop += 1;

  o = mat->argp;

  if (ins->kind == REQUIRED)
    { int arg_holder;

      arg_holder = Last_Req;
      Match_Vector[o].inst    = ins;
      Match_Vector[o].matched = FULL_MATCH;
      if (ins->beg->loop != NULL)
        Match_Vector[o].iter = ins->beg->loop->mark;
      else
        Match_Vector[o].iter = 1;
      Last_Req = o;

#ifdef DEBUG_SEARCH
      printf("D-Match %d to arg %d(%d)\n",ins->end->number,o,ins->span);
#endif
      traverse_edge(ins->end);
#ifdef DEBUG_SEARCH
      printf("D-Undo %d\n",ins->end->number);
#endif

      o = mat->argp;
      Last_Req = arg_holder;
      Match_Vector[o].matched = UN_MATCHED;
      Match_Vector[o].inst    = NULL;
    }

  else if (ins->kind == FLAGS || ins->kind == OFLAG)
    { if (Match_Vector[o].matched != FULL_MATCH)
        { int        mval;
          Alpha_Set  mset;
          Candidate *m;

          mval = Match_Vector[o].matched;
          for (k = 0; k < 4; k++)
            mset[k] = Match_Vector[o].matflag[k];
        
          Match_Vector[o].inst    = ins;
          Match_Vector[o].matched = PART_MATCH;
          for (k = 0; k < 4; k++)
            Match_Vector[o].matflag[k] |= mat->set[k];
          Match_Vector[o].iter = 1;

#ifdef DEBUG_SEARCH
          printf("F-Match %d to arg %d:",ins->end->number,o);
          for (k = 0; k < 4; k++)
            printf(" %08x",Match_Vector[o].matflag[k]);
          printf("\n");
#endif
          traverse_edge(ins->end);
          for (m = mat->next; m != NULL; m = m->next)
            match_instance(ins,m);    //  A flag unit can match more than one arg!
#ifdef DEBUG_SEARCH
          printf("F-Undo %d\n",ins->end->number);
#endif

          o = mat->argp;
          Match_Vector[o].matched = mval;
          for (k = 0; k < 4; k++)
            Match_Vector[o].matflag[k] = mset[k];
        }
    }

  else    // ins->kind == OPTION
    { for (k = 0; k <= ins->span; k++)
        if (Match_Vector[o+k].matched != UN_MATCHED)
          break;

      if (k > ins->span)
        { for (k = 0; k <= ins->span; k++)
            { Match_Vector[o+k].matched = FULL_MATCH;
              Match_Vector[o].inst = NULL;
            }
          Match_Vector[o].inst = ins;
          if (ins->beg->loop != NULL)
            Match_Vector[o].iter = ins->beg->loop->mark;
          else
            Match_Vector[o].iter = 1;

#ifdef DEBUG_SEARCH
          printf("O-Match %d to arg %d(%d)\n",ins->end->number,o,ins->span);
#endif
          traverse_edge(ins->end);
#ifdef DEBUG_SEARCH
          printf("O-Undo %d\n",ins->end->number);
#endif

          o = mat->argp;
          for (k = 0; k <= ins->span; k++)
            { Match_Vector[o+k].matched = UN_MATCHED;
              Match_Vector[o+k].inst    = NULL;
            }
        }
    }

  Ftop -= 1;
}

//  Find a match to the command line

static void find_a_match(Automaton *mach, int no_escapes)
{ int i, k, v;

  Num_Matches  = 0;
  Num_Numbers  = 0;
  Last_Req     = 0;
  A_Match      = (Match_State *) Guarded_Malloc(sizeof(Match_State)*ArgC,"Process_Arguments");
  F_Path       = (Unit **) Guarded_Malloc(sizeof(Unit *)*(ArgC+mach->longest),"Process_Arguments");
  Match_Vector = (Match_State *) Guarded_Malloc(sizeof(Match_State)*ArgC,"Process_Arguments");
  Fstack       = (Unit **) Guarded_Malloc(sizeof(Unit *)*(ArgC+mach->longest),"Process_Arguments");

#ifdef DEBUG_SEARCH
  printf("\nArg to Flags:\n");
#endif
  for (i = 1; i < ArgC; i++)
    { Match_Vector[i].matched = UN_MATCHED;
      for (k = 0; k < 4; k++)
        { Match_Vector[i].matflag[k] = 0;
          Match_Vector[i].argflag[k] = 0;
        }
      if (ArgV[i][0] == OPTION_CHAR)
        { for (k = 1; ArgV[i][k] != '\0'; k++)
            { v = ArgV[i][k];
              Match_Vector[i].argflag[v>>5] |= (1 << (v&0x1f));
            }
#ifdef DEBUG_SEARCH
          printf("  %s ->",ArgV[i]);
          for (k = 0; k < 4; k++)
            printf(" %08x",Match_Vector[i].argflag[k]);
          printf("\n");
#endif
        }
    }

#ifdef DEBUG_SEARCH
  printf("\nBacktrack Search:\n");
#endif

  Ftop = 0;
  traverse_state(mach->start);

#ifdef DEBUG_RESULT
  if (Num_Matches > 0)
    { printf("\nThe Match (%d,%d):\n",Num_Matches,Num_Numbers);
      for (i = 1; i < ArgC; i++)
        if (A_Match[i].matched == PART_MATCH)
          { printf(" %2d: Flags:",i);
            for (k = 0; k < 4; k++)
              printf(" %08x",A_Match[i].matflag[k]);
            printf("\n");
          }
        else if (A_Match[i].inst != NULL)
          { printf(" %2d: ",i);
            printf(" [%2d:",A_Match[i].iter);
            if (A_Match[i].inst->beg->loop == NULL)
              printf("NL] ");
            else
              printf("%2d] ",A_Match[i].inst->beg->loop->number);
            print_instance(stdout,A_Match[i].inst,0,0);
          }
      printf("\nThe Match Path\n");
      for (i = 0; i < Flen; i++)
        { printf(" %2d: ",i);
          print_instance(stdout,F_Path[i],0,0);
        }
    }
#endif

  free(Fstack);
  free(Match_Vector);

  if (Num_Matches == 0)
    { fprintf(stderr,"%s: Do not recognize command arguments\n\n",Program);
      Print_Argument_Usage(stderr,no_escapes);
      exit (1);
    }
  else if (Num_Matches > 1)
    fprintf(stderr,"\nWARNING: Command line can be matched in two or more ways.\n");
}


/*****************************************************************************************/
/*   CONVERT ARG-BASED MATCH INTO UNIT-BASED MATCH                                       */
/*****************************************************************************************/

static void store_match(int argc, char *argv[])
{ int    i, j, k, v;
  Value *vbase;
  char  *tbase;

  { int nvals, atot;   //  Allocate arrays to hold matched values and strings

    nvals = 0;
    atot  = 0;
    for (i = 1; i < argc; i++)
      { Unit *ins = A_Match[i].inst;
        if (ins != NULL)
          nvals += ins->nvals;
        atot += strlen(argv[i])+1;
      }
  
    Vtop = vbase = (Value *) Guarded_Malloc(sizeof(Value)*nvals,"Process_Arguments");
    Ttop = tbase = (char  *) Guarded_Malloc(atot,"Process_Arguments");
    Fill_Values = 1;
  }

  // Add each arg match to a linked list of the matching unit and fetch all values
  // Also record in the mark field of each loop start node, the # of times it was traversed

  for (k = 0; k < 4; k++)
    Flag_Vector[k] = 0;

  for (k = 0; k < Flen; k++)
    F_Path[k]->used = 2;

  for (i = 1; i < argc; i++)
    { Unit *ins = A_Match[i].inst;

      if (A_Match[i].matched == PART_MATCH)
        for (j = 1; (v = argv[i][j]) != '\0'; j++)
          Flag_Vector[v>>5] |= (1 << (v&0x1f));
      else if (ins != NULL)
        { Match *mat = (Match *) Guarded_Malloc(sizeof(Match),"Process_Arguments");
          mat->value = Vtop;
          if (ins->kind == REQUIRED)
            match_required_arg(ins,argv[i]);
          else
            match_option_suffix(ins,0,argv[i],argv+i);
          mat->iter  = A_Match[i].iter;
          mat->next  = ins->alist;
          ins->alist = mat;
        }
      if (A_Match[i].inst != NULL)
        { State *s = A_Match[i].inst->beg->loop;
          if (s != NULL && A_Match[i].iter > s->mark)
            s->mark = A_Match[i].iter;
        }
    }

  //  Every synonym of a used unit is used, mark them as such
  //  Set the repeat count for every unit in a loop

  for (i = 0; i < Num_of_Units; i++)
    { if (Unit_Table[i].beg->loop != NULL) 
        Unit_Table[i].maxi = Unit_Table[i].beg->loop->mark;
      else
        Unit_Table[i].maxi = 0;
      if (Unit_Table[i].used == 2)
        { j = i;
          do
            { Unit_Table[j].used = 1;
              j = Unit_Table[j].equiv;
            }
          while (j != i);
        }
    }

#ifdef DEBUG_RESULT
  { int k;

    printf("\nReported Match (%d,%d)\n\nFlags = -",Vtop-vbase,Ttop-tbase);
    for (i = 0; i < 128; i++)
      if ((Flag_Vector[i>>5] & (1 << (i&0x1f))) != 0)
        printf("%c",i);
    printf("\n\nUnit Final Matches\n");
    for (k = 0; k < Num_of_Units; k++)
      { Match *m;
        print_unit(stdout,Unit_Table+k);
        printf(": us=%d mx=%d\n",Unit_Table[k].used,Unit_Table[k].maxi);
        for (m = Unit_Table[k].alist; m != NULL; m = m->next)
          { printf("      %d:",m->iter);
            for (j = 0; j < Unit_Table[k].nvals; j++)
              { printf(" ");
                print_value(stdout,m->value + j);
              }
            printf("\n");
          }
      }
  }
#endif
}


/*****************************************************************************************/
/*   PROCESS_ARGUMENTS: TOP LEVEL                                                        */
/*****************************************************************************************/

void Process_Arguments(int argc, char *argv[], char *spec[], int no_escapes)
{ static int   First_Call = 1;
  Node        *root;
  Automaton   *mach;

  if (!First_Call)
    { fprintf(stderr,"Error in Process_Arguments:\n  %s\n",C2_ERROR);
      exit (0);
    }
  First_Call = 0;

  ArgC = argc;
  ArgV = argv;

  // Grab command name and make a copy of the spec

  { int   lines;
    char *n;
 
    n = argv[0] + (strlen(argv[0]) - 1);
    while (n > argv[0] && *n != '/')
      n -= 1;
    if (*n == '/') n += 1;
    Program = Guarded_Strdup(n,"Process_Arguments");

    lines = 0;
    while (spec[lines] != NULL)
      lines += 1;

    if (lines == 0)
      { Master_Spec = (char **) Guarded_Malloc(sizeof(char *)*2,"Process_Arguments");
        Master_Spec[1] = NULL;
        Master_Spec[0] = Guarded_Strdup("\n","Process_Arguments");
      }
    else
      { Master_Spec = (char **) Guarded_Malloc(sizeof(char *)*(lines+1),"Process_Arguments");
        Master_Spec[lines] = NULL;
        while (--lines >= 0)
          { int   len = strlen(spec[lines])+2;
            char *pad = (char*)Guarded_Malloc(sizeof(char)*len,"Process_Arguments");
            strcpy(pad,spec[lines]);
            pad[len-2] = '\n';
            pad[len-1] = '\0';
            Master_Spec[lines] = pad;
          }
      }
  }

  // Parse the Specification

  Scan       = Master_Spec[0];
  Specbeg    = Master_Spec[0];
  Specline   = 0;
  In_Curly   = 0;

  check_eol();
  if (*Scan == '\0')
    root = new_atom(WHITE,Scan,0,NULL,Specline,0);
  else
    root = or();

  if (*Scan != '\0')
    syntax_error(PT_ERROR);

#ifdef DEBUG_TREE
  print_tree(stdout,root);
#endif

  root = remove_empty_loops(root,0);
  if (root == NULL)
    root = new_atom(WHITE,Scan,0,NULL,Specline,0);

#ifdef DEBUG_TREE
  print_tree(stdout,root);
#endif

  // Convert it into a finite automaton and check semantics

  mach = build_automaton(root);

  compute_unit_lookaheads(mach->start);
  check_automaton_units(mach->final);
  compute_unit_finishes(mach->start);
  reset_marks(mach->start);

#ifdef DEBUG_FA
  print_automaton(stdout,mach);
#endif

  // Build tables of all unit instances (Unit_Table) and place instances in a list at their
  // begin state.  Find arguments that match (partially in the case of flag lists) each unit
  // instance.

  build_unit_table(mach);
  find_candidate_matches();

#ifdef DEBUG_FA
  printf("\nAutomaton with Unit Instances\n");
  print_automaton(stdout,mach);
#endif

  // First check that a match is at least possible, and then if so backtrack search for a
  //   path through the automaton that matches all units, result in global A_Match[1..ArgC]
  //   and F_Path[0..Ftop].

  is_match_possible(mach,root,no_escapes);
  find_a_match(mach,no_escapes);

  // Record matches into Unit_Table as final result

  store_match(argc,argv);

  free(F_Path);
  free(A_Match);
}


/*****************************************************************************************/
/*   RESULT ACCESS ROUTINES                                                              */
/*****************************************************************************************/

// Find unit whose name matches query.  Complete matches are preferred over prefix matches.
//   The match must be unique.  -1 is returned if the match is to a flag coded only in the
//   flag vector (FLAGS).

static int search_table(char *query)
{ int full;
  int c_hit, p_hit, fg_hit;
  int hits;
  int j, k;

  hits = -1;
  for (k = 0; k < Num_of_Units; k++)
    { Unit_Table[k].mlist = -1;
      Unit_Table[k].hit   =  0;
    }
  c_hit = p_hit = fg_hit = 0;

  for (k = 0; k < Num_of_Units; k++)
    { if (Unit_Table[k].kind == OPTION || Unit_Table[k].kind == OFLAG)
        { Atom **unit = Unit_Table[k].unit;
          int    ulen = Unit_Table[k].len;
          int    i, j, vj;
  
          vj = 0;
          for (i = 0; i < ulen; i++)
            { if (query[vj] == '\0') break;
              if (unit[i]->label == TYPE)
                { if (query[vj] != VALUE_CHAR) goto nomatch;
                  vj += 1;
                }
              else if (unit[i]->label == WHITE)
                { if (query[vj] != ' ') goto nomatch;
                  vj += 1;
                }
              else if (unit[i]->label != SYN_L &&
                       unit[i]->label != SYN_R   ) // unit[i]->label == TEXT|LIST|DASH 
                { char *name = unit[i]->name;
                  int   nlen = unit[i]->nlen;
      
                  for (j = 0; j < nlen; j++)
                    { if (query[vj] == '\0') break;
                      if (name[j] == ESCAPE_CHAR)
                        j += 1;
                      if (name[j] != query[vj]) goto nomatch;
                      vj += 1;
                    }
                } 
            }
          if (query[vj] != '\0') goto nomatch;
  
          if (Unit_Table[k].kind == OFLAG)
            fg_hit = 1;
  
          full = (i >= ulen);
        } 
      else if (Unit_Table[k].kind == REQUIRED)
        { char *name = Unit_Table[k].unit[0]->name;
    	  int   nlen = Unit_Table[k].unit[0]->nlen;
          int   qlen = strlen(query);
    
          if (nlen < qlen || strncmp(name,query,qlen))
            goto nomatch;
          full = (nlen == qlen);
        }
      else
        goto nomatch;

      // if a synonym has previous been matched and this entry also matches then
      //   ignore this match if the synonym match is full, otherwise void the
      //   synonym match.

      for (j = Unit_Table[k].equiv; j != k; j = Unit_Table[j].equiv)
        if (Unit_Table[j].hit)
          { if (Unit_Table[j].prefix)
              { Unit_Table[j].hit = 0;
#ifdef DEBUG_LOOKUP
                printf("Undoing hit %d\n",j);
#endif
                p_hit -= 1;
              }
            else
              goto nomatch;
          }

      // if the same unit has previously been hit and is used (matched) then
      //   ignore this hit, otherwise void the previous hit.

      for (j = Unit_Table[k].equal; j != k; j = Unit_Table[j].equal)
        if (Unit_Table[j].hit)
          { if (Unit_Table[j].used == 0)
              { Unit_Table[j].hit = 0;
#ifdef DEBUG_LOOKUP
                printf("Undoing hit %d\n",j);
#endif
                if (Unit_Table[j].prefix)
                  p_hit -= 1;
                else
                  c_hit -= 1;
              }
            else
              goto nomatch;
          }
     
      if (full)                      //  add hit to list
        { Unit_Table[k].prefix = 0;
          c_hit += 1;
        }
      else
        { Unit_Table[k].prefix = 1;
          p_hit += 1;
        }
#ifdef DEBUG_LOOKUP
      printf("Match to %d\n",k);
#endif
      Unit_Table[k].hit   = 1;
      Unit_Table[k].mlist = hits;
      hits                = k;
    nomatch:
      continue;
    }
  
  //  if a flag provides the hit, count it (but its obviously not on the list)

  if (!fg_hit && strlen(query) == 2 && query[0] == OPTION_CHAR)
    { k = query[1];
      if ((Flag_Space[k>>5] & (1 << (k&0x1f))) != 0)
        c_hit  += 1;
    }
#ifdef DEBUG_LOOKUP
  printf("Hits = %d %d %d\n",c_hit,p_hit,fg_hit);
#endif

  if (c_hit  > 1) goto ambig;
  if (c_hit == 1) goto uniq;
  if (p_hit  > 1) goto ambig;
  if (p_hit == 1) goto uniq;

  fprintf(stderr,"\nError in Process_Arguments:\n");
  fprintf(stderr,"  Could not find name %s in the specification\n",query);
  exit (1);

ambig:
  fprintf(stderr,"\nError in Process_Arguments:\n");
  fprintf(stderr,"  Name %s does not match a unique unit in the specification\n",query);
  exit (1);

uniq:
  if (c_hit > 0)
    while (hits >= 0 && Unit_Table[hits].prefix)
      hits = Unit_Table[hits].mlist;
  else
    while (hits >= 0 && Unit_Table[hits].hit == 0)
      hits = Unit_Table[hits].mlist;
#ifdef DEBUG_LOOKUP
  printf("hit = %d\n",hits);
#endif
  return (hits);
}

//  Find the match record for the it'th setting of non-scalar unit k
//    (being sure to search all synonyms)

static Match *find_iteration(int k, int it)
{ int    j;
  Match *m;

  j = k;
  do 
    { for (m = Unit_Table[j].alist; m != NULL; m = m->next)
        if (m->iter == it+1)
          return (m);
      j = Unit_Table[j].equiv;
    }
  while (j != k);
  return (NULL);
}

// Find value match record for (scalar) unit k (being sure to search all synonyms)

static Match *find_value(int k)
{ int    j;
  Match *m;

  j = k;
  do 
    { m = Unit_Table[j].alist;
      if (m != NULL)
        return (m);
      j = Unit_Table[j].equiv;
    }
  while (j != k);
  return (NULL);
}

// Return 0 if the named unit (or synonym) is unmatched or its match is not in a loop,
//     otherwise the number of times through the loop

int Get_Repeat_Count(char *name)
{ int n;

  n = search_table(name);
  if (n < 0)
    return (0);
  else
    return (Unit_Table[n].maxi);
}

// Return non-zero if the named unit was matched (in the k'th iteration if non-scalar)

int Is_Arg_Matched(char *name, ...)
{ va_list ap;
  int     k, n;

  n = search_table(name);
  if (n < 0 || Unit_Table[n].kind == OFLAG)
    { if (n >= 0 && find_value(n) != NULL)   // tricky: an OFLAG may be set in the vector
        return (1);                          //   or in a synonym that has a match record!
      else
        { k = name[1];
          return ((Flag_Vector[k>>5] & (1 << (k & 0x1f))) != 0);
        }
    }
  else if (Unit_Table[n].maxi > 0)
    { va_start(ap,name);
      k = va_arg(ap,int);
      va_end(ap);
      if (k > Unit_Table[n].maxi)
        { fprintf(stderr,"\nError in Process_Arguments:\n");
          fprintf(stderr,"  Loop containing %s is not iterated %d times\n",name,k);
          exit (1);
        }
      return (find_iteration(n,k) != NULL);
    }
  else
    return (Unit_Table[n].used);
}

// Return a pointer to the default value for the j'th value of unit n, or NULL if none

static Value *get_default(int n, int j)
{ int    len  = Unit_Table[n].len;
  Atom **unit = Unit_Table[n].unit;
  int    c, i;

  c = 0;
  for (i = 0; i < len; i++)       // count type templates in the atom array til get to the j'th
    { if (unit[i]->label == TYPE)
        { c += 1;
          if (c == j)
            { if (unit[i]->has_def)   //  and then return the default if its there
                return ( & (unit[i]->def_val));
               break;                 //  otherwise break and return NULL
            }
        }
    }
  return (NULL);
}

// Return a pointer to the value of n[k][j] (named 'name') ensuring that its type it 'type'

static Value *get_value(char *name, int n, int k, int j, int type)
{ static Value ival;
  Match       *mat;
  Value       *v;

#ifdef DEBUG_MATCH
  printf("  getting (%s)%d[%d][%d] of type %d\n",name,n,k,j,type);
#endif

  ival.type = INT_TYPE;
  if (n < 0 || Unit_Table[n].kind == OFLAG)
    { if (n >= 0 && find_value(n) != NULL)
        ival.value.intval = 1; 
      else
        { k = name[1];
          ival.value.intval = ((Flag_Vector[k>>5] & (1 << (k & 0x1f))) != 0);
        }
      v = &ival;
    }
  else if (Unit_Table[n].maxi > 0)
    { if (k > Unit_Table[n].maxi)
        { fprintf(stderr,"\nError in Process_Arguments:\n");
          fprintf(stderr,"  Loop containing %s is not iterated %d times\n",name,k);
          exit (1);
        }
      mat = find_iteration(n,k);
      if (mat == NULL)
        { v = get_default(n,j);
          if (v == NULL)
            { fprintf(stderr,"\nError in Process_Arguments:\n");
              fprintf(stderr,"  %s[%d]",name,k);
              if (Unit_Table[n].nvals > 1)
                fprintf(stderr,"[%d]",j);
              fprintf(stderr," does not have a value\n");
              exit (1);
            }
        }
      else
        v = mat->value + (j-1);
    }
  else
    { mat = find_value(n); 
      if (j == 0)
        { ival.value.intval = (mat != NULL);
          v = &ival;
        }
      else if (mat == NULL)
        { v = get_default(n,j);
          if (v == NULL)
            { fprintf(stderr,"\nError in Process_Arguments:\n");
              fprintf(stderr,"  %s[%d] does not have a value\n",name,j);
              exit (1);
            }
        }
      else
        v = mat->value + (j-1);
    }
  if (v->type != type)
    { fprintf(stderr,"\nError in Process_Arguments:\n");
      fprintf(stderr,"  %s",name);
      if (j > 0 && Unit_Table[n].nvals > 1)
        fprintf(stderr,"[%d]",j);
      fprintf(stderr," does not have type %s\n",Type_Strings[type]);
      exit (1);
    }
  return (v);
}

#define GET_PARAMS			\
  k = j = 0;				\
  n = search_table(name);		\
  va_start(ap,name);			\
  if (n >= 0)				\
    { if (Unit_Table[n].maxi > 0)	\
        k = va_arg(ap,int);		\
      else				\
        k = 0;				\
      if (Unit_Table[n].nvals > 1)	\
        j = va_arg(ap,int);		\
      else				\
        j = Unit_Table[n].nvals;	\
    }					\
  va_end(ap);

int Get_Int_Arg(char *name, ...)
{ int        n, k, j;
  va_list    ap;

  GET_PARAMS
  return (get_value(name,n,k,j,INT_TYPE)->value.intval);
}

double Get_Double_Arg(char *name, ...)
{ int        n, k, j;
  va_list    ap;

  GET_PARAMS
  return (get_value(name,n,k,j,DOUBLE_TYPE)->value.dblval);
}

char *Get_String_Arg(char *name, ...)
{ int        n, k, j;
  va_list    ap;

  GET_PARAMS
  return (get_value(name,n,k,j,STRING_TYPE)->value.strval);
}

// Print usage statement from the sepcification

void Print_Argument_Usage(FILE *file, int no_escapes)
{ int   i, idnt;
  char *s;

  idnt = fprintf(file,"Usage: %s",Program);
  if (Master_Spec[0] != NULL)
    { for (i = 0; (s = Master_Spec[i]) != NULL; i++)
        { if (i > 0)
            fprintf(file,"%*s",idnt,"");
          fputc(' ',file);
          while (*s != '\0')
            { if (*s == ESCAPE_CHAR && no_escapes)
                s += 1;
              fputc(*s++,file);
            }
        }
    }
  else
    fputc('\n',file);
}

char *Program_Name()
{ return (Program); }

#undef  SPEC1
#undef  SPEC2
#undef  SPEC3
#undef  OTHER

#ifdef SPEC1

static char *Spec1[] = { "[-{m|matrix} <file(PAM120)>] [-{t|thresh} <int(40)>]",
                         "       <query:file>  <target:string> ...",
                         NULL
                      };

int main(int argc, char *argv[])
{ int i;

  Process_Arguments(argc,argv,Spec1,0);

  printf("-matrix = %s\n",Get_String_Arg("-matrix @"));
  printf("-mat    = %s\n",Get_String_Arg("-mat"));
  printf("-m      = %s\n",Get_String_Arg("-m"));
  printf("-t      = %d\n",Get_Int_Arg("-t"));
  printf("query  = %s\n",Get_String_Arg("quer"));
  printf("|targ| = %d\n",Get_Repeat_Count("t"));
  for (i = 1; i <= Get_Repeat_Count("target"); i++)
    printf("t[%d] = %s\n",i,Get_String_Arg("t",i));

  fprintf(stderr,"\n");
  Print_Argument_Usage(stderr,0);
  exit (0);
}

#endif

#ifdef SPEC2

static char *Spec2[] = { "[-a['c[':<string>]]] <index:string> [ <list:file> | <beg:int> <end:int> ] ...",
                         NULL
                       };

int main(int argc, char *argv[])
{ int i;

  Process_Arguments(argc,argv,Spec2,0);

  printf("-a = %d\n",Get_Int_Arg("-a"));
  printf("is(-ac:@) = %d\n",Is_Arg_Matched("-ac:@"));
  if (Is_Arg_Matched("-ac:@"))
    printf("-ac:@ = %s\n",Get_String_Arg("-ac:"));
  printf("-ac = %d\n",Get_Int_Arg("-ac"));
  printf("|beg| = %d\n",Get_Repeat_Count("beg"));
  for (i = 1; i <= Get_Repeat_Count("beg"); i++)
    { printf("is(beg[%d]) = %d\n",i,Is_Arg_Matched("beg",i));
      if (Is_Arg_Matched("beg",i))
        printf("beg,end[%d] = %d,%d\n",i,Get_Int_Arg("beg",i),Get_Int_Arg("end",i));
      else
        printf("list[%d] = %s\n",i,Get_String_Arg("li",i));
    }

  fprintf(stderr,"\n");
  Print_Argument_Usage(stderr,0);
  exit (0);
}

#endif

#ifdef SPEC3

static char *Spec3[] = { "[!Ccus] <1:string> <2:string>   |",
                         "[!Ccu] -d <1:string>            |",
                         "[!Ccu] -s <1:string>            |",
                         "[!Ccu] -d -s <1:string> <2:string>",
                         NULL
                       };

int main(int argc, char *argv[])
{ int i;

  Process_Arguments(argc,argv,Spec3,0);

  printf("-c = %d\n",Get_Int_Arg("-c"));
  printf("is(-s) = %d\n",Is_Arg_Matched("-s"));
  printf("-d = %d\n",Get_Int_Arg("-d"));
  printf("1 = %s\n",Get_String_Arg("1"));
  printf("is(2) = %d\n",Is_Arg_Matched("2"));
  if (Is_Arg_Matched("2"))
    printf("  2 = %s\n",Get_String_Arg("2"));

  fprintf(stderr,"\n");
  Print_Argument_Usage(stderr,0);
  exit (0);
}

#endif

#ifdef OTHER

static char *Spec4[] = { "( -a| -b| -c)d",
                         "(xx <int(32)> |yy <double(0.3e-2)> <int>|zz(t <file> |ee[<int>] ))",
                         NULL
                       };

static char *Spec5[] = { "-a{b|[c]d|e'|e}t <int(3)> [ -color'(r<int>,b<int>,g<int>') ]", NULL };

static char *Spec6[] = { "{-a|-b|-c}xx{bo|tu[x]} [ -u{b[t]|ii}me <int> ] {-axx|-uu}tu", NULL };

static char *Spec7[] = { "[(-a(b|c))...] -xx ( <a:string> | <b:int> <c:int> ) ...", NULL };

static char *Spec8[] = { "([{-a|-b}] [{-c|-d}] | [{-e|-f}] | [{-e|-b}])", NULL };

static char *Spec9[] = { "[(-ab <int>)...] <u:int> ( <a:string> | <b:int> <c:int> ) ...", NULL };

static char *Spec10[] = { "( [<1:int>] [-ab <int>] )...", NULL };

int main(int argc, char *argv[])
{ int i;

  Process_Arguments(argc,argv,Spec10,1);

  Print_Argument_Usage(stderr,1);
  exit (0);
}

#endif
