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
#include "zargumentprocessor.h"
#include "zswctreematcher.h"

using namespace std;

int color_to_value(const color_t &color)
{
  int value = color[2];
  value *= 256;
  value += color[1];
  value *= 256;
  value += color[0]; 

  return value;
}

void value_to_color(int value, color_t &color)
{
  if (value < 0) {
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
  } else {
    int color2 = value / 256;
    int color1 = color2 % 256;
    color2 /= 256;
    int color0 = value % 256;
    color[0] = color0;
    color[1] = color1;
    color[2] = color2;
  }
}

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"[<input:string>] [-o <string>]",
    "[--body_map <string>]", "[--plane_id <int>]", "[-t]", NULL};

  //Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  ZArgumentProcessor::processArguments(argc, argv, Spec);

  if (ZArgumentProcessor::isArgMatched("-t")) {
    cout << "testing ..." << endl;

    if (ZArgumentProcessor::isArgMatched("--plane_id")) {
      cout << "Plane ID: " << ZArgumentProcessor::getIntArg("--plane_id")
        << endl;
    }

    std::vector<ZSwcTree*> treeArray =
      ZSwcTree::loadTreeArray("/Users/zhaot/Work/neutube/neurolabi/data/"
          "tmp/result/swc2/adjusted");

    std::ofstream fstream2(
        "/Users/zhaot/Work/neutube/neurolabi/data/trees.txt");
    for (size_t i = 0; i < treeArray.size(); i++) {
      fstream2 << treeArray[i]->source() << endl;
    }
    fstream2.close();

          /*
    std::vector<ZSwcTree*> treeArray =
      ZSwcTree::loadTreeArray("/Users/zhaot/Work/neutube/neurolabi/data/"
          "tmp/compare");
*/
    //Pairwise comparison of the trees
    ZSwcTreeMatcher matcher;
    matcher.setShollAnalysisParameters(10, 110, 20);
    std::vector<std::vector<double> > scoreMatrix =
      matcher.computePairwiseMatchingScore(treeArray);

    std::ofstream fstream("/Users/zhaot/Work/neutube/neurolabi/data/match2.txt");

    //Output results
    for (size_t i = 0; i < scoreMatrix.size() - 1; i++) {
      for (size_t j = 0; j < scoreMatrix[i].size(); j++) {
        fstream << scoreMatrix[i][j] << ", ";
        cout << scoreMatrix[i][j] << ", ";

      }
      fstream << endl;
      cout << endl;
    }

    fstream.close();

    return 0;
  }
    
  
  Stack *stack = Read_Stack_U(Get_String_Arg(const_cast<char*>("input")));
  Stack *mask = Copy_Stack(stack);
  Zero_Stack(mask);

  int planeId = Get_Int_Arg(const_cast<char*>("--plane_id"));
  ZSuperpixelMapArray superpixelMapArray;
  superpixelMapArray.load(Get_String_Arg(const_cast<char*>("--body_map")),
      planeId);

  /*
     map<int, int> superpixelBodyMap;
     for (size_t i = 0; i < superpixelMapArray.size(); i++) {
     superpixelBodyMap[superpixelMapArray[i].superpixelId()] =
     superpixelMapArray[i].bodyId();
     }
  */
  Intpair_Map *superpixelBodyMap = Make_Intpair_Map(100000);
  for (size_t i = 0; i < superpixelMapArray.size(); i++) {
    Intpair_Map_Add(superpixelBodyMap, 0, superpixelMapArray[i].superpixelId(),
        superpixelMapArray[i].bodyId());
  }

  size_t nvoxel = Stack_Voxel_Number(stack);

  switch (stack->kind) {
    case GREY16:
      {
        uint16_t *array16 = (uint16_t*) stack->array;
        uint16_t *out_array16 = (uint16_t*) mask->array;
        for (size_t k = 0; k < nvoxel; k++) {
          //array16[k] = (uint16_t) superpixelBodyMap[array16[k]];
          array16[k] = (uint16_t) Intpair_Map_Value(superpixelBodyMap,
              0, array16[k]);
        }
      }
      break;
    case COLOR:
      {
        color_t *arrayc = (color_t*) stack->array;
        color_t *out_arrayc = (color_t*) mask->array;
        for (size_t k = 0; k < nvoxel; k++) {
          /*
             value_to_color(superpixelBodyMap[color_to_value(arrayc[k])],
             out_arrayc[k]);
             */
          value_to_color(Intpair_Map_Value(superpixelBodyMap, 0, 
                color_to_value(arrayc[k])), out_arrayc[k]);
        }
      }
      break;
    default:
      break;
  }

  Write_Stack_U(Get_String_Arg(const_cast<char*>("-o")), mask, NULL);

  return 0;
}
