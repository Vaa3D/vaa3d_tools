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

using namespace std;

struct SwcScoreCompare {
  bool operator() (const pair<double, string> &swcScore1,
      const pair<double, string> &swcScore2) 
  {
    if (swcScore1.first > swcScore2.first) {
      return true;
    } 

    return false;
  }
};

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> --database <string> ", NULL};
  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  ZSwcTree inputTree;
  inputTree.load(Get_String_Arg(const_cast<char*>("input")));

  ZSwcBranch *branch1 = inputTree.extractFurthestBranch();

  vector<pair<double, string> > swcScoreArray;

  FILE *fp = fopen(Get_String_Arg(const_cast<char*>("--database")), "r");
  ZString str;
  char swcFile[500];
  while (str.readLine(fp)) {
    if (str.contains("swc")) {
      ZSwcTree record;
      strcpy(swcFile, str.c_str());
      strtrim(swcFile);

      cout << swcFile << endl;
      
      record.load(swcFile);
      
      pair<double, string> swcScore;
      swcScore.second = swcFile;

      ZSwcBranch *branch2 = record.extractFurthestBranch();
      ZSwcTreeMatcher matcher;
      matcher.setShollAnalysisParameters(10, 110, 20);
      std::map<int, int> matching = matcher.matchBranchByLocalSholl(
          branch1, branch2, &inputTree, &record, true);

      std::cout << "Score: " << matcher.matchingScore() << std::endl;
      swcScore.first = matcher.matchingScore();

      if (matcher.matchingScore() != 1.0) {
        swcScoreArray.push_back(swcScore);
      }

      delete branch2;
    }
  }

  sort(swcScoreArray.begin(), swcScoreArray.end(), SwcScoreCompare());

  cout << endl;
  cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  cout << endl;
  cout << "Most similar neurons: " << endl << endl;

  double bestScore = swcScoreArray[0].first;

  for (int i = 0; i <= swcScoreArray.size() / 5; i++) {
    if (swcScoreArray[i].first * 1.5 > bestScore) {
      ZString swcpath(swcScoreArray[i].second);
      cout << swcpath.lastInteger() << endl;
    }
  }
  cout << endl;

  delete branch1;

  return 0;
}
