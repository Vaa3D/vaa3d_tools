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
#include "flyem/zflyembodyanalyzer.h"
#include "zargumentprocessor.h"
#include "zstackgraph.h"
#include "zgraphcompressor.h"
#include "zgraph.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {
    "<input:string> [-o <string>]",
    "[--landmark <string>]", "[--intv <int> <int> <int>]",
    "[--minloop <int(100)>]", "[--bound_adjust]",
    NULL};

  ZArgumentProcessor::processArguments(argc, argv, Spec);

  const char *input =
          ZArgumentProcessor::getStringArg(const_cast<char*>("input"));

  cout << "Loading object ..." << endl;

  ZObject3dScan obj;

  if (ZFileType::fileType(input) == ZFileType::OBJECT_SCAN_FILE) {
    obj.load(input);
  } else if (ZFileType::fileType(input) == ZFileType::TIFF_FILE) {
    ZStack stack;
    stack.load(input);
    if (stack.channelNumber() > 1) {
      cout << "  More than one channel found. Only the first channel will be loaded" << endl;
    }
    Stack_Binarize(stack.c_stack(0));
    obj.loadStack(stack.c_stack(0));
  } else {
    cout << "Invalid input. Abort." << endl;
    return 1;
  }

  if (!obj.isEmpty()) {
    if (ZArgumentProcessor::isArgMatched("--landmark")) {
      ZPointArray pts;
      const char *landmark = ZArgumentProcessor::getStringArg("--landmark");
      ZFlyEmBodyAnalyzer bodyAnalyzer;
      int intv[3] = {0, 0, 0};
      bool usingDefaultIntv = true;
      if (ZArgumentProcessor::isArgMatched("--intv")) {
        for (int i = 0; i < 3; ++i) {
          intv[i] = ZArgumentProcessor::getIntArg("--intv", i + 1);
        }
        usingDefaultIntv = false;
      }

      if (eqstr(landmark, "hole")) {
        if (usingDefaultIntv) {
          intv[0] = 1;
          intv[1] = 1;
          intv[2] = 0;
        }

        bodyAnalyzer.setDownsampleInterval(intv[0], intv[1], intv[2]);

        cout << "Examining holes ..." << endl;
        pts = bodyAnalyzer.computeHoleCenter(obj);
      } else if (eqstr(landmark, "terminal")) {
        if (usingDefaultIntv) {
          intv[0] = 1;
          intv[1] = 1;
          intv[2] = 1;
        }

        bodyAnalyzer.setDownsampleInterval(intv[0], intv[1], intv[2]);

        cout << "Examining termini ..." << endl;
        pts = bodyAnalyzer.computeTerminalPoint(obj);
      } else if (eqstr(landmark, "loop")) {
        if (usingDefaultIntv) {
          intv[0] = 1;
          intv[1] = 1;
          intv[2] = 1;
        }

        bodyAnalyzer.setDownsampleInterval(intv[0], intv[1], intv[2]);
        bodyAnalyzer.setMinLoopSize(ZArgumentProcessor::getIntArg("--minloop"));
        pts = bodyAnalyzer.computeLoopCenter(obj);
      } else {
        cout << "Invalid landmark, which must be one one of the following:" << endl;
        cout << "  hole, terminal" << endl;
        return 1;
      }

      if (!pts.empty()) {
        cout << "Saving results ...";
        if (ZArgumentProcessor::isArgMatched("--bound_adjust")) {
          ZCuboid box = obj.getBoundBox();
          ZPoint corner = box.firstCorner();
          corner *= -1;
          pts.translate(corner);
        }

        std::string output = ZArgumentProcessor::getStringArg("-o");
        switch (ZFileType::fileType(output)) {
        case ZFileType::SWC_FILE:
            pts.exportSwcFile(output, 3.0);
            break;
        case ZFileType::JSON_FILE:
            pts.exportJsonFile(output);
            break;
        default:
            cout << "Unknown output format. Saved as csv file" << endl;
            pts.exportCsvFile(output);
            break;
        }

        cout << output << " saved." << endl;
      } else {
        cout << "No landmark found." << endl;
        return 0;
      }
    }
  } else {
    cout << "Invalid or empty object. Abort." << endl;
    return 1;
  }

  return 0;
}
