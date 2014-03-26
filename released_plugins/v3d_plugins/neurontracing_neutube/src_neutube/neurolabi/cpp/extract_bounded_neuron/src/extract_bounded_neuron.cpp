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
#include "zobject3dscan.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.2") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> -o <string>",
    "--body_id <int> [--sorted_map <string>] [--isolate <int> <int>]", 
    "[--ds <int>]",
    "[--final_ds <int>]",
    "[--compress <string>] [--sobj] [--sobj_intv <int> <int> <int>]", NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  char *dataDir = Get_String_Arg(const_cast<char*>("input"));
  int bodyId = Get_Int_Arg(const_cast<char*>("--body_id"));

  if (Is_Arg_Matched(const_cast<char*>("--sobj"))) {
    ZString objPath = dataDir;
    objPath += "/";
    objPath.appendNumber(bodyId);
    objPath += ".sobj";
    if (!fexist(objPath.c_str())) {
      return 1;
    }

    ZObject3dScan obj;
    obj.load(objPath);

    if (Is_Arg_Matched(const_cast<char*>("--sobj_intv"))) {
      int xintv = Get_Int_Arg(const_cast<char*>("--sobj_intv"), 1);
      int yintv = Get_Int_Arg(const_cast<char*>("--sobj_intv"), 2);
      int zintv = Get_Int_Arg(const_cast<char*>("--sobj_intv"), 3);
      obj.downsampleMax( xintv, yintv, zintv);
    }

    int offset[3];
    Stack *stack = obj.toStack(offset);

    Write_Stack(Get_String_Arg(const_cast<char*>("-o")), stack);

    char filePath[500];
    sprintf(filePath, "%s.offset.txt", Get_String_Arg(const_cast<char*>("-o")));
    FILE *fp = fopen(filePath, "w");
    fprintf(fp, "%d %d %d", offset[0], offset[1], offset[2]);
    fclose(fp);

    return 0;
  }

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

  ZFileList additionalFileList;
  additionalFileList.load(string(dataDir) + "/a", "tif",
                          ZFileList::SORT_BY_LAST_NUMBER);

  int r = 250;
  int dr = 75;

  if (Is_Arg_Matched(const_cast<char*>("--isolate"))) {
    r = Get_Int_Arg(const_cast<char*>("--isolate"), 1);
    dr = Get_Int_Arg(const_cast<char*>("--isolate"), 2);
  }

  Stack *stack = Read_Stack_U(fileList.getFilePath(0));

  int intv[3] = {0, 0, 0};
  if (Is_Arg_Matched(const_cast<char*>("--ds"))) {
    intv[0] = Get_Int_Arg(const_cast<char*>("--ds")) - 1;
    intv[1] = intv[0];
    //intv[1] = Get_Int_Arg(const_cast<char*>("--ds"), 2) - 1;
    //intv[2] = Get_Int_Arg(const_cast<char*>("--ds"), 3) - 1;
    stack = Downsample_Stack(stack, intv[0], intv[1], intv[2]);
    r /= intv[0] + 1;
    dr /= intv[0] + 1;
  }

  int width = stack->width;
  int height = stack->height;

  Stack *mip = Make_Stack(GREY, width, height, 1);
  Zero_Stack(mip);
  int planeSize = Stack_Voxel_Number(mip);


  char filePath[500];
  Kill_Stack(stack);

  int zStart = -1;
  int zEnd = 0;

  int initialZStart = -1;
  int initialZEnd = planeNumber - 1;

  if (Is_Arg_Matched(const_cast<char*>("--sorted_map"))) {
    ZFileList mapFileList;
    mapFileList.load(Get_String_Arg(const_cast<char*>("--sorted_map")), "txt",
                     ZFileList::SORT_BY_LAST_NUMBER);
    if (mapFileList.size() != planeNumber) {
      cout << "No correct sorted map found. Abort.";
      return 1;
    }

    for (int i = 0; i < mapFileList.size(); ++i) {
      FILE *fp = fopen(mapFileList.getFilePath(i), "r");
      String_Workspace *sw = New_String_Workspace();
      char *line = NULL;
      int valueArray[10];
      int n;
      cout << i << endl;
      while ((line = Read_Line(fp, sw)) != NULL) {
        String_To_Integer_Array(line, valueArray, &n);
        if (n == 4) {
          if (valueArray[3] == bodyId) {
            cout << line << endl;
            if (initialZStart < 0) {
              initialZStart = i;
            }
            initialZEnd = i;
            break;
          }
        }
      }
      fclose(fp);
    }
  }

  if (initialZStart < 0) {
    initialZStart = 0;
  }

  //Estimate Z range
  cout << "Estimating Z range ..." << endl;
  for (int i = initialZStart; i <= initialZEnd; i++) {
    cout << "  " << i << "/" << planeNumber << endl;
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
          mip->array[offset] = 1;
        }
      }
    } else {
      for (int offset = 0; offset < planeSize; offset++) {
        if (array16[offset] == bodyId) {
          bodyArea++;
          mip->array[offset] = 1;
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

  cout << "  " << zStart << " " << zEnd << endl;

  cout << "Removing isolated objects ..." << endl;
  Stack *mipMask = Stack_Remove_Isolated_Object(mip, NULL, r, dr);

  cout << "Estimating bound box ..." << endl;
  Cuboid_I boundBox;
  Stack_Bound_Box(mipMask, &boundBox);
  cout << "  (" << boundBox.cb[0] << "," <<  boundBox.cb[1] << "," << 
    boundBox.cb[2] << ")" << "->" << "(" << boundBox.ce[0] << "," <<
    boundBox.ce[1] << "," << boundBox.ce[2] << ")" << endl;

  int finalDs = 1;

  if (Is_Arg_Matched(const_cast<char*>("--final_ds"))) {
    finalDs = Get_Int_Arg(const_cast<char*>("--final_ds"));
  }

  int cropWidth = boundBox.ce[0] - boundBox.cb[0] + 1;
  int cropHeight = boundBox.ce[1] - boundBox.cb[1] + 1;

  int finalStackWidth, finalStackHeight, finalStackDepth;
  Downsample_Stack_Max_Size(cropWidth, cropHeight, 1, finalDs, finalDs, 1,
      &finalStackWidth, &finalStackHeight, &finalStackDepth);

  Stack *finalStack = Make_Stack(GREY, finalStackWidth, finalStackHeight,
      zEnd - zStart + 1);
  int finalPlaneArea = finalStackHeight * finalStackWidth;

  uint8_t *outArray = finalStack->array;

  Stack *bodyPlane = Make_Stack(GREY, mipMask->width, mipMask->height, 1);
  Stack *croppedBodyPlane = Make_Stack(GREY, cropWidth, cropHeight, 1);

  //Load images
  cout << "Extract planes ..." << endl;
  for (int i = zStart; i <= zEnd; i++) {
    cout << "  " << i << "/" << zEnd << endl;
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
    }

    color_t *arrayc = (color_t*) stack->array;
    uint16_t *array16 = (uint16_t*) stack->array;
    
    if (stack->kind == COLOR) {
      for (int offset = 0; offset < planeSize; offset++) {
        bodyPlane->array[offset] = 0;
      }

      for (int y = boundBox.cb[1]; y <= boundBox.ce[1]; ++y) {
        size_t offset = y * bodyPlane->width + boundBox.cb[0];
        for (int x = boundBox.cb[0]; x <= boundBox.ce[0]; ++x) {
          int tmpBodyId = (int) Color_To_Value(arrayc[offset]);
          if (additionalStack != NULL) {
            uint32_t value = additionalStack->array[offset];
            tmpBodyId += (int) (value << 24);
          }

          if (tmpBodyId == bodyId && mipMask->array[offset] == 1) {
            bodyPlane->array[offset] = 1;
          }
          ++offset;
        }
      }

      /*
      for (int offset = 0; offset < planeSize; offset++) {
        int tmpBodyId = (int) Color_To_Value(arrayc[offset]);
        if (additionalStack != NULL) {
          uint32_t value = additionalStack->array[offset];
          tmpBodyId += (int) (value << 24);
        }

        if (tmpBodyId == bodyId && mipMask->array[offset] == 1) {
          bodyPlane->array[offset] = 1;
        } else {
          bodyPlane->array[offset] = 0;
        }
      }
      */
    } else {
      for (int offset = 0; offset < planeSize; offset++) {
        if (array16[offset] == bodyId && 
            mipMask->array[offset] == 1) {
          bodyPlane->array[offset] = 1;
        } else {
          bodyPlane->array[offset] = 0;
        }
      }
    }

    Stack dest;
    dest.width = finalStack->width;
    dest.height = finalStack->height;
    dest.depth = 1;
    dest.kind = GREY;
    dest.array = outArray;

    Crop_Stack(bodyPlane, boundBox.cb[0], boundBox.cb[1], 0,
        cropWidth, cropHeight, 1, croppedBodyPlane);

    //Downsample the plane
    Downsample_Stack_Max(croppedBodyPlane, finalDs, finalDs, 0, &dest);
    
    outArray += finalPlaneArea;

    Free_Stack(stack);
    if (additionalStack != NULL) {
      Free_Stack(additionalStack);
    }
  }

  //finalStack = Downsample_Stack_Max(finalStack, 1, 1, 0);
  Write_Stack(Get_String_Arg(const_cast<char*>("-o")), finalStack);

  sprintf(filePath, "%s.offset.txt", Get_String_Arg(const_cast<char*>("-o")));
  FILE *fp = fopen(filePath, "w");
  fprintf(fp, "%d %d %d", boundBox.cb[0], boundBox.cb[1], zStart);
  fclose(fp);

  return 0;
}
