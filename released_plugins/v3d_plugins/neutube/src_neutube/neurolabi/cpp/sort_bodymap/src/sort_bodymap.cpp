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
#include "zstring.h"
#include "zfilelist.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> -o <string> [--compress]", NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  char *dataDir = Get_String_Arg(const_cast<char*>("input"));

  if (!dexist(dataDir)) {
    printf("The input directory %s does not exist.\n", dataDir);

    return 1;
  }

  //Get the plane range of the stacks
  ZFileList fileList;
  fileList.load(string(dataDir) + "/superpixel_maps", "png", 
      ZFileList::SORT_BY_LAST_NUMBER);
  int startPlane = fileList.startNumber();
  int endPlane = fileList.endNumber();
  int planeNumber = endPlane - startPlane + 1;

  cout << startPlane << " -- " << endPlane << endl;

  //For compressed body id
  map<int, int> bodyIdDict;
  int bodyNumber = 0;

  //Create segment-body pairs from segment to body map file
  Intpair_Map *segmentBodyMap = Make_Intpair_Map(10000 * planeNumber);
  FILE *fp = fopen((string(dataDir) + "/segment_to_body_map.txt").c_str(), "r");

  if (fp == NULL) {
    printf("Open %s failed. Please check if the file exists.\n",
        (string(dataDir) + "/segment_to_body_map.txt").c_str());
    return 1;
  }

  cout << "Loading segment_to_body_map.txt ..." << endl;
  ZString line;
  int lineNumber = 0;
  bool compress = Is_Arg_Matched(const_cast<char*>("--compress"));

  while (line.readLine(fp)) {
    //cout << line;
    vector<int> value = line.toIntegerArray();

    if (value.size() == 2) {
      //cout << ": " << value[0] << " " << value[1] << endl;
      Intpair_Map_Add(segmentBodyMap, 0, value[0], value[1]);
      
      if (compress) {
        if (bodyIdDict.count(value[1]) == 0) {
          bodyIdDict[value[1]] = ++bodyNumber;
        }
      }
    }
    lineNumber++;
  }

  fclose(fp);

  //Open the files
  char *workDir = Get_String_Arg(const_cast<char*>("-o"));

  char filePath[500];
  for (int i = 0; i < planeNumber; i++) {
    sprintf(filePath, "%s/superpixel_to_body_map%05d.txt", workDir,
        startPlane + i);
    fp = fopen(filePath, "w");
    fclose(fp);
  }
  
  int prevPlane = -1;
  FILE *superpixel_fp = fopen((string(dataDir) + 
        "/superpixel_to_segment_map.txt").c_str(), "r");
  fp = NULL;

  cout << "Loading superpixel_to_segment_map.txt ..." << endl;
  while (line.readLine(superpixel_fp)) {
    vector<int> value = line.toIntegerArray();
    if (value.size() == 3) {
      int segment_id = value[2];
      int body_id = Intpair_Map_Value(segmentBodyMap, 0, value[2]);
      if (value[0] != prevPlane) {
        if (fp != NULL) {
          fclose(fp);
        }
        sprintf(filePath, "%s/superpixel_to_body_map%05d.txt", 
            workDir, value[0]);
        fp = fopen(filePath, "a");
        prevPlane = value[0];
      }
      fprintf(fp, "%s %d\n", line.c_str(), body_id);
    }
  }
  fclose(fp);


  //Generate body id compress map
  if (compress) {
    sprintf(filePath, "%s/body_compress_map.txt", workDir);
    fp = fopen(filePath, "w");
    for (map<int, int>::iterator iter = bodyIdDict.begin(); 
        iter != bodyIdDict.end(); ++iter) {
      fprintf(fp, "%d\t%d\n", iter->first, iter->second);
    }

    fclose(fp);
  }

  return 0;
}
