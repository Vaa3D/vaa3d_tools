#ifdef _MSC_VER

#pragma warning( disable:4996 )

#ifndef __cplusplus
#define inline __inline
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

//  WINDOW pthreads "LIBRARY"

  //  Mutex macros

typedef SRWLOCK pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER RTL_SRWLOCK_INIT

#define pthread_mutex_lock(m)  AcquireSRWLockExclusive(m)

#define pthread_mutex_unlock(m) ReleaseSRWLockExclusive(m)

  //  Condition variable macros

typedef CONDITION_VARIABLE pthread_cond_t;

#define PTHREAD_COND_INITIALIZER RTL_CONDITION_VARIABLE_INIT

#define pthread_cond_signal(c) WakeConditionVariable(c)

#define pthread_cond_broadcast(c) WakeAllConditionVariable(c)

#define pthread_cond_wait(c,m) SleepConditionVariableSRW(c,m,INFINITE,0)

  //  Simple thread support

typedef struct
  { HANDLE *handle;
    void   *(*fct)(void *);
    void   *arg;
    void   *retval;
    int     id;
  } Mythread;

typedef Mythread *pthread_t;

static DWORD WINAPI MyStart(void *arg)
{ Mythread *tv = (Mythread *) arg;

  tv->retval = tv->fct(tv->arg);
  return (0);
}

static int pthread_create(pthread_t *thread, void *attr,
                          void *(*fct)(void *), void *arg)
{ Mythread *tv;
  if (attr != NULL)
    { fprintf(stderr,"Do not support thread attributes\n");
      exit (1);
    }
  tv = (Mythread *) malloc(sizeof(Mythread));
  if (tv == NULL)
    { fprintf(stderr,"pthread_create: Out of memory.\n");
      exit (1);
    };
  tv->fct    = fct;
  tv->arg    = arg;
  tv->handle = CreateThread(NULL,0,MyStart,tv,0,&tv->id);
  if (tv->handle == NULL)
    return (EAGAIN);
  else
    return (0);
}

static int pthread_join(pthread_t t, void **ret)
{ Mythread *tv = (Mythread *) t;

  WaitForSingleObject(tv->handle,INFINITE);
  if (ret != NULL)
    *ret = tv->retval;

  CloseHandle(tv->handle);
  free(tv);
  return (0);
}

typedef int pthread_id;

static pthread_id pthread_tag()
{ return (GetCurrentThreadId()); }

static int pthread_is_this(pthread_id id)
{ return (GetCurrentThreadId() == id); }

#else   //  Small extension to pthreads!

#include <pthread.h>

typedef pthread_t pthread_id;

#define pthread_tag() pthread_self()

static inline int pthread_is_this(pthread_id id)
{ return (pthread_equal(pthread_self(),id)); }

#endif

/*****************************************************************************************\
*                                                                                         *
*  Hash Table data abstraction.                                                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  March 2006                                                                    *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
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
  int         strmax;  /* current max of string array */
  int         strtop;  /* current top of string array */
  int        *vector;  /* hash vector */
  Hash_Entry *cells;   /* array where hash cells are allocated */
  char       *strings; /* array of entry strings */
} Table;

#define CELL_RATIO    .4  // maximum ratio of cells to hash vector length
#define STRING_RATIO   6  // expected average entry length (including terminating 0-byte)

#define T(x) ((Table *) x)

#define SIZEOF(x) ((int) sizeof(x))


/*  Hash Table memory management  */

static inline int table_vsize(Table *table)
{ return (SIZEOF(int)*table->size); }

static inline int table_csize(Table *table)
{ return (SIZEOF(Hash_Entry)*((int) (table->size*CELL_RATIO))); }

static inline int table_ssize(Table *table)
{ return (table->strmax); }


typedef struct __Table
  { struct __Table *next;
    struct __Table *prev;
    int             refcnt;
    int             vsize;
    int             csize;
    int             ssize;
    Table           table;
  } _Table;

static _Table *Free_Table_List = NULL;
static _Table *Use_Table_List  = NULL;

static pthread_mutex_t Table_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int Table_Offset = sizeof(_Table)-sizeof(Table);
static int Table_Inuse  = 0;

int Hash_Table_Refcount(Hash_Table *hash_table)
{ _Table *object = (_Table *) (((char *) hash_table) - Table_Offset);
  return (object->refcnt);
}

static inline int allocate_table_vector(Table *table, int vsize, char *routine)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  if (object->vsize < vsize)
    { void *x = Guarded_Realloc(table->vector,(size_t) vsize,routine);
      if (x == NULL) return (1);
      table->vector = x;
      object->vsize = vsize;
    }
  return (0);
}

static inline int sizeof_table_vector(Table *table)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  return (object->vsize);
}

static inline int allocate_table_cells(Table *table, int csize, char *routine)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  if (object->csize < csize)
    { void *x = Guarded_Realloc(table->cells,(size_t) csize,routine);
      if (x == NULL) return (1);
      table->cells = x;
      object->csize = csize;
    }
  return (0);
}

static inline int sizeof_table_cells(Table *table)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  return (object->csize);
}

static inline int allocate_table_strings(Table *table, int ssize, char *routine)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  if (object->ssize < ssize)
    { void *x = Guarded_Realloc(table->strings,(size_t) ssize,routine);
      if (x == NULL) return (1);
      table->strings = x;
      object->ssize = ssize;
    }
  return (0);
}

static inline int sizeof_table_strings(Table *table)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  return (object->ssize);
}

static inline void kill_table(Table *table);

static inline Table *new_table(int vsize, int csize, int ssize, char *routine)
{ _Table *object;
  Table  *table;

  pthread_mutex_lock(&Table_Mutex);
  if (Free_Table_List == NULL)
    { object = (_Table *) Guarded_Realloc(NULL,sizeof(_Table),routine);
      if (object == NULL) return (NULL);
      table = &(object->table);
      object->vsize = 0;
      table->vector = NULL;
      object->csize = 0;
      table->cells = NULL;
      object->ssize = 0;
      table->strings = NULL;
    }
  else
    { object = Free_Table_List;
      Free_Table_List = object->next;
      table = &(object->table);
    }
  Table_Inuse += 1;
  object->refcnt = 1;
  if (Use_Table_List != NULL)
    Use_Table_List->prev = object;
  object->next = Use_Table_List;
  object->prev = NULL;
  Use_Table_List = object;
  pthread_mutex_unlock(&Table_Mutex);
  if (allocate_table_vector(table,vsize,routine))
    { kill_table(table);
      return (NULL);
    }
  if (allocate_table_cells(table,csize,routine))
    { kill_table(table);
      return (NULL);
    }
  if (allocate_table_strings(table,ssize,routine))
    { kill_table(table);
      return (NULL);
    }
  return (table);
}

static inline Table *copy_table(Table *table)
{ Table *copy = new_table(table_vsize(table),table_csize(table),table_ssize(table),"Copy_Hash_Table");
  void *_vector = copy->vector;
  void *_cells = copy->cells;
  void *_strings = copy->strings;
  *copy = *table;
  copy->vector = _vector;
  if (table->vector != NULL)
    memcpy(copy->vector,table->vector,(size_t) table_vsize(table));
  copy->cells = _cells;
  if (table->cells != NULL)
    memcpy(copy->cells,table->cells,(size_t) table_csize(table));
  copy->strings = _strings;
  if (table->strings != NULL)
    memcpy(copy->strings,table->strings,(size_t) table_ssize(table));
  return (copy);
}

Hash_Table *Copy_Hash_Table(Hash_Table *hash_table)
{ return ((Hash_Table *) copy_table(((Table *) hash_table))); }

static inline int pack_table(Table *table)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  if (object->vsize > table_vsize(table))
    { int ns = table_vsize(table);
      if (ns != 0)
        { void *x = Guarded_Realloc(table->vector,(size_t) ns,"Pack_Table");
          if (x == NULL) return (1);
          table->vector = x;
        }
      else
        { free(table->vector);
          table->vector = NULL;
        }
      object->vsize = ns;
    }
  if (object->csize > table_csize(table))
    { int ns = table_csize(table);
      if (ns != 0)
        { void *x = Guarded_Realloc(table->cells,(size_t) ns,"Pack_Table");
          if (x == NULL) return (1);
          table->cells = x;
        }
      else
        { free(table->cells);
          table->cells = NULL;
        }
      object->csize = ns;
    }
  if (object->ssize > table_ssize(table))
    { int ns = table_ssize(table);
      if (ns != 0)
        { void *x = Guarded_Realloc(table->strings,(size_t) ns,"Pack_Table");
          if (x == NULL) return (1);
          table->strings = x;
        }
      else
        { free(table->strings);
          table->strings = NULL;
        }
      object->ssize = ns;
    }
  return (0);
}

Hash_Table *Pack_Hash_Table(Hash_Table *hash_table)
{ if (pack_table(((Table *) hash_table))) return (NULL);
  return (hash_table);
}

Hash_Table *Inc_Hash_Table(Hash_Table *hash_table)
{ _Table *object  = (_Table *) (((char *) hash_table) - Table_Offset);
  pthread_mutex_lock(&Table_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&Table_Mutex);
  return (hash_table);
}

static inline void free_table(Table *table)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  pthread_mutex_lock(&Table_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Table_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Hash_Table\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Table_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Table_List;
  Free_Table_List = object;
  Table_Inuse -= 1;
  pthread_mutex_unlock(&Table_Mutex);
}

void Free_Hash_Table(Hash_Table *hash_table)
{ free_table(((Table *) hash_table)); }

static inline void kill_table(Table *table)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  pthread_mutex_lock(&Table_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Table_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Hash_Table\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Table_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  Table_Inuse -= 1;
  pthread_mutex_unlock(&Table_Mutex);
  if (table->strings != NULL)
    free(table->strings);
  if (table->cells != NULL)
    free(table->cells);
  if (table->vector != NULL)
    free(table->vector);
  free(((char *) table) - Table_Offset);
}

void Kill_Hash_Table(Hash_Table *hash_table)
{ kill_table(((Table *) hash_table)); }

static inline void reset_table()
{ _Table *object;
  Table  *table;
  pthread_mutex_lock(&Table_Mutex);
  while (Free_Table_List != NULL)
    { object = Free_Table_List;
      Free_Table_List = object->next;
      table = &(object->table);
      if (table->strings != NULL)
        free(table->strings);
      if (table->cells != NULL)
        free(table->cells);
      if (table->vector != NULL)
        free(table->vector);
      free(object);
    }
  pthread_mutex_unlock(&Table_Mutex);
}

void Reset_Hash_Table()
{ reset_table(); }

int Hash_Table_Usage()
{ return (Table_Inuse); }

void Hash_Table_List(void (*handler)(Hash_Table *))
{ _Table *a, *b;
  for (a = Use_Table_List; a != NULL; a = b)
    { b = a->next;
      handler((Hash_Table *) &(a->table));
    }
}

static inline Table *read_table(FILE *input)
{ char name[10];
  Table *obj;
  Table read;
  fread(name,10,1,input);
  if (strncmp(name,"Hash_Table",10) != 0)
    return (NULL);
  obj = new_table(0,0,0,"Read_Hash_Table");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(Table),1,input) == 0) goto error;
  obj->vector = read.vector;
  if (table_vsize(obj) != 0)
    { if (allocate_table_vector(obj,table_vsize(obj),"Read_Hash_Table")) goto error;
      if (fread(obj->vector,(size_t) table_vsize(obj),1,input) == 0) goto error;
    }
  obj->cells = read.cells;
  if (table_csize(obj) != 0)
    { if (allocate_table_cells(obj,table_csize(obj),"Read_Hash_Table")) goto error;
      if (fread(obj->cells,(size_t) table_csize(obj),1,input) == 0) goto error;
    }
  obj->strings = read.strings;
  if (table_ssize(obj) != 0)
    { if (allocate_table_strings(obj,table_ssize(obj),"Read_Hash_Table")) goto error;
      if (fread(obj->strings,(size_t) table_ssize(obj),1,input) == 0) goto error;
    }
  return (obj);

error:
  kill_table(obj);
  return (NULL);
}

Hash_Table *Read_Hash_Table(FILE *input)
{ return ((Hash_Table *) read_table(input)); }

static inline void write_table(Table *table, FILE *output)
{ fwrite("Hash_Table",10,1,output);
  fwrite(table,sizeof(Table),1,output);
  if (table_vsize(table) != 0)
    fwrite(table->vector,(size_t) table_vsize(table),1,output);
  if (table_csize(table) != 0)
    fwrite(table->cells,(size_t) table_csize(table),1,output);
  if (table_ssize(table) != 0)
    fwrite(table->strings,(size_t) table_ssize(table),1,output);
}

void Write_Hash_Table(Hash_Table *hash_table, FILE *output)
{ write_table(((Table *) hash_table),output); }

/* Hash key for a string is xor of each consecutive 3 bytes. */

static int hash_key(string entry)
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
      sieve algorithm.  With these one builds a sieve for the 0xFFFF numbers from
      size upwards, using the primes to x-out sieve elements as being non-prime.
      This will work up to 0x7FFFFFF, beyond the largest positive integer, because 
      it suffices to sieve agains the square root of the largest number in the sieve.   */

static pthread_mutex_t Prime_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int next_prime(int size)
{ static int           firstime = 1;
  static int           Prime[0x4000], Ptop;
  static unsigned char Sieve[0x10000];
  int p, q, n;

  pthread_mutex_lock(&Prime_Mutex);

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

  while (size < 0x7FFF0000)
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

  pthread_mutex_unlock(&Prime_Mutex);

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

Hash_Table *G(New_Hash_Table)(int size)
{ Table *table;
  int    i, smax;

  size  = next_prime((int) (size/CELL_RATIO));
  smax  = size*STRING_RATIO;
  table = new_table(SIZEOF(int)*size,SIZEOF(Hash_Entry)*((int) (size*CELL_RATIO)),smax,
                    "New_Hash_Table");
  table->count  = 0;
  table->size   = size;
  table->strmax = smax;
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
  smax = (int) (2.1 * table->strtop + 1000);
 
  allocate_table_vector(table,SIZEOF(int)*size,"Hash_Add");
  allocate_table_cells(table,SIZEOF(Hash_Entry)*((int) (size*CELL_RATIO)),"Hash_Add");
  allocate_table_strings(table,smax,"Hash_Add");

  { int        *vector = table->vector;
    Hash_Entry *cells  = table->cells;
    int         c;

    table->size   = size;
    table->strmax = smax;
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

int Hash_Lookup(Hash_Table *hash_table, string entry)
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

int Hash_Add(Hash_Table *M(hash_table), string entry)
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

  len = (int) (strlen(entry) + 1);
  if (table->strtop + len > table->strmax)
    { int smax = (int)
           ((table->strtop + len) * (CELL_RATIO * table->size / table->count) * 1.1 + 1000);
      allocate_table_strings(table,smax,"Hash_Add");
      table->strmax = smax;
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

string Get_Hash_String(Hash_Table *hash_table, int i)
{ return (T(hash_table)->strings + T(hash_table)->cells[i].text); }

/* Return a pointer to the user data field for unique id i in table. */

void **Get_Hash_User_Hook(Hash_Table *hash_table, int i)
{ return (& (T(hash_table)->cells[i].user)); }

/* Clear the contents of hash table, reseting it to be empty. */

void Clear_Hash_Table(Hash_Table *M(hash_table))
{ Table *table = T(hash_table);
  int    i;

  table->count  = 0;
  table->strtop = 0;
  for (i = 0; i < table->size; i++)
    table->vector[i] = -1;
}
