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
#include "zdoublevector.h"
#include "zfiletype.h"
#include "zstring.h"
#include "swc/zswcresampler.h"
#include "zargumentprocessor.h"
#include "zswcglobalfeatureanalyzer.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {
    "<input:string> [-o <string>]",
    "[--feature <string>]",
    NULL};

  ZArgumentProcessor::processArguments(argc, argv, Spec);

  const char *input =
          ZArgumentProcessor::getStringArg(const_cast<char*>("input"));

  cout << "Loading SWC ..." << endl;

  if (ZFileType::fileType(input) == ZFileType::SWC_FILE) {
    ZSwcTree tree;
    tree.load(input);
    if (tree.isEmpty()) {
      cout << "Invalid or empty swc file. Abort." << endl;
      return 1;
    }

    if (ZArgumentProcessor::isArgMatched("--feature")) {
      ZSwcGlobalFeatureAnalyzer featureAnalyzer;
      ZSwcGlobalFeatureAnalyzer::EFeatureSet setName =
          ZSwcGlobalFeatureAnalyzer::UNDEFINED_NGF;
      if (eqstr(ZArgumentProcessor::getStringArg("--feature"), "NGF1")) {
        setName = ZSwcGlobalFeatureAnalyzer::NGF1;
      }
      std::vector<double> featureSet =
          featureAnalyzer.computeFeatureSet(tree, setName);
      ZDoubleVector::print(featureSet);
    }
  } else {
    cout << "The input is not a sparse object. Abort." << endl;
    return 1;
  }

  return 0;
}
