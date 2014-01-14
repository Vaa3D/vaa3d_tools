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

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> -o <string>",
    "--body_id <int> [--ds <int>] [--range <int> <int>]",
    "[--compress <string>] --grey_dir <string>",
    "[--cluster <int>] [--script <string>] [--cmd_dir <string>]", 
    NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  char *dataDir = Get_String_Arg(const_cast<char*>("input"));
  int bodyId = Get_Int_Arg(const_cast<char*>("--body_id"));
  char *greyDir = Get_String_Arg(const_cast<char*>("--grey_dir"));
  char *outDir = Get_String_Arg(const_cast<char*>("-o"));

  char filePath[500];

  //Load compress map
  if (Is_Arg_Matched(const_cast<char*>("--compress"))) {
    FILE *fp = fopen(Get_String_Arg(const_cast<char*>("--compress")), "r");
    ZString str;
    while (str.readLine(fp)) {
      vector<int> value = str.toIntegerArray();
      if (value.size() == 2) {
        if (value[0] == bodyId) {
          bodyId = value[1];
          break;
        }
      }
    }
    fclose(fp);
  }


  ZFileList fileList;
  fileList.load(dataDir, "tif", ZFileList::SORT_BY_LAST_NUMBER);
  int startPlane = fileList.startNumber();
  int endPlane = fileList.endNumber();
  int planeNumber = endPlane - startPlane + 1;

  if (Is_Arg_Matched(const_cast<char*>("--cluster"))) {
    FILE *fp = fopen(Get_String_Arg(const_cast<char*>("--script")), "w");
    int planeNumberPerJob = Get_Int_Arg(const_cast<char*>("--cluster"));
    int currentIndex = 0;
    while (currentIndex < planeNumber) {
      fprintf(fp, "qsub -P flyemproj -pe batch 4 -N extract_em -j y -o /dev/null -b y -cwd -V '%s/extract_em_neuron %s -o %s --body_id %d --grey_dir %s --range %d %d > %s/extract_em_neuron_%d_%d.txt'\n",
          Get_String_Arg(const_cast<char*>("--cmd_dir")),
          Get_String_Arg(const_cast<char*>("input")),
          Get_String_Arg(const_cast<char*>("-o")),
          Get_Int_Arg(const_cast<char*>("--body_id")),
          Get_String_Arg(const_cast<char*>("--grey_dir")),
          currentIndex, currentIndex + planeNumberPerJob - 1,
          Get_String_Arg(const_cast<char*>("-o")),
          currentIndex, currentIndex + planeNumberPerJob - 1);
      currentIndex += planeNumberPerJob;
    }

    fclose(fp);

    return 0;
  }

  ZFileList additionalFileList;
  additionalFileList.load(string(dataDir) + "/a", "tif", 
      ZFileList::SORT_BY_LAST_NUMBER);
    
  Stack *stack = Read_Stack_U(fileList.getFilePath(0));

  int intv[3] = {0, 0, 0};
  if (Is_Arg_Matched(const_cast<char*>("--ds"))) {
    intv[0] = Get_Int_Arg(const_cast<char*>("--ds")) - 1;
    intv[1] = intv[0];
    stack = Downsample_Stack(stack, intv[0], intv[1], intv[2]);
  }

  int width = stack->width;
  int height = stack->height;

  int planeSize = width * height;


  Kill_Stack(stack);

  int zStart = -1;
  int zEnd = 0;

  //Estimate Z range
  if (Is_Arg_Matched(const_cast<char*>("--range"))) {
    zStart = imax2(0, Get_Int_Arg(const_cast<char*>("--range"), 1));
    zEnd = imin2(planeNumber - 1, 
        Get_Int_Arg(const_cast<char*>("--range"), 2));
  } else {
    for (int i = 0; i < planeNumber; i++) {
      int bodyArea = 0;

      stack = Read_Stack_U(fileList.getFilePath(i));
      Stack *additionalStack = NULL;
      if (additionalFileList.size() > 0) {
        additionalStack = Read_Stack_U(additionalFileList.getFilePath(i));
      }

      if (Is_Arg_Matched(const_cast<char*>("--ds"))) {
        Stack *stack2 = Downsample_Stack(stack, intv[0], intv[1], intv[2]);
        Free_Stack(stack);
        stack = stack2;

        if (additionalStack != NULL) {
          stack2 = Downsample_Stack(additionalStack, intv[0], intv[1], intv[2]);
          Free_Stack(additionalStack);
          additionalStack = stack2;
        }
      }
      color_t *arrayc = (color_t*) stack->array;
      uint16_t *array16 = (uint16_t*) stack->array;

      if (stack->kind == COLOR) {
        for (int offset = 0; offset < planeSize; offset++) {
          int tmpBodyId = (int) Color_To_Value(arrayc[offset]);
          if (additionalStack != NULL) {
            uint32_t value = additionalStack->array[offset];
            tmpBodyId += (int) (value << 24);
          }
          if (tmpBodyId == bodyId) {
            bodyArea++;
          }
        }
      } else {
        for (int offset = 0; offset < planeSize; offset++) {
          if (array16[offset] == bodyId) {
            bodyArea++;
          }
        }
      }

      if (bodyArea > 0) {
        if (zStart < 0) {
          zStart = i;
        }
        zEnd = i;
      }

      Free_Stack(stack);
      if (additionalStack != NULL) {
        Free_Stack(additionalStack);
      }
    }
  }

  ZFileList greyFileList;
  greyFileList.load(greyDir, "raw",ZFileList::SORT_BY_LAST_NUMBER);

  //Load images
  for (int i = zStart; i <= zEnd; i++) {
    Stack *greyStack = Read_Flyem_Raw_Stack_Plane(greyFileList.getFilePath(i));

    stack = Read_Stack(fileList.getFilePath(i));
    Stack *additionalStack = NULL;
    if (additionalFileList.size() > 0) {
      additionalStack = Read_Stack_U(additionalFileList.getFilePath(i));
    }

    if (Is_Arg_Matched(const_cast<char*>("--ds"))) {
      Stack *stack2 = Downsample_Stack(stack, intv[0], intv[1], intv[2]);
      Free_Stack(stack);
      stack = stack2;

      if (additionalStack != NULL) {
        stack2 = Downsample_Stack(additionalStack, intv[0], intv[1], intv[2]);
        Free_Stack(additionalStack);
        additionalStack = stack2;
      }

      stack2 = Downsample_Stack(greyStack, intv[0], intv[1], intv[2]);
      Free_Stack(greyStack);
      greyStack = stack2;
    }

    color_t *arrayc = (color_t*) stack->array;
    uint16_t *array16 = (uint16_t*) stack->array;
    
    if (stack->kind == COLOR) {
      for (int offset = 0; offset < planeSize; offset++) {
        int tmpBodyId = (int) Color_To_Value(arrayc[offset]);
        if (additionalStack != NULL) {
          uint32_t value = additionalStack->array[offset];
          tmpBodyId += (int) (value << 24);
        }

        if (tmpBodyId != bodyId) {
          greyStack->array[offset] = 0;
        }      
      }
    } else {
      for (int offset = 0; offset < planeSize; offset++) {
        if (array16[offset] != bodyId) {
          greyStack->array[offset] = 0;
        }
      }
    }

    
    sprintf(filePath, "%s/grey%05d.tif", outDir, startPlane + i);
    std::cout << "saving " << filePath << std::endl;
    Write_Stack(filePath, greyStack);

    Free_Stack(greyStack);
    Free_Stack(stack);
    if (additionalStack != NULL) {
      Free_Stack(additionalStack);
    }
  }

  return 0;
}
