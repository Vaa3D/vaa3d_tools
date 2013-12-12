#include <stdio.h>
#include <stdlib.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_stack_tile_i.h"
#include "tz_stack_io.h"
#include "tz_xml_utils.h"
#include "tz_math.h"
#include "tz_stack_tile_i.h"

DEFINE_ZOBJECT_INTERFACE(Stack_Tile_I)

void Default_Stack_Tile_I(Stack_Tile_I *tile)
{
  if (tile != NULL) {
    tile->stack = NULL;
    tile->offset[0] = 0;
    tile->offset[1] = 0;
    tile->offset[2] = 0;
  }
}

void Clean_Stack_Tile_I(Stack_Tile_I *tile)
{
  if (tile->stack != NULL) {
    Kill_Stack(tile->stack);
  }

  Default_Stack_Tile_I(tile);
}

void Print_Stack_Tile_I(Stack_Tile_I *tile)
{
  Print_Stack_Info(tile->stack);
  printf("Offset: %d, %d, %d\n", tile->offset[0], tile->offset[1], 
	 tile->offset[2]);
}

Stack_Tile_I* Make_Stack_Tile_I(Stack *stack, int x, int y, int z)
{
  Stack_Tile_I *tile = New_Stack_Tile_I();
  tile->stack = stack;
  tile->offset[0] = x;
  tile->offset[1] = y;
  tile->offset[2] = z;

  return tile;  
}

Stack_Tile_I* Read_Stack_Tile_I(const char *filePath)
{
#ifdef HAVE_LIBXML2
  Stack_Document *stack_doc = Xml_Read_Stack_Document(filePath);
  Stack *stack = NULL;
  Stack_Tile_I *tile = NULL;

  if (stack_doc != NULL) {
    stack = Import_Stack_Document(stack_doc);
    if (stack != NULL) {
      tile = Make_Stack_Tile_I(stack, iround(stack_doc->offset[0]), 
			       iround(stack_doc->offset[1]), 
			       iround(stack_doc->offset[2]));
    }
    Kill_Stack_Document(stack_doc);
  }

  return tile;  
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif  
}

Cuboid_I* Stack_Tile_I_Boundbox(const Stack_Tile_I *tile, Cuboid_I *box)
{
  if (box == NULL) {
    box = New_Cuboid_I();
  }

  Cuboid_I_Set_S(box, tile->offset[0], tile->offset[1], tile->offset[2],
		 Stack_Tile_I_Width(tile), Stack_Tile_I_Height(tile), 
		 Stack_Tile_I_Depth(tile));

  return box;
}

int Stack_Tile_I_Width(const Stack_Tile_I *tile)
{
  return tile->stack->width;
}

int Stack_Tile_I_Height(const Stack_Tile_I *tile)
{
  return tile->stack->height;
}

int Stack_Tile_I_Depth(const Stack_Tile_I *tile)
{
  return tile->stack->depth;
}
