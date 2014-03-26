#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utilities.h>
#include "tz_intpair_map.h"

Intpair_Map_Entry* New_Intpair_Map_Entry()
{
  Intpair_Map_Entry *entry = (Intpair_Map_Entry*) 
    Guarded_Malloc(sizeof(Intpair_Map_Entry), "New_Intpair_Map_Entry");
  entry->pair[0] = -1;
  entry->pair[1] = -1;
  entry->value = -1;
  entry->next = NULL;
  
  return entry;
}

void Delete_Intpair_Map_Entry(Intpair_Map_Entry *entry)
{
  free(entry);
}

Intpair_Map* New_Intpair_Map()
{
  return Make_Intpair_Map(11);
}

Intpair_Map* Make_Intpair_Map(int length)
{
  Intpair_Map *map = (Intpair_Map*) Guarded_Malloc(sizeof(Intpair_Map), 
						   "New_Intpair_Map");
  map->length = length;
  map->bucket = (Intpair_Map_Entry*) 
    Guarded_Malloc(sizeof(Intpair_Map_Entry) * length, "New_Intpair_Map");

  int i;
  for (i = 0; i < length; i++) {
    map->bucket[i].next = NULL;
  }
  
  Reset_Intpair_Map(map);

  return map;
}

void Reset_Intpair_Map(Intpair_Map *map)
{
  int i;
  for (i = 0; i < map->length; i++) {
    map->bucket[i].pair[0] = -1;
    map->bucket[i].pair[1] = -1;
    map->bucket[i].value = -1;

    Intpair_Map_Entry *entry = map->bucket[i].next;
    while (entry != NULL) {
      Intpair_Map_Entry *next = entry->next;
      free(entry);
      entry = next;
    }

    map->bucket[i].next = NULL;
  }

  map->base = 10;
}

void Clean_Intpair_Map(Intpair_Map *map)
{
  Reset_Intpair_Map(map);
  free(map->bucket);
  map->length = 0;
  map->bucket = NULL;
  map->iterator = NULL;
}

void Delete_Intpair_Map(Intpair_Map *map)
{
  free(map);
}

void Kill_Intpair_Map(Intpair_Map *map)
{
  Clean_Intpair_Map(map);
  Delete_Intpair_Map(map);
}

void Intpair_Map_Set_Base(Intpair_Map *map, int base)
{
  if (map->base != base) {
    map->base = base;
    if (map->bucket != NULL) {
      Intpair_Map_Refresh(map);
    }
  }
}

static int intpair_map_code(const Intpair_Map *map, int x, int y)
{
  return (x * map->base + y) % map->length;
}

int Intpair_Map_Value(const Intpair_Map *map, int x, int y)
{
  if (map->length > 0) {
    int index = intpair_map_code(map, x, y);
  
    Intpair_Map_Entry *entry = map->bucket + index;
    while (entry != NULL) {
      if ((entry->pair[0] == x) && (entry->pair[1] == y)) {
	return entry->value;
      }
      entry = entry->next;
    }
  }

  return -1;
}

BOOL Intpair_Map_Add_Value(const Intpair_Map *map, int x, int y, int dv)
{
  if (map->length > 0) {
    int index = intpair_map_code(map, x, y);
  
    Intpair_Map_Entry *entry = map->bucket + index;
    while (entry != NULL) {
      if ((entry->pair[0] == x) && (entry->pair[1] == y)) {
        entry->value += dv;
        return TRUE;
      }
      entry = entry->next;
    }
  }

  return FALSE;
}

int Intpair_Map_Add(Intpair_Map *map, int x, int y, int value)
{
  int index = intpair_map_code(map, x, y);
  
  if (index < 0) {
    return 0;
  }

  Intpair_Map_Entry *entry = map->bucket + index;
  Intpair_Map_Entry *last_entry = entry;
  while (entry != NULL) {
    if ((entry->pair[0] == x) && (entry->pair[1] == y)) {
      /* the entry exists */
      return 0;
    }
    last_entry = entry;
    entry = entry->next;
  }

  while (entry != NULL) {
    last_entry = entry;
    entry = entry->next;
  }

  Intpair_Map_Entry *new_entry = NULL;
  
  if (last_entry->value < 0) {
    new_entry = last_entry;
  } else {
    new_entry = New_Intpair_Map_Entry();
    last_entry->next = new_entry;
  }

  new_entry->pair[0] = x;
  new_entry->pair[1] = y;
  new_entry->value = value;  
  
  return 1;
}

int Intpair_Map_Remove(Intpair_Map *map, int x, int y)
{
  int index = intpair_map_code(map, x, y);
  
  if (index < 0) {
    return 0;
  }

  Intpair_Map_Entry *prev = NULL;
  Intpair_Map_Entry *entry = map->bucket + index;
  while (entry != NULL) {
    if ((entry->pair[0] == x) && (entry->pair[1] == y)) {
      break;
    }
    prev = entry;
    entry = entry->next;
  }

  if (entry == NULL) {
    return 0;
  }

  if (prev != NULL) {
    prev->next = entry->next;
    Delete_Intpair_Map_Entry(entry);
  } else {
    if (entry->next == NULL) {
      entry->pair[0] = -1;
      entry->pair[1] = -1;
      entry->value = -1;
    } else {
      prev = entry;
      entry = entry->next;
      memcpy(prev, entry, sizeof(Intpair_Map_Entry));
      Delete_Intpair_Map_Entry(entry);
    }
  }

  return 1;
}

void Intpair_Map_Refresh(Intpair_Map *map)
{
  
}

void Print_Intpair_Map_Entry(const Intpair_Map_Entry *entry)
{
  if ((entry->pair[0] >= 0) && (entry->pair[1] >= 0)) {
    printf("(%d, %d) -> %d\n", entry->pair[0], entry->pair[1], entry->value);
  }
}

void Print_Intpair_Map(const Intpair_Map *map)
{
  printf("Int map with base %d\n", map->base);

  int i;
  for (i = 0; i < map->length; i++) {
    if ((map->bucket[i].pair[0] >= 0) && (map->bucket[i].pair[0] >= 0)) {
      printf("bucket %d:\n", i);
      Print_Intpair_Map_Entry(map->bucket + i);
      Intpair_Map_Entry *entry = map->bucket[i].next;
      while (entry != NULL) {
	Print_Intpair_Map_Entry(entry);
	entry = entry->next;
      }
    }
  }
}

void Intpair_Map_Iterator_Start(Intpair_Map *map)
{
  map->iterator = map->bucket;
}

Intpair_Map_Entry *Intpair_Map_Next(Intpair_Map *map)
{
  Intpair_Map_Entry *entry = map->iterator;

  if (map->iterator != NULL) {
    if (map->iterator->next != NULL) {
      map->iterator = map->iterator->next;
    } else {
      if (map->iterator - map->bucket >= map->length) {
	map->iterator = NULL;
      } else {
	map->iterator++;
      }
    }
  }

  return entry;
}
