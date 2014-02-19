/*****************************************************************************************\
*                                                                                         *
*  Hash Table data abstraction.                                                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  March 2006                                                                    *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"
#include "hash.h"

/* Hash Table cell or entry, index is implicitly given
   by position in table->cells array.                     */

typedef struct
{ int      next;  /* hash bucket link */
  int      text;  /* offset of string corresponding to entry in string array */
  void    *user;  /* user hook */
} Hash_Entry;

typedef struct
{ int         size;    /* size of hash vector */
  int         count;   /* number of entries in hash table */
  int         strtop;  /* current top of string array  */
  int        *vector;  /* hash vector */
  Hash_Entry *cells;   /* array where hash cells are allocated */
  char       *strings; /* array of entry strings */
} Table;

#define CELL_RATIO    .4  // maximum ratio of cells to hash vector length
#define STRING_RATIO   6  // expected average entry length (including terminating 0-byte)

#define T(x) ((Table *) x)

static inline int get_string_max(Table *table); // A kludge to get around macro-induced circularity

/*  Hash Table memory management  */

static inline int table_vsize(Table *table)
{ return (sizeof(int)*table->size); }

static inline int table_csize(Table *table)
{ return (sizeof(Hash_Entry)*((int) (table->size*CELL_RATIO))); }

static inline int table_ssize(Table *table)
{ return (get_string_max(table)); }


typedef struct __Table
  { struct __Table *next;
    size_t             vsize;
    size_t             csize;
    size_t             ssize;
    Table           table;
  } _Table;

static _Table *Free_Table_List = NULL;
static size_t    Table_Offset, Table_Inuse;

static inline void allocate_table_vector(Table *table, size_t vsize, char *routine)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  if (object->vsize < vsize)
    { if (object->vsize == 0)
        object->table.vector = NULL;
#ifdef _MSC_VER
      object->table.vector  = (decltype(object->table.vector))Guarded_Realloc(object->table.vector,vsize,routine);
#else
      object->table.vector  = Guarded_Realloc(object->table.vector,vsize,routine);
#endif
      object->vsize = vsize;
    }
}

static inline void allocate_table_cells(Table *table, size_t csize, char *routine)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  if (object->csize < csize)
    { if (object->csize == 0)
        object->table.cells = NULL;
#ifdef _MSC_VER
      object->table.cells  = (decltype(object->table.cells))Guarded_Realloc(object->table.cells,csize,routine);
#else
      object->table.cells  = Guarded_Realloc(object->table.cells,csize,routine);
#endif
      object->csize = csize;
    }
}

static inline void allocate_table_strings(Table *table, size_t ssize, char *routine)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  if (object->ssize < ssize)
    { if (object->ssize == 0)
        object->table.strings = NULL;
#ifdef _MSC_VER
      object->table.strings  = (decltype(object->table.strings))Guarded_Realloc(object->table.strings,ssize,routine);
#else
      object->table.strings  = Guarded_Realloc(object->table.strings,ssize,routine);
#endif
      object->ssize = ssize;
    }
}

static inline Table *new_table(size_t vsize, size_t csize, size_t ssize, char *routine)
{ _Table *object;

  if (Free_Table_List == NULL)
    { object = (_Table *) Guarded_Malloc(sizeof(_Table),routine);
      Table_Offset = ((char *) &(object->table)) - ((char *) object);
      object->vsize = 0;
      object->csize = 0;
      object->ssize = 0;
    }
  else
    { object = Free_Table_List;
      Free_Table_List = object->next;
    }
  Table_Inuse += 1;
  allocate_table_vector(&(object->table),vsize,routine);
  allocate_table_cells(&(object->table),csize,routine);
  allocate_table_strings(&(object->table),ssize,routine);
  return (&(object->table));
}

static inline Table *copy_table(Table *table)
{ Table *copy = new_table(table_vsize(table),table_csize(table),table_ssize(table),"Copy_Hash_Table");
  Table  temp = *copy;
  *copy = *table;
  copy->vector = temp.vector;
  if (table_vsize(table) != 0)
    memcpy(copy->vector,table->vector,table_vsize(table));
  copy->cells = temp.cells;
  if (table_csize(table) != 0)
    memcpy(copy->cells,table->cells,table_csize(table));
  copy->strings = temp.strings;
  if (table_ssize(table) != 0)
    memcpy(copy->strings,table->strings,table_ssize(table));
  return (copy);
}

Hash_Table *Copy_Hash_Table(Hash_Table *hash_table)
{ return ((Hash_Table *) copy_table((Table *) hash_table)); }

static inline void pack_table(Table *table)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  if (object->vsize > table_vsize(table))
    { object->vsize = table_vsize(table);
      if (object->vsize != 0)
#ifdef _MSC_VER
        object->table.vector = (decltype(object->table.vector))Guarded_Realloc(object->table.vector,
#else
        object->table.vector = Guarded_Realloc(object->table.vector,
#endif
                                               object->vsize,"Pack_Table");
      else
        { free(object->table.vector);
          object->vsize = 0;
        }
    }
  if (object->csize > table_csize(table))
    { object->csize = table_csize(table);
      if (object->csize != 0)
#ifdef _MSC_VER
        object->table.cells = (decltype(object->table.cells))Guarded_Realloc(object->table.cells,
#else
        object->table.cells = Guarded_Realloc(object->table.cells,
#endif
                                              object->csize,"Pack_Table");
      else
        { free(object->table.cells);
          object->csize = 0;
        }
    }
  if (object->ssize > table_ssize(table))
    { object->ssize = table_ssize(table);
      if (object->ssize != 0)
#ifdef _MSC_VER
        object->table.strings = (decltype(object->table.strings))Guarded_Realloc(object->table.strings,
#else
        object->table.strings = Guarded_Realloc(object->table.strings,
#endif
                                                object->ssize,"Pack_Table");
      else
        { free(object->table.strings);
          object->ssize = 0;
        }
    }
}

void Pack_Hash_Table(Hash_Table *hash_table)
{ pack_table(((Table *) hash_table)); }

static inline void free_table(Table *table)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  object->next = Free_Table_List;
  Free_Table_List = object;
  Table_Inuse -= 1;
}

void Free_Hash_Table(Hash_Table *hash_table)
{ free_table(((Table *) hash_table)); }

static inline void kill_table(Table *table)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  if (object->ssize != 0)
    free(table->strings);
  if (object->csize != 0)
    free(table->cells);
  if (object->vsize != 0)
    free(table->vector);
  free(((char *) table) - Table_Offset);
  Table_Inuse -= 1;
}

void Kill_Hash_Table(Hash_Table *hash_table)
{ kill_table(((Table *) hash_table)); }

static inline void reset_table()
{ _Table *object;
  while (Free_Table_List != NULL)
    { object = Free_Table_List;
      Free_Table_List = object->next;
      kill_table(&(object->table));
      Table_Inuse += 1;
    }
}

void Reset_Hash_Table()
{ reset_table(); }

int Hash_Table_Usage()
{ return (Table_Inuse); }

static inline int get_string_max(Table *table)   // 2nd half of the kludge (that know has the defs)
{ _Table *object  = (_Table *) (((char  *) table) - Table_Offset);
  return (object->ssize);
}

/* Hash key for a string is xor of each consecutive 3 bytes. */

static int hash_key(char *entry)
{ int i, key, glob;

  key = 0;
  glob = 0;
  for (i = 0; entry[i] != '\0'; i++)
    { glob = (glob << 8) | entry[i];
      if (i % 3 == 2)
        { key = key ^ glob;
          glob = 0;
        }
    }
  if (i % 3 != 0)
    key = key ^ glob;
  return (key);
}

/*  Find the next prime larger than size.  Variant of Sieve of Arosthenes:  First
      time its called computes all primes between 2 and 0xFFFF using the basic
      sieve algorithm.  With these one builds a sieve for the 0xFFFF number from
      size upwards, using the primes to x-out sieve elements as being non-prime.
      This will work up to 0xFFFFFFF, beyond the largest integer, because it suffices
      to sieve agains the square root of the large number in the sieve.              */

static int next_prime(int size)
{ static int           firstime = 1;
  static int           Prime[0x4000], Ptop;
  static unsigned char Sieve[0x10000];
  int p, q, n;

  if (firstime)
    { firstime = 0;

      Ptop = 0;
      for (p = 2; p < 0x10000; p++)
        Sieve[p] = 1;
      for (p = 2; p < 0x10000; p++)
        if (Sieve[p])
          { for (q = 2*p; q < 0x10000; q += p)
              Sieve[q] = 0;
            Prime[Ptop++] = p;
          }
    }

  while (size < 0x8FFEFFFF)
    { for (q = 0; q < 0x10000; q++)
        Sieve[q] = 1;
      for (p = 0; p < Ptop; p++)
        { n = Prime[p];
          if (n >= size) break;
          for (q = ((size-1)/n+1)*n - size; q < 0x10000; q += n)
            Sieve[q] = 0;
        }
      for (q = 0; q < 0x10000; q++)
        if (Sieve[q])
          return (size+q);
      size += 0x10000;
    }
  return (size);
}


/* Diagnostic output of hash table contents. */

void Print_Hash_Table(FILE *file, Hash_Table *hash_table)
{ Table      *table  = T(hash_table);
  int        *vector = table->vector;
  Hash_Entry *cells  = table->cells;
  int         i, c;

  fprintf(file,"\nHASH TABLE (%d,%d):\n",table->count,table->size);
  for (i = 0; i < table->size; i++)
    if ((c = vector[i]) >= 0)
      { fprintf(file,"  Vector %4d:\n",i);
        for (; c >= 0; c = cells[c].next)
          fprintf(file,"    %4d: '%s'\n",c,table->strings + cells[c].text);
      }
}

Hash_Table *New_Hash_Table(int size)
{ Table *table;
  int    i;

  size  = next_prime((int) (size/CELL_RATIO));
  table = new_table(sizeof(int)*size,sizeof(Hash_Entry)*((int) (size*CELL_RATIO)),
                    size*STRING_RATIO,"New_Hash_Table");
  table->count  = 0;
  table->size   = size;
  table->strtop = 0;
  for (i = 0; i < size; i++)
    table->vector[i] = -1;

  return ((Hash_Table *) table);
}

/* Double the size of a hash table
   while preserving its contents.    */

static Table *double_hash_table(Table *table)
{ int size, smax;

  size = next_prime(2*table->size);
  smax = 2.1 * table->strtop + 1000;
 
  Free_Hash_Table((Hash_Table *) table);

  // You get back the table you just put on the free list, but bigger now.

  table = new_table(sizeof(int)*size,sizeof(Hash_Entry)*((int) (size*CELL_RATIO)),
                    smax,"Hash_Add");

  { int        *vector = table->vector;
    Hash_Entry *cells  = table->cells;
    int         c;

    table->size = size;
    for (c = 0; c < size; c++)
      vector[c] = -1;
    for (c = 0; c < table->count; c++)
      { int key = hash_key(table->strings + cells[c].text) % size;
        cells[c].next = vector[key];
        vector[key] = c;
      }
  }

  return (table);
}

/* Lookup string 'entry' in table 'table' and return its
   unique nonnegative id, or -1 if it is not in the table. */

int Hash_Lookup(Hash_Table *hash_table, char *entry)
{ Table *table = T(hash_table);
  int    key, chain;

  key   = hash_key(entry) % table->size;
  chain = table->vector[key];
  while (chain >= 0)
    { if (strcmp(table->strings + table->cells[chain].text,entry) == 0)
        return (chain);
      chain = table->cells[chain].next;
    }
  return (-1);
}

/* Add string 'entry' in table 'table' and return its assigned
   uniqe nonnegative id, or -1 if it is already in the table.  */

int Hash_Add(Hash_Table *hash_table, char *entry)
{ Table *table = T(hash_table);
  int key, chain, len;

  key   = hash_key(entry) % table->size;
  chain = table->vector[key];
  while (chain >= 0)
    { if (strcmp(table->strings + table->cells[chain].text,entry) == 0)
        return (-1);
      chain = table->cells[chain].next;
    }

  if (table->count+1 > table->size*CELL_RATIO)
    { table = double_hash_table(table);
      key   = hash_key(entry) % table->size;
    }

  chain = table->count;
  table->cells[chain].next = table->vector[key];
  table->vector[key] = chain;

  len = strlen(entry) + 1;
  if (table->strtop + len > get_string_max(table))
    { /*int size = table->size;*/
      int smax = (table->strtop + len) * (CELL_RATIO * table->size / table->count) * 1.1 + 1000;
      Free_Hash_Table((Hash_Table *) table);
      table = new_table(table_vsize(table),table_csize(table),smax,"Hash_Add");
    }
  strcpy(table->strings + table->strtop, entry);
  table->cells[chain].text = table->strtop;
  table->strtop += len;
  return (table->count++);
}

/* Return the size of the hash table. */

int Get_Hash_Table_Size(Hash_Table *hash_table)
{ return (T(hash_table)->count); }

/* Return the string with unique id i in table. */

char *Get_Hash_String(Hash_Table *hash_table, int i)
{ return (T(hash_table)->strings + T(hash_table)->cells[i].text); }

/* Return a pointer to the user data field for unique id i in table. */

void **Get_Hash_User_Hook(Hash_Table *hash_table, int i)
{ return (& (T(hash_table)->cells[i].user)); }

/* Clear the contents of hash table, reseting it to be empty. */

void Clear_Hash_Table(Hash_Table *hash_table)
{ Table *table = T(hash_table);
  int    i;

  table->count  = 0;
  table->strtop = 0;
  for (i = 0; i < table->size; i++)
    table->vector[i] = -1;
}
