#include <iostream>
#include <sstream>
#include <fstream>
#include <set>

#include <string.h>
#include <stdlib.h>

#include "tz_utilities.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "tz_stack_attribute.h"
#include "tz_swc_tree.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "tz_sp_grow.h"
#include "zspgrowparser.h"
#include "tz_stack_stat.h"
#include "tz_intpair_map.h"
#include "tz_stack_utils.h"
#include "zstring.h"
#include "zargumentprocessor.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> -o <string>", NULL};

  ZArgumentProcessor::processArguments(argc, argv, Spec);

  ZString filePath = ZArgumentProcessor::getStringArg("input");

  Stack *stack = Read_Stack_U(filePath.c_str());
  Stack *additionalStack = NULL;
  ZString additionalFilePath =
      filePath.toDirPath() + "/a/" + filePath.toFileName();
  if (fexist(additionalFilePath.c_str())) {
    additionalStack = Read_Stack_U(additionalFilePath.c_str());
  }
    
  std::set<uint32_t> bodyIdSet;
  size_t voxelNumber = Stack_Voxel_Number(stack);

  if (stack->kind == COLOR) {
    color_t *arrayc = (color_t*) stack->array;
    for (size_t offset = 0; offset < voxelNumber; offset++) {
      uint32_t tmpBodyId = (int) Color_To_Value(arrayc[offset]);
      if (additionalStack != NULL) {
        uint32_t value = additionalStack->array[offset];
        tmpBodyId += (int) (value << 24);
      }
      bodyIdSet.insert(tmpBodyId);
    }
  } else {
    uint16_t *array16 = (uint16_t*) stack->array;
    for (size_t offset = 0; offset < voxelNumber; offset++) {
      uint32_t tmpBodyId = array16[offset];
      if (additionalStack != NULL) {
        uint32_t value = additionalStack->array[offset];
        tmpBodyId += (int) (value << 24);
      }
      bodyIdSet.insert(tmpBodyId);
    }
  }

  cout << bodyIdSet.size() << " ids" << endl;

  ofstream stream(ZArgumentProcessor::getStringArg("-o"));

  for (std::set<uint32_t>::const_iterator iter = bodyIdSet.begin();
       iter != bodyIdSet.end(); ++iter) {
    stream << *iter << endl;
  }

  stream.close();

  cout << "Results saved into " << ZArgumentProcessor::getStringArg("-o")
       << endl;

  return 0;
}
