#include <stdio.h>
#include <stdlib.h>
#include "tz_intpair_map.h"

int main(int argc, char *argv[])
{
#if 1
  Intpair_Map *map = Make_Intpair_Map(11);
  Intpair_Map_Add(map, 3, 5, 0);
  Intpair_Map_Add(map, 3, 2, 1);
  Intpair_Map_Add(map, 0, 2, 2);
  Print_Intpair_Map(map);

  printf("%d\n", Intpair_Map_Value(map, 3, 2));

  Intpair_Map_Remove(map, 3, 5);
  Print_Intpair_Map(map);

  Intpair_Map_Iterator_Start(map);
  Intpair_Map_Entry *entry = NULL;
  while ((entry = Intpair_Map_Next(map)) != NULL) {
    Print_Intpair_Map_Entry(entry);
  }

#endif

  return 0;
}
