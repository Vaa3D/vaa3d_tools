/**@file tz_intpair_map.h
 * @author Ting Zhao
 * @date 21-Apr-2009
 */

#ifndef _TZ_INTPAIR_MAP_H_
#define _TZ_INTPAIR_MAP_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/* both key and value are non-negative to be valid */

typedef struct _Intpair_Map_Entry {
  int pair[2]; /* key */
  int value;   /* value */
  struct _Intpair_Map_Entry *next;
} Intpair_Map_Entry;

typedef struct _Intpair_Map {
  int length;
  int base;
  Intpair_Map_Entry *bucket;
  Intpair_Map_Entry *iterator;
} Intpair_Map;

void Print_Intpair_Map_Entry(const Intpair_Map_Entry *entry);

Intpair_Map_Entry* New_Intpair_Map_Entry();
void Delete_Intpair_Map_Entry(Intpair_Map_Entry *entry);

Intpair_Map* New_Intpair_Map();
Intpair_Map* Make_Intpair_Map(int length);

void Reset_Intpair_Map(Intpair_Map *map);
void Clean_Intpair_Map(Intpair_Map *map);

void Delete_Intpair_Map(Intpair_Map *map);
void Kill_Intpair_Map(Intpair_Map *map);

void Intpair_Map_Set_Base(Intpair_Map *map, int base);

int Intpair_Map_Value(const Intpair_Map *map, int x, int y);

/**@brief Add up value to an entry.
 *
 * If the pair does not exist, nothing will be done and return FALSE; otherwise
 * the value of (x, y) is set to (v+dv) and it returns TRUE.
 */
BOOL Intpair_Map_Add_Value(const Intpair_Map *map, int x, int y, int v);

int Intpair_Map_Add(Intpair_Map *map, int x, int y, int value);
int Intpair_Map_Remove(Intpair_Map *map, int x, int y);

void Intpair_Map_Refresh(Intpair_Map *map);

void Print_Intpair_Map(const Intpair_Map *map);

/* iterating routines */
void Intpair_Map_Iterator_Start(Intpair_Map *map);
Intpair_Map_Entry *Intpair_Map_Next(Intpair_Map *map);

__END_DECLS

#endif
