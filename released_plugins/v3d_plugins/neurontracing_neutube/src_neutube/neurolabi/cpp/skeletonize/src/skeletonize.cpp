/**@file skeletonize.cpp
 * @author Ting Zhao
 */
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
#include "tz_stack_math.h"
#include "tz_stack_lib.h"
#include "tz_stack_stat.h"
#include "zstackskeletonizer.h"
#include "c_stack.h"
#include "zobject3d.h"
#include "zobject3dscan.h"
#include "zfiletype.h"
#include "zstring.h"
#include "swc/zswcresampler.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.2") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> [-o <string>]",
                               "[--intv <int> <int> <int>]",
                               "[--minlen <int(15)>] [--maxdist <int(50)>]",
                               "[--minobj <int(0)>]",
                               "[--keep_short]",
                               "[--interpolate] [--rmborder]",
                               "[--rebase]", "[--level <int>]",  NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  char *input = Get_String_Arg(const_cast<char*>("input"));

  /* Skeletonization */
  cout << "Read stack ...\n" << endl;

  Stack *stack = NULL;
  int offset[3] = { 0, 0, 0 };

  if (ZFileType::fileType(input) == ZFileType::OBJECT_SCAN_FILE) {
    ZObject3dScan obj;
    if (obj.load(input)) {
      stack = obj.toStack(offset);
      //ZObject3d *obj3d = obj.toObject3d();
      //stack = obj3d->toStack(offset);
    }
  } else {
    stack = C_Stack::readSc(input);
  }

  //Stack stackData;
  //C_Stack::view(stack, &stackData, 0);
  //stackData.text = strdup("");

  if (Is_Arg_Matched(const_cast<char*>("--level"))) {
    Stack_Binarize_Level(stack, Get_Int_Arg(const_cast<char*>("--level")));
    if (C_Stack::kind(stack) == GREY16) {
      Translate_Stack(stack, GREY, 1);
    }
  } else {
    cout << "Binarizing ..." << endl;
    Stack_Binarize(stack);
  }

  if (Stack_Max(stack, NULL) != 1) {
    cout << "Not a binary image. Abort" << endl;
    return 1;
  }

  if (Is_Arg_Matched(const_cast<char*>("--intv"))) {
    cout << "Downsampling ..." << endl;
    Stack *bufferStack = Downsample_Stack_Max(
          stack, Get_Int_Arg(const_cast<char*>("--intv"), 1),
          Get_Int_Arg(const_cast<char*>("--intv"), 2),
          Get_Int_Arg(const_cast<char*>("--intv"), 3),
          NULL);
    for (int i = 0; i < 3; ++i) {
      offset[i] /= (Get_Int_Arg(const_cast<char*>("--intv"), i + 1) + 1);
    }
    C_Stack::kill(stack);
    stack = bufferStack;
  }

  if (Is_Arg_Matched(const_cast<char*>("--rmborder"))) {
    printf("Remove borders ...\n");
    Stack_Not(stack, stack);
    Stack* solid = Stack_Majority_Filter(stack, NULL, 8);
    C_Stack::kill(stack);

    Stack_Not(solid, solid);
    stack = solid;
  }

  if (Is_Arg_Matched(const_cast<char*>("--interpolate"))) {
    printf("Interpolating ...\n");
    Stack *bufferStack = Stack_Bwinterp(stack, NULL);
    C_Stack::kill(stack);
    stack = bufferStack;
  }


  ZStackSkeletonizer skeletonizer;
  if (Is_Arg_Matched(const_cast<char*>("--rebase"))) {
    skeletonizer.setRebase(true);
  } else {
    skeletonizer.setRebase(false);
  }

  skeletonizer.setMinObjSize(Get_Int_Arg(const_cast<char*>("--minobj")));
  skeletonizer.setDistanceThreshold(Get_Int_Arg(const_cast<char*>("--maxdist")));
  skeletonizer.setLengthThreshold(Get_Int_Arg(const_cast<char*>("--minlen")));

  if (Is_Arg_Matched(const_cast<char*>("--keep_short"))) {
    skeletonizer.setKeepingSingleObject(true);
  } else {
    skeletonizer.setKeepingSingleObject(false);
  }

  ZSwcTree *wholeTree = skeletonizer.makeSkeleton(stack);

  if (wholeTree != NULL) {
    ZSwcResampler resampler;
    resampler.optimalDownsample(wholeTree);
    ZString outputPath = Get_String_Arg(const_cast<char*>("-o"));
    wholeTree->save(outputPath.c_str());
    cout << outputPath << " saved" << endl;
    outputPath.replace(".swc", ".offset.txt");
    ofstream stream(outputPath.c_str());
    stream << offset[0] << " " << offset[1] << " " << offset[2];
    stream.close();
  } else {
    std::cout << "Empty tree. No file saved";
    return 1;
  }

  return 0;
}
