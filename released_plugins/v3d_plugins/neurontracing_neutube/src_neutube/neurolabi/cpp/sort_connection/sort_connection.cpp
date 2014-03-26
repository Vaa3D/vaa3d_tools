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
#include "flyem/zfileparser.h"
#include "flyem/zsynapseannotationarray.h"
#include "flyem/zsynapseannotationanalyzer.h"
#include "zvaa3dmarker.h"

using namespace std;

static size_t get_index(map<int, size_t> &bodyMap, int bodyId)
{
  if (bodyMap.count(bodyId) == 0) {
    size_t lastIndex = bodyMap.size();
    bodyMap[bodyId] = lastIndex;

    return lastIndex; 
  }

  return bodyMap[bodyId];
}

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> --body_id <int>",
    "[-o <string>] [--minconn <int(1)>]", NULL};
  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(Get_String_Arg(const_cast<char*>("input")));

  int bodyId = Get_Int_Arg(const_cast<char*>("--body_id"));

  std::map<int, size_t> bodyIdMap;
  vector<int> bodyIdArray(1000, -1);
  vector<int> output(1000, 0);
  vector<int> input(1000, 0);

  for (size_t i = 0; i < synapseArray.size(); i++) {
    int tBarId = synapseArray[i].getTBarRef()->bodyId(); 
    vector<FlyEm::SynapseLocation> *partnerArray = 
      synapseArray[i].getPartnerArrayRef(); 
    if (tBarId == bodyId) {
      //Count outputs
      for (size_t j = 0; j < partnerArray->size(); j++) {
        output[get_index(bodyIdMap, (*partnerArray)[j].bodyId())]++;
      }
    } else {
      //Count inputs
      for (size_t j = 0; j < partnerArray->size(); j++) {
        if ((*partnerArray)[j].bodyId() == bodyId) {
          input[get_index(bodyIdMap, tBarId)]++;
        }
      }
    }
  }

  for (map<int, size_t>::iterator iter = bodyIdMap.begin();
      iter != bodyIdMap.end(); ++iter) {
    bodyIdArray[iter->second] = iter->first;
  }

  int minconn = Get_Int_Arg(const_cast<char*>("--minconn"));

  for (size_t i = 0; i < input.size(); i++) {
    if (input[i] >= minconn) {
      cout << bodyIdArray[i] << " x " << input[i] << " --> " << bodyId << endl;
    }
  }

  for (size_t i = 0; i < output.size(); i++) {
    if (output[i] >= minconn) {
      cout << bodyIdArray[i] << " x " << output[i] << " <-- " << bodyId << endl;
    }
  }

  return 0;
}
