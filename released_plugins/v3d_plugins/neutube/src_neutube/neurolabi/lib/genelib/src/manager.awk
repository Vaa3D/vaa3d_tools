#   Expand MANAGER declaration lines.  These are required to have the syntax:
#
#    'MANAGER' <class_names> <field_descriptor> *
#
#       <class_names> <- <visible_class_name>('('<hidden_class_name>')')?
#
#       <field_desriptor> <- <field_name>':'<wrapper_size_name>
#                          | <field_name>'^'<packable_sub_class>
#                          | <field_name>'*'<unpackable_sub_class>

BEGIN { FS = " +"; }

/^MANAGER/ {

# determine flag settings

  copy = pack = reset = free = kill = 0;
  if (index($2,"-") == 1)
    { if (index($2,"c") != 0)
        copy = 1;
      if (index($2,"p") != 0)
        pack = 1;
      if (index($2,"r") != 0)
        reset = 1;
      if (index($2,"f") != 0)
        free = 1;
      if (index($2,"k") != 0)
        kill = 1;
      base = 3;
    }
  else
    base = 2;

# setup class names

  if ((j = index($base,"(")) != 0)
    { hidden = 1;
      X = substr($base,j+1,length($base)-(j+1));
      Y = substr($base,1,j-1);
    } 
  else
    { hidden = 0;
      X = $base;
      Y = X;
    }
  x = tolower(X);
  y = tolower(Y);
  if (hidden)
    z = "((" X " *) " y ")";
  else
    z = y;

# interpret field information

  packable  = 0;
  genobject = 0;
  if (NF > base)
    { for (i = base+1; i <= NF; i++)
        { if ((j = index($i,":")) != 0)
            { type[i] = 0; packable = 1; genobject = 1; }
          else if ((j = index($i,"*")) != 0)
            type[i] = 1;
          else if ((j = index($i,"^")) != 0)
            { type[i] = 2; packable = 1; }
          else
            { print "ERROR: illegal macro argument \"" $i "\"";
              exit;
            }
          field[i] = substr($i,1,j-1);
          value[i] = substr($i,j+1,length($i)-j);
          fname[i] = field[i];
          gsub("[^a-zA-Z0-9_]+","_",fname[i]);
        }
    }

# generate container and free list declarations

  print "";
  print "typedef struct __" X;
  print "  { struct __" X " *next;";
  for (i = base+1; i <= NF; i++)
    if (type[i] == 0) 
      printf "    size_t%*s%s;\n", length(X)+8, "", value[i];
  print "    " X "           " x ";";
  print "  } _" X ";";
  print "";
  print "static _" X " *Free_" X "_List = NULL;";
  print "static size_t    " X "_Offset, " X "_Inuse;";

# generate allocate_<field[i]>

  for (i = base+1; i <= NF; i++)
    if (type[i] == 0)
      { print ""
        printf "static inline void allocate_%s_%s(%s *%s, size_t %s, char *routine)\n",
               x, fname[i], X, x, value[i]; 
        print  "{ _" X " *object  = (_" X " *) (((char *) " x ") - " X "_Offset);";
        print  "  if (object->" value[i] " < " value[i] ")";
        print  "    { if (object->" value[i] " == 0)";
        print  "        object->" x "." field[i] " = NULL;"
        print  "#ifdef _MSC_VER"
        printf "      object->%s.%s  = (decltype(object->%s.%s))Guarded_Realloc(object->%s.%s,%s,routine);\n",
               x, field[i], x, field[i], x, field[i], value[i];
        print  "#else"
        printf "      object->%s.%s  = Guarded_Realloc(object->%s.%s,%s,routine);\n",
               x, field[i], x, field[i], value[i];
        print  "#endif"
        print  "      object->" value[i] " = " value[i] ";";
        print  "    }";
        print  "}";
      }

# generate new_<x>

  print "";
  printf "static inline %s *new_%s(", X, x;
  for (i = base+1; i <= NF; i++)
    if (type[i] == 0)
      printf "size_t %s, ", value[i];
  printf "char *routine)\n";
  print "{ _" X " *object;";
  print "";
  print "  if (Free_" X "_List == NULL)";
  print "    { object = (_" X " *) Guarded_Malloc(sizeof(_" X "),routine);";
  print "      " X "_Offset = ((char *) &(object->" x ")) - ((char *) object);";
  for (i = base+1; i <= NF; i++)
    if (type[i] == 0)
      print "      object->" value[i] " = 0;";
  print "    }";
  print "  else";
  print "    { object = Free_" X "_List;";
  print "      Free_" X "_List = object->next;";
  print "    }";
  print "  " X "_Inuse += 1;";
  for (i = base+1; i <= NF; i++)
    if (type[i] == 0)
      print "  allocate_" x "_" fname[i] "(&(object->" x ")," value[i] ",routine);";
    else
      print "  object-> " x "." field[i] " = NULL;";
  print "  return (&(object->" x "));";
  print "}";

# generate copy_<x> and Copy_<Y> if -c is not set

  print "";
  print "static inline " X " *copy_" x "(" X " *" x ")";
  printf "{ %s *copy = new_%s(", X, x;
  for (i = base+1; i <= NF; i++)
    if (type[i] == 0)
      printf "%s_%s(%s),", x, value[i], x; 
  printf "\"Copy_%s\");\n", Y;
  if (NF > base)
    print "  " X "  temp = *copy;";
  print "  *copy = *" x ";";
  for (i = base+1; i <= NF; i++)
    if (type[i] == 0)
      { print "  copy->" field[i] " = temp." field[i] ";";
        print "  if (" x "_" value[i] "(" x ") != 0)";
        print "    memcpy(copy->" field[i] "," x "->" field[i] "," x "_" value[i] "(" x "));";
      }
    else
      { print "  if (" x "->" field[i] " != NULL)";
        print "    copy->" field[i] " = Copy_" value[i] "(temp." field[i] ");";
      }
  print "  return (copy);"
  print "}";

  if (!copy)
    { print "";
      print Y " *Copy_" Y "(" Y " *" y ")";
      if (hidden)
        print "{ return ((" Y " *) copy_" x "((" X " *) " y ")); }";
      else
        print "{ return (copy_" x "(" y ")); }";
    }

# if packable generate pack_<x> and Pack_<Y> if the -p flag is not set

  if (packable)
    { print "";
      print "static inline void pack_" x "(" X " *" x ")";
      if (genobject)
        print "{ _" X " *object  = (_" X " *) (((char *) " x ") - " X "_Offset);";
      else
        print "{";
      for (i = base+1; i <= NF; i++)
        if (type[i] == 0)
          { print "  if (object->" value[i] " > " x "_" value[i] "(" x "))";
            print "    { object->" value[i] " = " x "_" value[i] "(" x ");";
            print "      if (object->" value[i] " != 0)";
            print  "#ifdef _MSC_VER"
            print "        object->" x "." field[i] " = (decltype(object->" x "." field[i] "))Guarded_Realloc(object->" x "." field[i] ",";
            print  "#else"
            print "        object->" x "." field[i] " = Guarded_Realloc(object->" x "." field[i] ",";
            print "#endif"
            printf "%*s  object->%s,\"Pack_%s\");\n", 34 + length(field[i]) + length(x), "",
                                                      value[i], X;
            print "      else";
            print "        { free(object->" x "." field[i] ");";
            print "          object->" value[i] " = 0;";
            print "        }"
            print "    }";
          }
        else if (type[i] == 2)
          { print "  if (" x "->" field[i] " != NULL)";
            print "    Pack_" value[i] "(" x "->" field[i] ");";
          }
      print "}";

      if (!pack)
        { print "";
          print "void Pack_" Y "(" Y " *" y ")";
          print "{ pack_" x "(" z "); }";
        }
    }

# generate free_<x> and Free_<Y> if the -f flag is not set

  print "";
  print "static inline void free_" x "(" X " *" x ")";
  print "{ _" X " *object  = (_" X " *) (((char *) " x ") - " X "_Offset);";
  print "  object->next = Free_" X "_List;";
  print "  Free_" X "_List = object;";
  if (NF > base)
    { for (i = NF; i > base; i--)
        if (type[i] > 0)
          { print "  if (" x "->" field[i] " != NULL)";
            print "    Free_" value[i] "(" x "->" field[i] ");"
          }
    }
  print "  " X "_Inuse -= 1;";
  print "}";

  if (!free)
    { print "";
      print "void Free_" Y "(" Y " *" y ")";
      print "{ free_" x "(" z "); }";
    }

# generate kill_<x> and Kill_<Y> if the -k flag is not set

  print "";
  print "static inline void kill_" x "(" X " *" x ")";
  if (genobject)
    print "{ _" X " *object  = (_" X " *) (((char *) " x ") - " X "_Offset);";
  else
    print "{";
  for (i = NF; i > base; i--)
    if (type[i] > 0)
      { print "  if (" x "->" field[i] " != NULL)"
        print "    Kill_" value[i] "(" x "->" field[i] ");"
      }
    else
      { print "  if (object->" value[i] " != 0)"
        print "    free(" x "->" field[i] ");"
      }
  print "  free(((char *) " x ") - " X "_Offset);";
  print "  " X "_Inuse -= 1;";
  print "}";

  if (!kill)
    { print "";
      print "void Kill_" Y "(" Y " *" y ")";
      print "{ kill_" x "(" z "); }";
    }

# generate reset_<x> and Reset_<Y> if the -r flag is not set

  print "";
  print "static inline void reset_" x "()";
  print "{ _" X " *object;";
  print "  while (Free_" X "_List != NULL)";
  print "    { object = Free_" X "_List;";
  print "      Free_" X "_List = object->next;";
  print "      kill_" x "(&(object->" x "));";
  print "      " X "_Inuse += 1;";
  print "    }";
  print "}";

  if (!reset)
    { print "";
      print "void Reset_" Y "()";
      print "{ reset_" x "(); }";
    }

# generate <Y>_Usage

  print "";
  print "int " Y "_Usage()";
  print "{ return (" X "_Inuse); }";
}

! /^MANAGER/ { print $0; }
