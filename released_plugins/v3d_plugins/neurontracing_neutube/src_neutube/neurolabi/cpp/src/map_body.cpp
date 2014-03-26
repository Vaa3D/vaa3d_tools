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
#include "zfilelist.h"
#include "zstring.h"

using namespace std;

static void map_plane_body(Stack *stack, Intpair_Map *superpixelBodyMap,
    Stack *out, Stack *out2)
{
  size_t nvoxel = Stack_Voxel_Number(stack);

  switch (stack->kind) {
    case GREY16:
      {
        uint16_t *array16 = (uint16_t*) stack->array;
        uint16_t *out_array16 = (uint16_t*) out->array;
        for (size_t k = 0; k < nvoxel; k++) {
          array16[k] = (uint16_t) Intpair_Map_Value(superpixelBodyMap,
              0, array16[k]);
        }
      }
      break;
    case COLOR:
      if (out->kind == COLOR) {
        color_t *arrayc = (color_t*) stack->array;
        color_t *out_arrayc = (color_t*) out->array;
        for (size_t k = 0; k < nvoxel; k++) {
          int value = Intpair_Map_Value(superpixelBodyMap, 0, 
              Color_To_Value(arrayc[k]));
          if (value < 0) {
            out_arrayc[k][0] = 0;
            out_arrayc[k][1] = 0;
            out_arrayc[k][2] = 0;
            out2->array[k] = 0;
          } else {
            out2->array[k] = 
              Value_To_Color(value, out_arrayc[k]);
          }
        }
      } else if (out->kind == GREY16) {
        color_t *arrayc = (color_t*) stack->array;
        uint16_t *out_array16 = (uint16_t*) out->array;
        for (size_t k = 0; k < nvoxel; k++) {
          int value = Intpair_Map_Value(superpixelBodyMap, 0, 
              Color_To_Value(arrayc[k]));
          if (value >= 0) {
            out_array16[k] = value;
          } else {
            out_array16[k] = 0;
          }
        }
      }
      break;
    default:
      break;
  }
}

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> -o <string> --body_map <string>",
    "[--intv <int> <int> <int>]", 
    "[--cluster <int> --script <string> --cmd_dir <string>]",
    "[--range <int> <int>]", NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  char *dataDir = Get_String_Arg(const_cast<char*>("input"));

  //Get the plane range of the stacks
  ZFileList fileList;
  fileList.load(string(dataDir) + "/superpixel_maps", "png", 
      ZFileList::SORT_BY_LAST_NUMBER);
  int startPlane = fileList.startNumber();
  int endPlane = fileList.endNumber();
  int planeNumber = endPlane - startPlane + 1;

  char filePath[500];
  //Load compress map
  sprintf(filePath, "%s/body_compress_map.txt", 
      Get_String_Arg(const_cast<char*>("--body_map")));
  bool compressed = fexist(filePath);
  map<int, int> bodyIdDict;
  if (compressed) {
    FILE *fp = fopen(filePath, "r");
    ZString str;
    while (str.readLine(fp)) {
      vector<int> value = str.toIntegerArray();
      if (value.size() == 2) {
        bodyIdDict[value[0]] = value[1];
      }
    }
    fclose(fp);
  }

  if (Is_Arg_Matched(const_cast<char*>("--cluster"))) {
    FILE *fp = fopen(Get_String_Arg(const_cast<char*>("--script")), "w");
    int planeNumberPerJob = Get_Int_Arg(const_cast<char*>("--cluster"));
    int currentIndex = 0;
    while (currentIndex < planeNumber) {
      if (Is_Arg_Matched(const_cast<char*>("--intv"))) {
        fprintf(fp, "qsub -N map_body -j y -o /dev/null -b y -cwd -V '%s/map_body %s -o %s --body_map %s --intv %d %d %d --range %d %d > %s/map_body_%d_%d.txt'\n",
            Get_String_Arg(const_cast<char*>("--cmd_dir")),
            Get_String_Arg(const_cast<char*>("input")),
            Get_String_Arg(const_cast<char*>("-o")),
            Get_String_Arg(const_cast<char*>("--body_map")),
            Get_Int_Arg(const_cast<char*>("--intv"), 1),
            Get_Int_Arg(const_cast<char*>("--intv"), 2),
            Get_Int_Arg(const_cast<char*>("--intv"), 3),
            currentIndex, currentIndex + planeNumberPerJob - 1,
            Get_String_Arg(const_cast<char*>("-o")),
            currentIndex, currentIndex + planeNumberPerJob - 1);
        currentIndex += planeNumberPerJob;
      } else {
        fprintf(fp, "qsub -N map_body -j y -o /dev/null -b y -cwd -V '%s/map_body %s -o %s --body_map %s --range %d %d > %s/map_body_%d_%d.txt'\n",
            Get_String_Arg(const_cast<char*>("--cmd_dir")),
            Get_String_Arg(const_cast<char*>("input")),
            Get_String_Arg(const_cast<char*>("-o")),
            Get_String_Arg(const_cast<char*>("--body_map")),
            currentIndex, currentIndex + planeNumberPerJob - 1,
            Get_String_Arg(const_cast<char*>("-o")),
            currentIndex, currentIndex + planeNumberPerJob - 1);
        currentIndex += planeNumberPerJob;
      }
    }

    fclose(fp);

    return 0;
  }

  int startPlaneIndex = 0;
  int endPlaneIndex = planeNumber - 1;

  if (Is_Arg_Matched(const_cast<char*>("--range"))) {
    startPlaneIndex = imax2(0, Get_Int_Arg(const_cast<char*>("--range"), 1));
    endPlaneIndex = imin2(endPlaneIndex, 
        Get_Int_Arg(const_cast<char*>("--range"), 2));
  }

  for (int i = startPlaneIndex; i <= endPlaneIndex; i++) {
    int planeId = startPlane + i;
    cout << planeId << endl;

    ZSuperpixelMapArray superpixelMapArray;
    sprintf(filePath, "%s/superpixel_to_body_map%05d.txt", 
        Get_String_Arg(const_cast<char*>("--body_map")), planeId);
    superpixelMapArray.load(filePath, planeId);

    Intpair_Map *superpixelBodyMap = Make_Intpair_Map(100000);
    for (size_t j = 0; j < superpixelMapArray.size(); j++) {
      if (compressed) {
        Intpair_Map_Add(superpixelBodyMap, 0, 
            superpixelMapArray[j].superpixelId(), 
            bodyIdDict[superpixelMapArray[j].bodyId()]);
      } else {
        Intpair_Map_Add(superpixelBodyMap, 0, 
            superpixelMapArray[j].superpixelId(), 
            superpixelMapArray[j].bodyId());
      }
    }
    
    Stack *stack = Read_Stack_U(fileList.getFilePath(i));
    Stack *ds_stack = stack;
    if (Is_Arg_Matched(const_cast<char*>("--intv"))) {
      ds_stack = Downsample_Stack(stack, 
          Get_Int_Arg(const_cast<char*>("--intv"), 1),
          Get_Int_Arg(const_cast<char*>("--intv"), 2), 
          Get_Int_Arg(const_cast<char*>("--intv"), 3));
    }

    Stack *out = NULL;
    Stack *out2 = NULL;
    if (compressed) {
      out = Make_Stack(GREY16, ds_stack->width, ds_stack->height, 1);
    } else {
      out = Make_Stack(COLOR, ds_stack->width, ds_stack->height, 1);
      out2 = Make_Stack(GREY, ds_stack->width, ds_stack->height, 1);
    }
    
    map_plane_body(ds_stack, superpixelBodyMap, out, out2);
    sprintf(filePath, "%s/body_map%05d.tif", 
        Get_String_Arg(const_cast<char*>("-o")), planeId);
    Write_Stack(filePath, out);
    if (out2 != NULL) {
      sprintf(filePath, "%s/a/body_map%05d.tif", 
          Get_String_Arg(const_cast<char*>("-o")), planeId);
      Write_Stack(filePath, out2);
      Free_Stack(out2);
    }

    Free_Stack(stack);
    Free_Stack(out);
    if (ds_stack != stack) {
      Free_Stack(ds_stack);
    }
    Kill_Intpair_Map(superpixelBodyMap);
  }

  printf("map_body succeeded\n");

  return 0;
}
