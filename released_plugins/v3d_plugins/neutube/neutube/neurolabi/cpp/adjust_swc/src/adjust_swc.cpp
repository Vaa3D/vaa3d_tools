#include <iostream>
#include <sstream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <algorithm>

#include "tz_utilities.h"
#include "tz_swc_tree.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "tz_intpair_map.h"
#include "zswctreematcher.h"
#include "zswcbranch.h"
#include "zfilelist.h"
#include "zstring.h"
#include "flyem/zsynapseannotationanalyzer.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> -o <string> --config <string> ",
    "[--xyswitch] [--yflip] [--zcalbr]", NULL};
  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  const char *workDir = Get_String_Arg(const_cast<char*>("input"));
  const char *outDir = Get_String_Arg(const_cast<char*>("-o"));
  ZFileList fileList;
  cout << "Scanning swc files ..." << endl;
  fileList.load(workDir, "swc", ZFileList::NO_SORT);

  if (fileList.size() == 0) {
    cout << "No swc file found." << endl;
    return 1;
  }

  char filePath[500];
  char strBuffer[500];
  FlyEm::ZSynapseAnnotationAnalyzer analyzer;
  analyzer.loadConfig(Get_String_Arg(const_cast<char*>("--config")));

  for (int i = 0; i < fileList.size(); i++) {
    Swc_Tree *tree = Read_Swc_Tree(fileList.getFilePath(i));

    ZString offsetFile = fileList.getFilePath(i);
    offsetFile.replace(".swc", ".tif.offset.txt");
    FILE *fp = fopen(offsetFile.c_str(), "r");

    if (fp == NULL) {
      cout << "Cannot open " << offsetFile << ". No translation." << endl;
    } else {
      ZString offsetStr;
      offsetStr.readLine(fp);
      fclose(fp);
      vector<int> offset = offsetStr.toIntegerArray();

      offset[0] /= analyzer.config().swcDownsample2;
      offset[1] /= analyzer.config().swcDownsample2;

      //Translate swc file
      Swc_Tree_Translate(tree, offset[0], offset[1], offset[2]);
    }

    if (Is_Arg_Matched(const_cast<char*>("--xyswitch"))) {
      Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, false);
      Swc_Tree_Node *tn = NULL;
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
        double tmp = tn->node.x;
        tn->node.x = tn->node.y;
        tn->node.y = tmp;
      }
    }

    if (Is_Arg_Matched(const_cast<char*>("--yflip"))) {
      double maxY = static_cast<double>(analyzer.config().height - 1) /
          analyzer.config().swcDownsample2;
      Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, false);
      Swc_Tree_Node *tn = NULL;
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
        tn->node.y = maxY - tn->node.y;
      }
    }

    if (Is_Arg_Matched(const_cast<char*>("--zcalbr"))) {
      Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, false);
      Swc_Tree_Node *tn = NULL;
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
        tn->node.z = tn->node.z - analyzer.config().startNumber;
      }
    }

    int overallDownsample = analyzer.config().swcDownsample1 *
            analyzer.config().swcDownsample2;

    //Scale swc file
    Swc_Tree_Resize(tree, analyzer.config().xResolution * overallDownsample,
                    analyzer.config().yResolution * overallDownsample,
                    analyzer.config().zResolution, TRUE);

    //Save Swc file
    fullpath_e(outDir, fname(fileList.getFilePath(i), strBuffer), "swc", 
        filePath);
    Write_Swc_Tree(filePath, tree);
    cout << filePath << " saved" << endl;

    Kill_Swc_Tree(tree);
  }


  return 0;
}
