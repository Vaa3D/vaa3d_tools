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

#ifndef _SR_HASH
#define _SR_HASH

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"

typedef void Hash_Table;

Hash_Table *G(New_Hash_Table)(int size);

Hash_Table *G(Copy_Hash_Table)(Hash_Table *table);
Hash_Table *Pack_Hash_Table(Hash_Table *R(M(table)));
Hash_Table *Inc_Hash_Table(Hash_Table *R(I(table)));
void        Free_Hash_Table(Hash_Table *F(table));
void        Kill_Hash_Table(Hash_Table *K(table));
void        Reset_Hash_Table();
int         Hash_Table_Usage();
void        Hash_Table_List(void (*handler)(Hash_Table *));
int         Hash_Table_Refcount(Hash_Table *table);
Hash_Table *G(Read_Hash_Table)(FILE *input);
void        Write_Hash_Table(Hash_Table *table, FILE *output);

void Clear_Hash_Table(Hash_Table *M(table));
void Print_Hash_Table(FILE *file, Hash_Table *table);

int Hash_Lookup(Hash_Table *table, string entry);
int Hash_Add(Hash_Table *M(table), string entry);

int    Get_Hash_Size(Hash_Table *table);
string Get_Hash_String(Hash_Table *table, int i);
void **Get_Hash_User_Hook(Hash_Table *table, int i);

#ifdef __cplusplus
}
#endif

#endif
