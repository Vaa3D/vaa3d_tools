#include <iostream>
#include <sstream>
#include <fstream>

#include <string.h>
#include <stdlib.h>

#include "tz_utilities.h"
#include "tz_string.h"
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
#include "tz_stack_stat.h"
#include "tz_intpair_map.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> -o <string> [--offset]", NULL};
  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  File_List *fileList = File_List_Load_Dir(
      Get_String_Arg(const_cast<char*>("input")), "tif",
      NULL);
  File_List_Sort_By_Number(fileList);
  if (!Is_Arg_Matched(const_cast<char*>("--offset"))) {
    printf("Making MIP ...\n");
  }

  Stack *stack = Stack_Mip_F(fileList);
  //Write_Stack(const_cast<char*>("../../data/mip.tif"), stack);

  if (!Is_Arg_Matched(const_cast<char*>("--offset"))) {
    printf("Making mask ...\n");
  }
  Stack *mask = Stack_Remove_Isolated_Object(stack, NULL, 100, 15);
  //Write_Stack(const_cast<char*>("../../data/mask.tif"), mask);

  Kill_Stack(stack);

  if (Is_Arg_Matched(const_cast<char*>("--offset"))) {
    Cuboid_I bound_box;
    Stack_Bound_Box(mask, &bound_box);

    bound_box.cb[2] = String_Last_Integer(fileList->file_path[0]) - 161;

    printf("%d %d %d\n", bound_box.cb[0], bound_box.cb[1], bound_box.cb[2]); 

    return 0;
  }

  printf("Making stack ...\n");
  stack = Read_Image_List_Bounded_M(fileList, mask);

  Write_Stack(Get_String_Arg(const_cast<char*>("-o")), stack);

  return 0;
}
