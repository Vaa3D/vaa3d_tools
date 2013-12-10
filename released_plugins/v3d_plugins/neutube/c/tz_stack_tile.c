#include <stdio.h>
#include <stdlib.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_stack_tile.h"
#include "tz_stack_io.h"
#include "tz_xml_utils.h"

DEFINE_ZOBJECT_INTERFACE(Stack_Tile)

void Default_Stack_Tile(Stack_Tile *tile)
{
  if (tile != NULL) {
    tile->stack = NULL;
    tile->offset[0] = 0.0;
    tile->offset[1] = 0.0;
    tile->offset[2] = 0.0;
  }
}

void Clean_Stack_Tile(Stack_Tile *tile)
{
  if (tile->stack != NULL) {
    Kill_Stack(tile->stack);
  }

  Default_Stack_Tile(tile);
}

void Print_Stack_Tile(Stack_Tile *tile)
{
  Print_Stack_Info(tile->stack);
  printf("Offset: %g, %g, %g\n", tile->offset[0], tile->offset[1], 
	 tile->offset[2]);
}

Stack_Tile* Make_Stack_Tile(Stack *stack, const double *offset)
{
  Stack_Tile *tile = New_Stack_Tile();
  tile->stack = stack;
  tile->offset[0] = offset[0];
  tile->offset[1] = offset[1];
  tile->offset[2] = offset[2];

  return tile;
}

Stack_Tile* Make_Stack_Tile_D(Stack *stack, double x, double y, double z)
{
  Stack_Tile *tile = New_Stack_Tile();
  tile->stack = stack;
  tile->offset[0] = x;
  tile->offset[1] = y;
  tile->offset[2] = z;

  return tile;  
}

Stack_Tile* Read_Stack_Tile(const char *filePath)
{
#ifdef HAVE_LIBXML2
  Stack_Document *stack_doc = Xml_Read_Stack_Document(filePath);
  Stack *stack = NULL;
  Stack_Tile *tile = NULL;

  if (stack_doc != NULL) {
    stack = Import_Stack_Document(stack_doc);
    if (stack != NULL) {
      tile = Make_Stack_Tile(stack, stack_doc->offset);
    }
    Kill_Stack_Document(stack_doc);
  }

  return tile;  
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif  
}
