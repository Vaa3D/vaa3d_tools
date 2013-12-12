//##########################################################################################
//                                                                                         #
//  Combine object manager and code generator macro translator                             #
//                                                                                         #
//  Author:  Gene Myers                                                                    #
//  Date  :  July 2011                                                                     #
//                                                                                         #
//  (c) July 26, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   #
//      Copyrighted as per the full copy in the associated 'README' file                   #
//                                                                                         #
//##########################################################################################
//
//   Expand MANAGER declaration lines.  These are required to have the syntax:
//
//   MANAGER' [-cprfk[iI][oO]] <class_names> <field_descriptor> *
//
//       <class_names> <- <visible_class_name>('('<hidden_class_name>')')?
//
//       <field_desriptor> <- <block_name>':'<wrapper_size_name>
//                          | <block_name>'!'<64bit_wrapper_size_name>
//                          | <field_name>'*'<sub_class>
//                          | <field_name>'@'<referenced_class>
//
//   Expands #GENERATE statements to ease the burden of programming for each of a set of
//     instances in a list, say the types of an array.  #GENERATE statements, and all other
//     #-statments (save for #WHEN) must occur on a line by themselves.  The statements are:
//
//   #LISTDEF @<var> = <item_1> [ ... <item_n> ]
//
//     A #LISTDEF statement defines a list-variable, @<var>, that denotes the list of items
//     after the =-sign, and that can be used in subsequent #GENERATE statements to specify
//     what they iterate over.  White space separates the items that may be any string that
//     does not contain whitespace.  A var on the other hand must be an alphanumberic string
//     The generality of item syntax is such that you must surround them by white space in
//     order for command and expression parsing to work.
//     
//   #GENERATE <var_1> [ , ... , <var_k> ] = <list_1>  [ , ... , <list_k> ]
//     <code>
//   #END
//
//         where   <list_j>  <- <item_1> [ ... <item_n> ] | @<var>
//
//     The <code> between the #GENERATE and its bracketing #END statement is repeated once
//     for each ordered k-tuple of items from the k lists in the header.  For the i'th iteration,
//     <var_j> will designate the i'th value in the list designated by <list_j>.  Obviously all
//     k lists must have the same length, say n, and the <code> is repeated n times in sequence.
//     Within the i'th copy of the code, occurences of '<'var_j'>' are replaced by the i'th item
//     in <list_j>, say it is item_ji.  Occurences of '<'lower_case(var_j)'>' are replaced by 
//     lower_case(item_ji) and occurences of '<'upper_case(var_j)'>' are replaced by
//     upper_case(item_ji).  If the upper_case(var) = var and/or lower_case(var) = var,
//     then item_ji is substituted.  Obviously when @<var> is designated as a list in the
//     header it is denoting the list that it was last defined to be assigned to in a #LISTDEF
//     statement.
//
//   #IF <predicate>
//      <code>
//   #ELSEIF <predicate>
//      <code>
//   ...
//   #ELSE
//      <code>
//   #END
//
//      The code for the first true predicate in the IF-chain is retained.  The predicate can
//      be any expression built up from &&, ||, <=, >=, <, >, !=, ==, +, -, *, /, %, integer
//      constants, and vars and items from enclosing #GENERATE statements and previously
//      defined #LISTDEFs.  Because of the arbitrary nature of items, operators should be
//      surrounded by white space.  The value of a var is the ordinal position of its current
//      item value in the list generating it, unless the item is an integer constant in which
//      case its value is that of the constant.  Similarly, the value of an item is its ordinal
//      position in its most recent listing, unless it is an integer constant in which case its
//      value is that of the constant.
//
//  <code>  #WHEN <predicate>
//
//      A convenience when only one line of code is conditional.  If the predicate is true
//      then <code> is retained, otherwise it is not.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef _MSC_VER

#pragma warning( disable:4996 )  // Turn off deprecation warnings on Windows

#define strdup _strdup

#endif

#undef DEBUG

//  OBJECT MANGER ROUTINES

  // Output the thread support required by the library

void prolog()
{ printf("#ifdef _MSC_VER\n");
  printf("\n");
  printf("#pragma warning( disable:4996 )\n"); // Turn off deprecation warnings on Windows
  printf("\n");
  printf("#ifndef __cplusplus\n");
  printf("#define inline __inline\n");
  printf("#endif\n");
  printf("\n");
  printf("#define WIN32_LEAN_AND_MEAN\n");
  printf("\n");
  printf("#include <windows.h>\n");
  printf("#include <stdlib.h>\n");
  printf("#include <stdio.h>\n");
  printf("#include <errno.h>\n");
  printf("\n");
  printf("//  WINDOW pthreads \"LIBRARY\"\n");
  printf("\n");
  printf("  //  Mutex macros\n");
  printf("\n");
  printf("typedef SRWLOCK pthread_mutex_t;\n");
  printf("\n");
  printf("#define PTHREAD_MUTEX_INITIALIZER RTL_SRWLOCK_INIT\n");
  printf("\n");
  printf("#define pthread_mutex_lock(m)  AcquireSRWLockExclusive(m)\n");
  printf("\n");
  printf("#define pthread_mutex_unlock(m) ReleaseSRWLockExclusive(m)\n");
  printf("\n");
  printf("  //  Condition variable macros\n");
  printf("\n");
  printf("typedef CONDITION_VARIABLE pthread_cond_t;\n");
  printf("\n");
  printf("#define PTHREAD_COND_INITIALIZER RTL_CONDITION_VARIABLE_INIT\n");
  printf("\n");
  printf("#define pthread_cond_signal(c) WakeConditionVariable(c)\n");
  printf("\n");
  printf("#define pthread_cond_broadcast(c) WakeAllConditionVariable(c)\n");
  printf("\n");
  printf("#define pthread_cond_wait(c,m) SleepConditionVariableSRW(c,m,INFINITE,0)\n");
  printf("\n");
  printf("  //  Simple thread support\n");
  printf("\n");
  printf("typedef struct\n");
  printf("  { HANDLE *handle;\n");
  printf("    void   *(*fct)(void *);\n");
  printf("    void   *arg;\n");
  printf("    void   *retval;\n");
  printf("    int     id;\n");
  printf("  } Mythread;\n");
  printf("\n");
  printf("typedef Mythread *pthread_t;\n");
  printf("\n");
  printf("static DWORD WINAPI MyStart(void *arg)\n");
  printf("{ Mythread *tv = (Mythread *) arg;\n");
  printf("\n");
  printf("  tv->retval = tv->fct(tv->arg);\n");
  printf("  return (0);\n");
  printf("}\n");
  printf("\n");
  printf("static int pthread_create(pthread_t *thread, void *attr,\n");
  printf("                          void *(*fct)(void *), void *arg)\n");
  printf("{ Mythread *tv;\n");
  printf("  if (attr != NULL)\n");
  printf("    { fprintf(stderr,\"Do not support thread attributes\\n\");\n");
  printf("      exit (1);\n");
  printf("    }\n");
  printf("  tv = (Mythread *) malloc(sizeof(Mythread));\n");
  printf("  if (tv == NULL)\n");
  printf("    { fprintf(stderr,\"pthread_create: Out of memory.\\n\");\n");
  printf("      exit (1);\n");
  printf("    };\n");
  printf("  tv->fct    = fct;\n");
  printf("  tv->arg    = arg;\n");
  printf("  tv->handle = CreateThread(NULL,0,MyStart,tv,0,&tv->id);\n");
  printf("  if (tv->handle == NULL)\n");
  printf("    return (EAGAIN);\n");
  printf("  else\n");
  printf("    return (0);\n");
  printf("}\n");
  printf("\n");
  printf("static int pthread_join(pthread_t t, void **ret)\n");
  printf("{ Mythread *tv = (Mythread *) t;\n");
  printf("\n");
  printf("  WaitForSingleObject(tv->handle,INFINITE);\n");
  printf("  if (ret != NULL)\n");
  printf("    *ret = tv->retval;\n");
  printf("\n");
  printf("  CloseHandle(tv->handle);\n");
  printf("  free(tv);\n");
  printf("  return (0);\n");
  printf("}\n");
  printf("\n");
  printf("typedef int pthread_id;\n");
  printf("\n");
  printf("static pthread_id pthread_tag()\n");
  printf("{ return (GetCurrentThreadId()); }\n");
  printf("\n");
  printf("static int pthread_is_this(pthread_id id)\n");
  printf("{ return (GetCurrentThreadId() == id); }\n");
  printf("\n");
  printf("#else   //  Small extension to pthreads!\n");
  printf("\n");
  printf("#include <pthread.h>\n");
  printf("\n");
  printf("typedef pthread_t pthread_id;\n");
  printf("\n");
  printf("#define pthread_tag() pthread_self()\n");
  printf("\n");
  printf("static inline int pthread_is_this(pthread_id id)\n");
  printf("{ return (pthread_equal(pthread_self(),id)); }\n");
  printf("\n");
  printf("#endif\n");
  printf("\n");
}

#define WRAPPER   0
#define WRAPPER64 1
#define SUBOBJECT 2
#define REFERENCE 3

void expand_class(int NF, char **item, int lineno)
{ int copy, pack, reset, freet, kill, read, write;
  int input, output;
  int base;

  int hidden;
  char *X, *Y;
  char *x, *y, *z;

  int    packable, hasblock;
  int   *type;
  char **field, **value, **fname;

  int    i, first;
  char  *j, *k;

  // determine flag settings

  copy = pack = reset = freet = kill = read = write = 0;
  input = output = 0;
  if (item[0][0] == '-')
    { if (strchr(item[0],'c') != NULL)
        copy = 1;
      if (strchr(item[0],'p') != NULL)
        pack = 1;
      if (strchr(item[0],'r') != NULL)
        reset = 1;
      if (strchr(item[0],'f') != NULL)
        freet = 1;
      if (strchr(item[0],'k') != NULL)
        kill = 1;
      if (strchr(item[0],'i') != NULL)
        input = read = 1;
      if (strchr(item[0],'o') != NULL)
        output = write = 1;
      if (strchr(item[0],'I') != NULL)
        input = 1;
      if (strchr(item[0],'O') != NULL)
        output = 1;
      base = 1;
    }
  else
    base = 0;

  //  setup class names

  if ((j = strchr(item[base],'(')) != NULL)
    { hidden = 1;
      X = j+1;
      if ((k = strchr(X,')')) == NULL)
        { fprintf(stderr,"Object Manager @ line %d: Class name left paren missing \"%s\"!\n",
                         lineno,item[base]);
          exit (1);
        }
      k[0] = '\0';
      Y = item[base];
      j[0] = '\0';
    } 
  else
    { hidden = 0;
      Y = item[base];
      X = Y;
    }
  x = strdup(X);
  y = strdup(Y);
  for (j = x; *j != '\0'; j++)
    *j = tolower(*j);
  for (j = y; *j != '\0'; j++)
    *j = tolower(*j);
  if (hidden)
    { z = (char *) malloc(strlen(X) + strlen(y) + 8);
      sprintf(z,"((%s *) %s)",X,y);
    }
  else
    z = strdup(y);

  //  interpret field information

  type  = (int *)   malloc(sizeof(int)*(NF+1));
  field = (char **) malloc(sizeof(char *)*(NF+1));
  value = (char **) malloc(sizeof(char *)*(NF+1));
  fname = (char **) malloc(sizeof(char *)*(NF+1));

  packable  = 0;
  hasblock  = 0;
  if (NF > base)
    { for (i = base+1; i <= NF; i++)
        { if ((j = strchr(item[i],':')) != NULL)
            type[i] = WRAPPER;
          else if ((j = strchr(item[i],'!')) != NULL)
            type[i] = WRAPPER64;
          else if ((j = strchr(item[i],'*')) != NULL)
            type[i] = SUBOBJECT;
          else if ((j = strchr(item[i],'@')) != NULL)
            type[i] = REFERENCE;
          else
            { fprintf(stderr,"Object Manager @ line %d: Illegal macro argument \"%s\"!\n",
                             lineno,item[i]);
              exit (1);
            }
          field[i] = item[i];
          j[0] = '\0';
          value[i] = j+1;
          fname[i] = strdup(field[i]);
          j = fname[i];
          for (k = fname[i]; *k != '\0'; k++)
            if (isalnum(*k))
              *j++ = *k;
            else if (j == fname[i] || j[-1] != '_')
              *j++ = '_';
          *j++ = '\0';
          if (type[i] <= SUBOBJECT)
            packable = 1;
          if (type[i] <= WRAPPER64)
            hasblock = 1;
        }
    }

  // generate container and free list declarations

  printf("\n");
  printf("typedef struct __%s\n",X);
  printf("  { struct __%s *next;\n",X);
  printf("    struct __%s *prev;\n",X);
  printf("    int%*srefcnt;\n", ((int) strlen(X))+8, "");
  for (i = base+1; i <= NF; i++)
    if (type[i] == WRAPPER) 
      printf("    int%*s%s;\n", ((int) strlen(X))+8, "", value[i]);
    else if (type[i] == WRAPPER64) 
      printf("    int64%*s%s;\n", ((int) strlen(X))+5, "", value[i]);
  printf("    %s           %s;\n",X,x);
  printf("  } _%s;\n",X);
  printf("\n");
  printf("static _%s *Free_%s_List = NULL;\n",X,X);
  printf("static _%s *Use_%s_List  = NULL;\n",X,X);
  printf("\n");
  printf("static pthread_mutex_t %s_Mutex = PTHREAD_MUTEX_INITIALIZER;\n",X);
  printf("\n");
  printf("static int %s_Offset = sizeof(_%s)-sizeof(%s);\n",X,X,X);
  printf("static int %s_Inuse  = 0;\n",X);

  // generate <X>_Refcount

  printf("\n");
  printf("int %s_Refcount(%s *%s)\n",Y,Y,y); 
  printf("{ _%s *object = (_%s *) (((char *) %s) - %s_Offset);\n",X,X,y,X);
  printf("  return (object->refcnt);\n");
  printf("}\n");

  // generate allocate_<x>_<field[i]> and sizeof_<x>_<field[i]>

  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      { printf("\n");
        if (type[i] == WRAPPER)
          printf("static inline int allocate_%s_%s(%s *%s, int %s, char *routine)\n",
                 x, fname[i], X, x, value[i]); 
        else // type[i] == WRAPPER64
          printf("static inline int allocate_%s_%s(%s *%s, int64 %s, char *routine)\n",
                 x, fname[i], X, x, value[i]); 
        printf("{ _%s *object = (_%s *) (((char *) %s) - %s_Offset);\n",X,X,x,X);
        printf("  if (object->%s < %s)\n", value[i], value[i]);
        printf("    { void *x = Guarded_Realloc(%s->%s,(size_t) %s,routine);\n",
               x, field[i], value[i]);
        printf("      if (x == NULL) return (1);\n");
        printf("      %s->%s = x;\n", x, field[i]);
        printf("      object->%s = %s;\n", value[i], value[i]);
        printf("    }\n");
        printf("  return (0);\n");
        printf("}\n");

        printf("\n");
        if (type[i] == WRAPPER)
          printf("static inline int sizeof_%s_%s(%s *%s)\n", x, fname[i], X, x); 
        else // type[i] == WRAPPER64
          printf("static inline int64 sizeof_%s_%s(%s *%s)\n", x, fname[i], X, x); 
        printf("{ _%s *object = (_%s *) (((char *) %s) - %s_Offset);\n", X, X, x, X);
        printf("  return (object->%s);\n", value[i]);
        printf("}\n");
      }

  // generate new_<x>

  printf("\n");
  printf("static inline void kill_%s(%s *%s);\n", x, X, x);
  printf("\n");
  printf("static inline %s *new_%s(", X, x);
  for (i = base+1; i <= NF; i++)
    if (type[i] == WRAPPER)
      printf("int %s, ", value[i]);
    else if (type[i] == WRAPPER64)
      printf("int64 %s, ", value[i]);
  printf("char *routine)\n");
  printf("{ _%s *object;\n", X);
  printf("  %s  *%s;\n", X, x);
  printf("\n");
  printf("  pthread_mutex_lock(&%s_Mutex);\n", X);
  printf("  if (Free_%s_List == NULL)\n", X);
  printf("    { object = (_%s *) Guarded_Realloc(NULL,sizeof(_%s),routine);\n", X, X);
  printf("      if (object == NULL) return (NULL);\n");
  printf("      %s = &(object->%s);\n", x, x);
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      { printf("      object->%s = 0;\n", value[i]);
        printf("      %s->%s = NULL;\n", x, field[i]);
      }
  printf("    }\n");
  printf("  else\n");
  printf("    { object = Free_%s_List;\n", X);
  printf("      Free_%s_List = object->next;\n", X);
  printf("      %s = &(object->%s);\n", x, x);
  printf("    }\n");
  printf("  %s_Inuse += 1;\n", X);
  printf("  object->refcnt = 1;\n");
  printf("  if (Use_%s_List != NULL)\n", X);
  printf("    Use_%s_List->prev = object;\n", X);
  printf("  object->next = Use_%s_List;\n", X);
  printf("  object->prev = NULL;\n");
  printf("  Use_%s_List = object;\n", X);
  printf("  pthread_mutex_unlock(&%s_Mutex);\n", X);
  for (i = base+1; i <= NF; i++)
    if (type[i] > WRAPPER64)
      printf("  %s->%s = NULL;\n", x, field[i]);
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      { printf("  if (allocate_%s_%s(%s,%s,routine))\n", x, fname[i], x, value[i]);
        printf("    { kill_%s(%s);\n", x, x);
        printf("      return (NULL);\n");
        printf("    }\n");
      }
  printf("  return (%s);\n", x);
  printf("}\n");

  // generate copy_<x> and Copy_<Y> if -c is not set

  printf("\n");
  printf("static inline %s *copy_%s(%s *%s)\n", X, x, X, x);
  printf("{ %s *copy = new_%s(", X, x);
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      printf("%s_%s(%s),", x, value[i], x); 
  printf("\"Copy_%s\");\n", Y);
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      printf("  void *_%s = copy->%s;\n", field[i], field[i]);
  printf("  *copy = *%s;\n", x);
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      { printf("  copy->%s = _%s;\n", field[i], field[i]);
        printf("  if (%s->%s != NULL)\n", x, field[i]);
        printf("    memcpy(copy->%s,%s->%s,(size_t) %s_%s(%s));\n",
               field[i], x, field[i], x, value[i], x);
      }
    else
      { printf("  if (%s->%s != NULL)\n", x, field[i]);
        if (type[i] == REFERENCE)
          printf("    Inc_%s(%s->%s);\n", value[i], x, field[i]);
        else
          printf("    copy->%s = Copy_%s(%s->%s);\n", field[i], value[i], x, field[i]);
      }
  printf("  return (copy);\n");
  printf("}\n");

  if (!copy)
    { printf("\n");
      printf("%s *Copy_%s(%s *%s)\n", Y, Y, Y, y);
      printf("{ return ((%s *) copy_%s(%s)); }\n", Y, x, z);
    }

  // generate pack_<x> and Pack_<Y> if the -p flag is not set

  if (packable)
    { printf("\n");
      printf("static inline int pack_%s(%s *%s)\n", x, X, x);
      if (hasblock)
        printf("{ _%s *object  = (_%s *) (((char *) %s) - %s_Offset);\n", X, X, x, X);
      else
        printf("{\n");
      for (i = base+1; i <= NF; i++)
        if (type[i] <= WRAPPER64)
          { printf("  if (object->%s > %s_%s(%s))\n", value[i], x, value[i], x);
            if (type[i] == WRAPPER)
              printf("    { int ns = %s_%s(%s);\n", x, value[i], x);
            else
              printf("    { int64 ns = %s_%s(%s);\n", x, value[i], x);
            printf("      if (ns != 0)\n");
            printf("        { void *x = Guarded_Realloc(%s->%s,(size_t) ns,\"Pack_%s\");\n",
                   x, field[i], X);
            printf("          if (x == NULL) return (1);\n");
            printf("          %s->%s = x;\n", x, field[i]);
            printf("        }\n");
            printf("      else\n");
            printf("        { free(%s->%s);\n", x, field[i]);
            printf("          %s->%s = NULL;\n", x, field[i]);
            printf("        }\n");
            printf("      object->%s = ns;\n", value[i]);
            printf("    }\n");
          }
        else if (type[i] == SUBOBJECT)
          { printf("  if (%s->%s != NULL)\n", x, field[i]);
            printf("    if (Pack_%s(%s->%s) == NULL) return (1);\n", value[i], x, field[i]);
          }
      printf("  return (0);\n");
      printf("}\n");
    }

  if (!pack)
    { printf("\n");
      printf("%s *Pack_%s(%s *%s)\n", Y, Y, Y, y);
      if (packable)
        { printf("{ if (pack_%s(%s)) return (NULL);\n", x, z);
          printf("  return (%s);\n", y);
          printf("}\n");
        }
      else
        printf("{ return (%s); }\n", y);
    }

  // generate <Y> *Inc_<Y>(<Y> *<y>)

  printf("\n");
  printf("%s *Inc_%s(%s *%s)\n", Y, Y, Y, y);
  printf("{ _%s *object  = (_%s *) (((char *) %s) - %s_Offset);\n", X, X, y, X);
  printf("  pthread_mutex_lock(&%s_Mutex);\n", X);
  printf("  object->refcnt += 1;\n");
  printf("  pthread_mutex_unlock(&%s_Mutex);\n", X);
  printf("  return (%s);\n", y);
  printf("}\n");

  // generate free_<x> and Free_<Y> if the -f flag is not set

  printf("\n");
  printf("static inline void free_%s(%s *%s)\n", x, X, x);
  printf("{ _%s *object  = (_%s *) (((char *) %s) - %s_Offset);\n", X, X, x, X);
  printf("  pthread_mutex_lock(&%s_Mutex);\n", X);
  printf("  if (--object->refcnt > 0)\n");
  printf("    { pthread_mutex_unlock(&%s_Mutex);\n", X);
  printf("      return;\n");
  printf("    }\n");
  printf("  if (object->refcnt < 0)\n");
  printf("    fprintf(stderr,\"Warning: Freeing previously released %s\\n\");\n", Y);
  printf("  if (object->prev != NULL)\n");
  printf("    object->prev->next = object->next;\n");
  printf("  else\n");
  printf("    Use_%s_List = object->next;\n", X);
  printf("  if (object->next != NULL)\n");
  printf("    object->next->prev = object->prev;\n");
  printf("  object->next = Free_%s_List;\n", X);
  printf("  Free_%s_List = object;\n", X);
  printf("  %s_Inuse -= 1;\n", X);
  printf("  pthread_mutex_unlock(&%s_Mutex);\n", X);
  for (i = NF; i > base; i--)
    if (type[i] > WRAPPER64)
      { printf("  if (%s->%s != NULL)\n", x, field[i]);
        printf("    { Free_%s(%s->%s);\n", value[i], x, field[i]);
        printf("      %s->%s = NULL;\n", x, field[i]);
        printf("    }\n");
      }
  printf("}\n");

  if (!freet)
    { printf("\n");
      printf("void Free_%s(%s *%s)\n", Y, Y, y);
      printf("{ free_%s(%s); }\n", x, z);
    }

  // generate kill_<x> and Kill_<Y> if the -k flag is not set

  printf("\n");
  printf("static inline void kill_%s(%s *%s)\n", x, X, x);
  printf("{ _%s *object  = (_%s *) (((char *) %s) - %s_Offset);\n", X, X, x, X);
  printf("  pthread_mutex_lock(&%s_Mutex);\n", X);
  printf("  if (--object->refcnt > 0)\n");
  printf("    { pthread_mutex_unlock(&%s_Mutex);\n", X);
  printf("      return;\n");
  printf("    }\n");
  printf("  if (object->refcnt < 0)\n");
  printf("    fprintf(stderr,\"Warning: Killing previously released %s\\n\");\n", Y);
  printf("  if (object->prev != NULL)\n");
  printf("    object->prev->next = object->next;\n");
  printf("  else\n");
  printf("    Use_%s_List = object->next;\n", X);
  printf("  if (object->next != NULL)\n");
  printf("    object->next->prev = object->prev;\n");
  printf("  %s_Inuse -= 1;\n", X);
  printf("  pthread_mutex_unlock(&%s_Mutex);\n", X);
  for (i = NF; i > base; i--)
    if (type[i] > WRAPPER64)
      { printf("  if (%s->%s != NULL)\n", x, field[i]);
        printf("    Kill_%s(%s->%s);\n", value[i], x, field[i]);
      }
    else
      { printf("  if (%s->%s != NULL)\n", x, field[i]);
        printf("    free(%s->%s);\n", x, field[i]);
      }
  printf("  free(((char *) %s) - %s_Offset);\n", x, X);
  printf("}\n");

  if (!kill)
    { printf("\n");
      printf("void Kill_%s(%s *%s)\n", Y, Y, y);
      printf("{ kill_%s(%s); }\n", x, z);
    }

  // generate reset_<x> and Reset_<Y> if the -r flag is not set

  printf("\n");
  printf("static inline void reset_%s()\n", x);
  printf("{ _%s *object;\n", X);
  if (hasblock)
    printf("  %s  *%s;\n", X, x);
  printf("  pthread_mutex_lock(&%s_Mutex);\n", X);
  printf("  while (Free_%s_List != NULL)\n", X);
  printf("    { object = Free_%s_List;\n", X);
  printf("      Free_%s_List = object->next;\n", X);

  first = 1;
  for (i = NF; i > base; i--)
    if (type[i] <= WRAPPER64)
      { if (first)
          { first = 0;
            printf("      %s = &(object->%s);\n", x, x);
          }
        printf("      if (%s->%s != NULL)\n", x, field[i]);
        printf("        free(%s->%s);\n", x, field[i]);
      }
  printf("      free(object);\n");
  printf("    }\n");
  printf("  pthread_mutex_unlock(&%s_Mutex);\n", X);
  printf("}\n");

  if (!reset)
    { printf("\n");
      printf("void Reset_%s()\n", Y);
      printf("{ reset_%s(); }\n", x);
    }

  // generate <Y>_Usage

  printf("\n");
  printf("int %s_Usage()\n", Y);
  printf("{ return (%s_Inuse); }\n", X);

  //generate Used_<Y>_List

  printf("\n");
  printf("void %s_List(void (*handler)(%s *))\n", Y, Y);
  printf("{ _%s *a, *b;\n", X); 
  printf("  for (a = Use_%s_List; a != NULL; a = b)\n", X);
  printf("    { b = a->next;\n");
  printf("      handler((%s *) &(a->%s));\n", Y, x);
  printf("    }\n");
  printf("}\n");

  // if -i or -I is set then generate read_<x> and if -i is not set then generate Read_<Y>

  if (input)
    { int has_subos, can_error;

      printf("\n");
      printf("static inline %s *read_%s(FILE *input)\n", X, x);
      printf("{ char name[%d];\n",((int) strlen(Y)));
      printf("  %s *obj;\n", X);
      has_subos = 0;
      can_error = 0;
      for (i = base+1; i <= NF; i++)
        if (type[i] <= WRAPPER64 || type[i] == SUBOBJECT)
          { can_error = 1;
            has_subos = 1;
          }
      if (has_subos)
        printf("  %s read;\n", X);

      printf("  fread(name,%d,1,input);\n", ((int) strlen(Y)));
      printf("  if (strncmp(name,\"%s\",%d) != 0)\n", Y, ((int) strlen(Y)));
      printf("    return (NULL);\n");
      printf("  obj = new_%s(", x);
      for (i = base+1; i <= NF; i++)
        if (type[i] <= WRAPPER64)
          printf("0,"); 
      printf("\"Read_%s\");\n", Y);
      printf("  if (obj == NULL) return (NULL);\n");

      if (has_subos)
        printf("  read = *obj;\n");

      printf("  if (fread(obj,sizeof(%s),1,input) == 0) goto error;\n", X);
      for (i = base+1; i <= NF; i++)
        if (type[i] <= WRAPPER64)
          { printf("  obj->%s = read.%s;\n", field[i], field[i]);
            printf("  if (%s_%s(obj) != 0)\n", x, value[i]);
            printf("    { if (allocate_%s_%s(obj,%s_%s(obj),\"Read_%s\")) goto error;\n",
                          x, field[i], x, value[i], Y);
            printf("      if (fread(obj->%s,(size_t) %s_%s(obj),1,input) == 0) goto error;\n",
                          field[i], x, value[i]);
            printf("    }\n");
          }
        else if (type[i] == SUBOBJECT)
          { printf("  if (read.%s != NULL)\n", field[i]);
            printf("    { obj->%s = Read_%s(input);\n", field[i], value[i]);
            printf("      if (obj->%s == NULL) goto error;\n", field[i]);
            printf("    }\n");
            printf("  else\n");
            printf("    obj->%s = NULL;\n", field[i]);
          }
        else
          printf("  obj->%s = NULL;\n", field[i]);
      printf("  return (obj);\n");

      if (can_error)
        { printf("\n");
          printf("error:\n");
          printf("  kill_%s(obj);\n", x);
          printf("  return (NULL);\n");
        }

      printf("}\n");
    
      if (!read)
        { printf("\n");
          printf("%s *Read_%s(FILE *input)\n", Y, Y);
          printf("{ return ((%s *) read_%s(input)); }\n", Y, x);
        }
    }

  // if -o or -O is set then generate write_<x> and if -o is not set then generate Write_<Y>

  if (output)
    { printf("\n");
      printf("static inline void write_%s(%s *%s, FILE *output)\n", x, X, x);
      printf("{ fwrite(\"%s\",%d,1,output);\n", Y, ((int) strlen(Y)));
      printf("  fwrite(%s,sizeof(%s),1,output);\n", x, X);
      for (i = base+1; i <= NF; i++)
        if (type[i] <= WRAPPER64)
          { printf("  if (%s_%s(%s) != 0)\n", x, value[i], x);
            printf("    fwrite(%s->%s,(size_t) %s_%s(%s),1,output);\n",
                   x, field[i], x, value[i], x);
          }
        else if (type[i] == SUBOBJECT)
          { printf("  if (%s->%s != NULL)\n", x, field[i]);
            printf("    Write_%s(%s->%s,output);\n", value[i], x, field[i]);
          }
      printf("}\n");

      if (!write)
        { printf("\n");
          printf("void Write_%s(%s *%s, FILE *output)\n", Y, Y, y);
          printf("{ write_%s(%s,output); }\n", x, z);
        }
    }

  for (i = base+1; i <= NF; i++)
    free(fname[i]);
  free(fname);
  free(value);
  free(field);
  free(type);
  free(z);
  free(y);
  free(x);
}

//  CODE GENERATOR DECLARATION AND UTILITY ROUTINES

#define GEN_LINE    0
#define IF_LINE     1
#define ELSEIF_LINE 2
#define ELSE_LINE   3
#define END_LINE    4

typedef struct
  { int     type;    //  Type of statement
    int     line;    //  Line number of statement
    int     src;     //  Block it starts
  } Directive;

typedef struct
  { int     type;    //  Type of statement
    int     line;    //  Line number of statement
    int     lbeg;    //  list[lbeg,lend-1] contains lines encompassed by this block
    int     lend;
    int     link;    //  index of next block at this nesting level
    char   *expr;    //  conditional expression (for IF & ELSEIF only)
    int     dbeg;    //  defs[dbeg,dend-1] contains the vars for a GENERATE statement
    int     dend;
  } Block;

typedef struct
  { char   *name;       //  variable name, @ if list defined
    int     vbeg;       //  vals[vbeg,vend-1] contains the values for the var (in order)
    int     vend;
  } ListDef;

static int        smax, bmax, lmax, dmax, vmax;
static int        stop, btop, ltop, dtop, vtop;
static int        dhgh, vhgh;                     //  high-water mark of @vars & values

static Directive *stack = NULL;   //  array of smax directives filled currently to stop
static Block     *block = NULL;   //  array of bmax blocks filled currently to btop
static char     **list  = NULL;   //  array of lmax code lines filled currently to ltop
static ListDef   *defs  = NULL;   //  array of dmax var defs filled currently to dtop
static char     **vals  = NULL;   //  array of vmax values filled currently to vtop

#define PUSH(T,L,S)					\
{ if (stop >= smax)					\
    { smax  = stop + 10;				\
      stack = realloc(stack,sizeof(Directive)*smax);	\
    }							\
  stack[stop].type = (T);				\
  stack[stop].line = (L);				\
  stack[stop].src  = (S);				\
  stop += 1;						\
}

#define ADDBLOCK(T,L,E)					\
{ if (btop >= bmax)					\
    { bmax  = (int) (1.2*btop + 30);			\
      block = realloc(block,sizeof(Block)*bmax);	\
    }							\
  block[btop].type = (T);				\
  block[btop].line = (L);				\
  block[btop].lbeg = ltop;				\
  block[btop].lend = ltop;				\
  block[btop].expr = (E);				\
  block[btop].link = -1;				\
  block[btop].dbeg = dtop;				\
  block[btop].dend = dtop;				\
  btop += 1;						\
}

#define ADDLINE()					\
{ if (ltop >= lmax)					\
    { lmax = (int) (1.2*ltop + 500);			\
      list = realloc(list,sizeof(char *)*lmax);		\
    }							\
  list[ltop++] = strdup(buffer);			\
  block[btop-1].lend = ltop;				\
}

#define ADDDEF(N)					\
{ if (dtop >= dmax)					\
    { dmax = (int) (1.2*dtop + 30);			\
      defs = realloc(defs,sizeof(ListDef)*dmax);	\
    }							\
  defs[dtop].name = strdup(N);				\
  defs[dtop].vbeg = vtop;				\
  defs[dtop].vend = vtop;				\
  dtop += 1;						\
}

#define ADDVAL(V)					\
{ char *e;						\
  if (vtop >= vmax)					\
    { vmax = (int) (1.2*vtop + 50);			\
      vals = realloc(vals,sizeof(char *)*vmax);		\
    }							\
  vals[vtop++] = strdup(V);				\
  defs[dtop-1].vend = vtop;				\
  strtol((V),&e,10);					\
}

static int findname(char *s)   //  Find name in the LISTDEF portion of defs table
{ int d;
  for (d = dhgh-1; d >= 0; d--)
    if (strcmp(defs[d].name,s) == 0)
      return (d);
  return (-1);
}

static char *word(char **start)  //  Isolate a white-space surrounded 'item'
{ char *s, *t;

  s = *start;
  while (isspace(*s))
    s += 1;
  if (*s == '\0')
    { *start = s;
      return (NULL);
    }
  t = s++;
  while (*s != '\0' && !isspace(*s))
    s += 1;
  if (*s != '\0')
    *s++ = '\0';
  *start = s;
  return (t);
}

static char *TypeName[] = { "GEN", "IF", "ELIF", "ELSE", "END" };

void print_state()
{ int i, j, k;

  printf("\nExpander: %d %d %d[%d] %d[%d]\n\n",btop,ltop,dtop,dhgh,vtop,vhgh);

  for (i = 0; i < btop; i++)
    { printf("  %3d: %s @%d [%d,%d] [%d,%d] :%d:",
             i,TypeName[block[i].type],block[i].link,
             block[i].lbeg,block[i].lend,block[i].dbeg,block[i].dend,block[i].line);
      if (block[i].expr != NULL)
        printf(" '%s'",block[i].expr);
      printf("\n");
      for (j = block[i].dbeg; j < block[i].dend; j++)
        { printf("    '%s':",defs[j].name);
          for (k = defs[j].vbeg; k < defs[j].vend; k++)
            printf(" '%s'",vals[k]);
          printf("\n");
        }
      for (j = block[i].lbeg; j < block[i].lend; j++)
        printf("    '%s'\n",list[j]);
    }

  printf("\n  @vars:\n");
  for (j = 0; j < dhgh; j++)
    { printf("    '%s':",defs[j].name);
      for (k = defs[j].vbeg; k < defs[j].vend; k++)
        printf(" '%s'",vals[k]);
      printf("\n");
    }
}

static void setup_maps();
static int code_gen(int);


//  MAIN ROUTINE AND LINE PARSER

int main(int argc, char *argv[])
{ char   buffer[50001];
  int    lineno;

  int    imax;
  char **items;

  char  *s, *k, *j;

  (void) argc;
  (void) argv;

  lineno = 0;

  prolog();

  imax  = 0;
  items = NULL;

  smax = bmax = lmax = dmax = vmax = 0;
  stop = btop = ltop = dtop = vtop = 0;
  dhgh = vhgh = 0;

  setup_maps();

  while (fgets(buffer,5000,stdin) != NULL)
    { lineno += 1;
      if (buffer[strlen(buffer)-1] != '\n')
        { fprintf(stderr,"%4d: Line longer than 5000 chars!\n",lineno);
          exit (1);
        }
      buffer[strlen(buffer)-1] = '\0';

      s = buffer;
      while (isspace(*s))
        s += 1;

      if (strncmp(buffer,"MANAGER",7) == 0 && isspace(buffer[7]))
        { int n;

          if (stop > 0)
            { fprintf(stderr,"%4d: #MANAGER statement is nested in a #GENERATE block\n",lineno);
              exit (1);
            }
          s = buffer+7;
          n = 0;
          while (*s != '\0')
            { while (isspace(*s))
                s += 1;
              if (*s != '\0')
                { n += 1;
                  while (*s != '\0' && !isspace(*s))
                    s += 1;
                }
            }
          if (n > imax)
            { imax = n+5;
              items = (char **) realloc(items,sizeof(char *)*imax);
            }
          s = buffer+7;
          n = 0;
          while (*s != '\0')
            { while (isspace(*s))
                s += 1;
              if (*s != '\0')
                { items[n++] = s;
                  while (*s != '\0' && !isspace(*s))
                    s += 1;
                  if (isspace(*s))
                    *s++ = '\0';
                }
            }

          expand_class(n-1,items,lineno);
        }

      else if ((k = strstr(buffer,"#GENERATE")) != NULL)
        { if (k != s)
            { fprintf(stderr,"%4d: #GENERATE must be on a line by itself\n",lineno);
              exit (1);
            }

          PUSH(GEN_LINE,lineno,btop)

          ADDBLOCK(GEN_LINE,lineno,NULL)

          k += 8;
          if ((j = strchr(k,'=')) == NULL)
            { fprintf(stderr,"%4d: #GENERATE does not contain an =-sign\n",lineno);
              exit (1);
            }
	  *j = '\0';

          block[btop-1].dbeg = dtop;
          block[btop-1].dend = dtop;

          { int   n = 0;
            char *e = j+1 + strlen(j+1);
            char *f = j;
            int   d, v;

            while (k < f)
              { k += 1;
                if ((s = strchr(k,',')) != NULL)
                  *s = '\0';
                if ((s = word(&k)) == NULL)
                  { fprintf(stderr,"%4d: Missing gen var before comma or equal\n",lineno);
                    exit (1);
                  }
                ADDDEF(s)
                block[btop-1].dend = dtop;
                if ((s = word(&k)) != NULL)
                  { fprintf(stderr,"%4d: Gen vars not separated by comma\n",lineno);
                    exit (1);
                  }

                if (j >= e)
                  { fprintf(stderr,"%4d: #GENERATE var %s not defined\n",lineno,s);
                    exit (1);
                  }

                j += 1;
                if ((s = strchr(j,',')) != NULL && isspace(s[-1]) && isspace(s[1]))
                  *s = '\0';
                if ((s = word(&j)) == NULL)
                  { fprintf(stderr,"%4d: Empty value list?\n",lineno);
                    exit (1);
                  }
                if (s[0] == '@')
                  { if ((d = findname(s)) < 0)
                      { fprintf(stderr,"%4d: List var %s is not defined\n",lineno,s);
                        exit (1);
                      }
                    for (v = defs[d].vbeg; v < defs[d].vend; v++)
                      ADDVAL(vals[v])
                    if (word(&j) != NULL)
                      { fprintf(stderr,"%4d: List var in value list?\n",lineno);
                        exit (1);
                      }
                  }
                else
                  { ADDVAL(s)
                    while ((s = word(&j)) != NULL)
                      ADDVAL(s)
                  }

                if (n == 0)
                  n = defs[dtop-1].vend - defs[dtop-1].vbeg;
                else if (n != defs[dtop-1].vend - defs[dtop-1].vbeg)
                  { fprintf(stderr,"%4d: #GENERATE lists are not of same length\n",lineno);
                    exit (1);
                  }
              }
            if (j < e)
              { fprintf(stderr,"%4d: #GENERATE has extra value list(s)\n",lineno);
                exit (1);
              }
          }
        }

      else if ((k = strstr(buffer,"#IF")) != NULL)
        { if (k != s)
            { fprintf(stderr,"%4d: #IF must be on a line by itself\n",lineno);
              exit (1);
            }
          if (stop <= 0)
            { fprintf(stderr,"%4d: #IF is not nested withing a #GENERATE block\n",lineno);
              exit (1);
            }

          PUSH(IF_LINE,lineno,btop)

          ADDBLOCK(IF_LINE,lineno,strdup(k+3))
        }

      else if ((k = strstr(buffer,"#ELSEIF")) != NULL)
        { if (k != s)
            { fprintf(stderr,"%4d: #ELSEIF must be on a line by itself\n",lineno);
              exit (1);
            }
          if (stop <= 0)
            { fprintf(stderr,"%4d: #ELSEIF not proceeded by #IF or another #ELSEIF\n",lineno);
              exit (1);
            }
          if (stack[stop-1].type != IF_LINE && stack[stop-1].type != ELSEIF_LINE)
            { fprintf(stderr,"%4d: #ELSEIF not proceeded by #IF or another #ELSEIF\n",lineno);
              exit (1);
            }

          stop -= 1;
          block[stack[stop].src].link = btop;

          PUSH(ELSEIF_LINE,lineno,btop)

          ADDBLOCK(ELSEIF_LINE,lineno,strdup(k+7))
        }

      else if ((k = strstr(buffer,"#ELSE")) != NULL)
        { if (k != s)
            { fprintf(stderr,"%4d: #ELSE must be on a line by itself\n",lineno);
              exit (1);
            }
          if (stop <= 0)
            { fprintf(stderr,"%4d: #ELSE not proceeded by #IF or an #ELSEIF\n",lineno);
              exit (1);
            }
          if (stack[stop-1].type != IF_LINE && stack[stop-1].type != ELSEIF_LINE)
            { fprintf(stderr,"%4d: #ELSE not proceeded by #IF or an #ELSEIF\n",lineno);
              exit (1);
            }

          stop -= 1;
          block[stack[stop].src].link = btop;

          PUSH(ELSE_LINE,lineno,btop)

          ADDBLOCK(ELSE_LINE,lineno,NULL)
        }

      else if ((k = strstr(buffer,"#END")) != NULL)
        { if (k != s)
            { fprintf(stderr,"%4d: #END must be on a line by itself\n",lineno);
              exit (1);
            }
          if (stop <= 0)
            { fprintf(stderr,"%4d: #END has no starting #GENERATE or cascading #IF\n",lineno);
              exit (1);
            }

          stop -= 1;
          block[stack[stop].src].link = btop;

          ADDBLOCK(END_LINE,lineno,NULL)

          if (stop == 0)
            { int i;

#ifdef DEBUG
              print_state();
#endif
              code_gen(0);

              for (i = 0; i < ltop; i++)
                free(list[i]);
              for (i = 0; i < btop; i++)
                free(block[i].expr);
              for (i = dhgh; i < dtop; i++)
                free(defs[i].name);
              for (i = vhgh; i < vtop; i++)
                free(vals[i]);
              ltop = 0;
              btop = 0;
              dtop = dhgh;
              vtop = vhgh;
            }
        }

      else if ((k = strstr(buffer,"#LISTDEF")) != NULL)
        { if (k != s)
            { fprintf(stderr,"%4d: #LISTDEF must be on a line by itself\n",lineno);
              exit (1);
            }
          if (stop > 0)
            { fprintf(stderr,"%4d: #LISTDEF statement is nested in a #GENERATE block\n",lineno);
              exit (1);
            }

          k += 8;
          if ((j = strchr(k,'=')) == NULL)
            { fprintf(stderr,"%4d: #LISTDEF does not contain an =-sign\n",lineno);
              exit (1);
            }
          *j++ = '\0';

          if ((s = word(&k)) == NULL)
            { fprintf(stderr,"%4d: #LISTDEF var before =-sign is missing\n",lineno);
              exit (1);
            }
          if (s[0] != '@')
            { fprintf(stderr,"%4d: #LISTDEF var must start with an @-sign\n",lineno);
              exit (1);
            }
          ADDDEF(s)
          dhgh = dtop;
          
          while ((s = word(&j)) != NULL)
            ADDVAL(s);
          vhgh = vtop;
        }

      else if ((k = strstr(buffer,"#WHEN")) != NULL)
        { if (stop <= 0)
            { fprintf(stderr,"%4d: #WHEN is not nested withing a #GENERATE block\n",lineno);
              exit (1);
            }

          ADDBLOCK(IF_LINE,lineno,strdup(k+5))

          *k = '\0';
          ADDLINE()

          block[btop-1].link = btop;

          ADDBLOCK(END_LINE,lineno,NULL)
        }

      else if (stop > 0)
        { ADDLINE() }

      else
        printf("%s\n",buffer);
    }

  if (stop > 0)
    { fprintf(stderr,"%4d: End of file without closing #END, unclosed at line %d\n",
                     lineno,stack[stop-1].line);
      exit (1);
    }

  exit (0);
}

//  PREDICATE EVALUTION PROLOG

static char *parse;
static int   pline;

static void skip_white()
{ while (isspace(*parse))
    parse += 1;
}

static int eval_predicate();

static int evaluate_predicate(char *s, int lineno)
{ int val;

  pline = lineno;
  parse = s;
  skip_white();
  val = eval_predicate();
  if (*parse != '\0')
    { fprintf(stderr,"%4d: Expression terminates prematurely, remainder is '%s'\n",pline,parse);
      exit (1);
    }
#ifdef DEBUG
  printf("[[E%d]]",val);
#endif
  return (val);
}

//  CODE GENERATION

static char idmap[128], upmap[128], lomap[128];

static char *maps[3] = { idmap, upmap, lomap };

static void setup_maps()
{ int i;

  for (i = 0; i < 128; i++)
    idmap[i] = upmap[i] = lomap[i] = i;
  for (i = 0; i < 26; i++)
    { upmap['a'+i] = 'A'+i;
      lomap['A'+i] = 'a'+i;
    }
}

static char *matchname(char *text, char *name, char *map, int tchar)
{ char *s, *t;

  t = text;
  for (s = name; *s != '\0'; s++)
    { if (*t != map[(int) (*s)])
        break;
      if (*t++ == '\0')
        break;
    }
  if (*s == '\0' && *t == tchar)
    return (t+1);
  return (NULL);
}

static void expand_line(char *line)
{ char *s, *t, *v;
  int   i, b, d, c;
  char *m;

  s = line;
  while (1)
    { t = strchr(s,'<');
      if (t == NULL)
        { printf("%s\n",s);
          break;
        }
      *t = '\0';
      printf("%s",s);
      *t++ = '<';
      for (i = stop-1; i >= 0; i--)
        { b = stack[i].src;
          for (d = block[b].dbeg; d < block[b].dend; d++)
            { v = defs[d].name;
              for (c = 0; c < 3; c++)
                if ((s = matchname(t,v,maps[c],'>')) != NULL)
                  break;
              if (c < 3)
                break;
            }
          if (d < block[b].dend)
            break;
        }
      if (i >= 0)
        { v = vals[defs[d].vbeg + stack[i].line]; 
#ifdef DEBUG
          printf("[[%d,%d,%d]]",c,i,d);
#endif
          m = maps[c];
          for (t = v; *t != '\0'; t++)
            putchar(m[(int) (*t)]);
        }
      else
        { putchar('<');
#ifdef DEBUG
          printf("[[N]]");
#endif
          s = t; 
        }
    }
}

static void code_block(int beg, int end)
{ int i, j;

  i = beg;
  while (i < end)
    { for (j = block[i].lbeg; j < block[i].lend; j++)
        expand_line(list[j]);
      i += 1;
      if (i < end)
        i = code_gen(i);
    }
}

static int code_gen(int beg)
{ int  end;
  int  i, n;

  if (block[beg].type == GEN_LINE)
    { end = block[beg].link;
      n = block[beg].dbeg;
      n = defs[n].vend - defs[n].vbeg;
      for (i = 0; i < n; i++)
        { PUSH(GEN_LINE,i,beg);
          code_block(beg,end);
          stop -= 1;
        }
    }

  else   // block[beg].type == IF_LINE
    { end = beg;
      n   = 1;
      while (block[end].type != END_LINE)
        { if (n && (block[end].type == ELSE_LINE ||
                    evaluate_predicate(block[end].expr,block[end].line)))
            { n = 0;
              code_block(end,block[end].link);
            }
          end = block[end].link;
        }
    }
  return (end);
}

//  PREDICATE EVALUATION

static int eval_atom()
{ int   val;

  if (*parse == '(')
    { parse += 1;
      skip_white();
      val = eval_predicate();
      if (*parse != ')')
        { fprintf(stderr,"%4d: Expression does not have balanced parens\n",pline);
          exit (1);
        }
      skip_white();
      parse += 1;
      return (val);
    }
  else
    { int   i, b, d, v, c;
      char *s, *t;

      if ((s = word(&parse)) == NULL)
        { fprintf(stderr,"%4d: Premature end of expression\n",pline);
          exit (1);
        }
#ifdef DEBUG
      printf("[[L'%s':",s);
#endif
      if (parse[-1] == '\0')
        c = parse[-1] = ' ';
      else
        c = '\0';
      skip_white();
      val = strtol(s,&t,10);
      if (*t == c)
        {
#ifdef DEBUG
          printf("I%d]]",val);
#endif
          return (val);
        }
  
      for (i = stop-1; i >= 0; i--)
        { b = stack[i].src;
          for (d = block[b].dbeg; d < block[b].dend; d++)
            if (matchname(s,defs[d].name,maps[0],c) != NULL)
              break;
          if (d < block[b].dend)
            break;
        }
      if (i >= 0)
        { s = vals[defs[d].vbeg + stack[i].line];
          val = strtol(s,&t,10);
          if (*t == '\0')
            {
#ifdef DEBUG
              printf("H%d(%d)]]",val,i);
#endif
              return (val);
            }
          else
            {
#ifdef DEBUG
              printf("G%d(%d)]]",stack[i].line+1,i);
#endif
              return (stack[i].line+1);
            }
        }

      for (i = stop-1; i >= 0; i--)
        { b = stack[i].src;
          for (d = block[b].dbeg; d < block[b].dend; d++)
            { for (v = defs[d].vend-1; v >= defs[d].vbeg; v--)
                if (matchname(s,vals[v],maps[0],c) != NULL)
                  break;
              if (v >= defs[d].vbeg)
                break;
            }
          if (d < block[b].dend)
            break;
        }
      if (i >= 0)
        {
#ifdef DEBUG
          printf("V%d(%d)]]",(v-defs[d].vbeg)+1,d);
#endif
          return ((v-defs[d].vbeg)+1);
        }

      for (d = dhgh-1; d >= 0; d--)
        { for (v = defs[d].vend-1; v >= defs[d].vbeg; v--)
            if (matchname(s,vals[v],maps[0],c) != NULL)
              break;
          if (v >= defs[d].vbeg)
            break;
        }
      if (d >= 0)
        {
#ifdef DEBUG
          printf("L%d(%d)]]",(v-defs[d].vbeg)+1,d);
#endif
          return ((v-defs[d].vbeg)+1);
        }

      fprintf(stderr,"%4d: Expression item '%s' is not defined\n",pline,s);
      exit (1);
    }
}

int eval_fact()
{ int val;

  val = eval_atom();
  while (*parse == '*' || *parse == '/' || *parse == '%')
    { if (*parse++ == '*')
        { skip_white();
          val = val * eval_atom();
        }
      else if (*parse++ == '/')
        { skip_white();
          val = val / eval_atom();
        }
      else
        { skip_white();
          val = val % eval_atom();
        }
      skip_white();
    }
  return (val);
}

int eval_sign()
{ int sgn;

  sgn = 1;
  while (*parse == '+' || *parse == '-')
    { if (*parse++ == '-')
        sgn *= -1;
      skip_white();
    }
  return (sgn * eval_fact());
}

int eval_term()
{ int val;

  val = eval_sign();
  while (*parse == '+' || *parse == '-')
    { if (*parse++ == '+')
        { skip_white();
          val = val + eval_sign();
        }
      else
        { skip_white();
          val = val - eval_sign();
        }
    }
  return (val);
}

int eval_compare()
{ int val;

  val = eval_term();
  while (1)
    { if (strncmp(parse,"<=",2) == 0)
        { parse += 2;
          skip_white();
          val = (val <= eval_term());
        }
      else if (strncmp(parse,">=",2) == 0)
        { parse += 2;
          skip_white();
          val = (val >= eval_term());
        }
      else if (strncmp(parse,"==",2) == 0)
        { parse += 2;
          skip_white();
          val = (val == eval_term());
        }
      else if (strncmp(parse,"!=",2) == 0)
        { parse += 2;
          skip_white();
          val = (val != eval_term());
        }
      else if (*parse == '<')
        { parse += 1;
          skip_white();
          val = (val < eval_term());
        }
      else if (*parse == '>')
        { parse += 1;
          skip_white();
          val = (val > eval_term());
        }
      else
        break;
    }
  return (val);
}

int eval_negate()
{ int neg;

  neg = 0;
  while (*parse == '!')
    { parse += 1;
      skip_white();
      neg = 1-neg;
    }
  if (neg)
    return (1 - eval_compare());
  else
    return (eval_compare());
}

int eval_clause()
{ int val;

  val = eval_negate();
  while (strncmp(parse,"&&",2) == 0)
    { parse += 2;
      skip_white();
      if ( ! eval_negate())
        val = 0;
      skip_white();
    }
  return (val);
}

int eval_predicate()
{ int val;

  val = eval_clause();
  while (strncmp(parse,"||",2) == 0)
    { parse += 2;
      skip_white();
      if (eval_clause())
        val = 1;
    }
  return (val);
}
