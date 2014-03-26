#include <iostream>
#include <sstream>
#include <fstream>

#include <string.h>
#include <stdlib.h>

#include "tz_utilities.h"
#include "zsegmentmaparray.h"
#include "zsuperpixelmaparray.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "tz_stack_attribute.h"
#include "tz_stack_document.h"
#include "tz_xml_utils.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_objlabel.h"
#include "tz_swc_tree.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "tz_sp_grow.h"
#include "zspgrowparser.h"
#include "tz_stack_stat.h"
#include "tz_intpair_map.h"
#include "tz_stack_utils.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> [-o <string>]",
    "--body_id <int>", NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  Stack *stack = Read_Stack_U(Get_String_Arg(const_cast<char*>("input")));
  Stack *mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(mask);


  int bodyId = Get_Int_Arg(const_cast<char*>("--body_id"));

  size_t nvoxel = Stack_Voxel_Number(stack);

  switch (stack->kind) {
    case GREY16:
      {
        uint16_t *array16 = (uint16_t*) stack->array;
        for (size_t k = 0; k < nvoxel; k++) {
          if (array16[k] == (uint16_t)bodyId) {
            mask->array[k] = 1;
          }
        }
      }
      break;
    case COLOR:
      {
        color_t *arrayc = (color_t*) stack->array;
        for (size_t k = 0; k < nvoxel; k++) {
          if (Color_To_Value(arrayc[k]) == bodyId) {
            mask->array[k] = 1;
          }
        }
      }
      break;
    default:
      break;
  }

  Write_Stack_U(Get_String_Arg(const_cast<char*>("-o")), mask, NULL);

  return 0;
}
