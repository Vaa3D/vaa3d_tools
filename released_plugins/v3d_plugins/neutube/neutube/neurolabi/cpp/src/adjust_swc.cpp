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
    NULL};
  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  const char *workDir = Get_String_Arg(const_cast<char*>("input"));
  const char *outDir = Get_String_Arg(const_cast<char*>("-o"));
  ZFileList fileList;
  fileList.load(workDir, "swc", ZFileList::NO_SORT);

  char filePath[500];
  char strBuffer[500];
  for (int i = 0; i < fileList.size(); i++) {
    Swc_Tree *tree = Read_Swc_Tree(fileList.getFilePath(i));

    ZString offsetFile = fileList.getFilePath(i);
    offsetFile.replace(".swc", ".tif.offset.txt");
    FILE *fp = fopen(offsetFile.c_str(), "r");
    ZString offsetStr;
    offsetStr.readLine(fp);
    fclose(fp);
    vector<int> offset = offsetStr.toIntegerArray();
    
    FlyEm::ZSynapseAnnotationAnalyzer analyzer;
    analyzer.loadConfig(Get_String_Arg(const_cast<char*>("--config")));

    offset[0] /= analyzer.config().swcDownsample2;
    offset[1] /= analyzer.config().swcDownsample2;

    //Translate swc file
    Swc_Tree_Translate(tree, offset[0], offset[1], offset[2]);

    //Scale swc file
    Swc_Tree_Resize(tree, analyzer.config().xResolution, 
        analyzer.config().yResolution, analyzer.config().zResolution, FALSE);
    
    //Save Swc file
    fullpath_e(outDir, fname(fileList.getFilePath(i), strBuffer), "swc", 
        filePath);
    Write_Swc_Tree(filePath, tree);
    cout << filePath << " saved" << endl;

    Kill_Swc_Tree(tree);
  }


  return 0;
}
