#include <iostream>
#include <sstream>
#include <fstream>

#include <string.h>
#include <stdlib.h>

#include "tz_utilities.h"
#include "tz_swc_tree.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "tz_intpair_map.h"
#include "zswctreematcher.h"
#include "zswcbranch.h"
#include "tz_iarray.h"
#include "tz_darray.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {
    "<input1:string> [<input2:string> | --pairwise] [-o <string>]"
    " [--method <string>] [--normalized] [--pair_swc <int>]", NULL};
  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  if (Is_Arg_Matched(const_cast<char*>("--pairwise"))) {
     std::vector<ZSwcTree*> treeArray = ZSwcTree::loadTreeArray(
         Get_String_Arg(const_cast<char*>("input1")));
     ZSwcTreeMatcher matcher;
     matcher.setShollAnalysisParameters(10, 110, 20);

     std::vector<std::vector<double> > scoreMatrix =
       matcher.computePairwiseMatchingScore(treeArray);

     std::ostringstream stream;
     for (size_t i = 0; i < scoreMatrix.size() - 1; i++) {
       for (size_t j = 0; j < scoreMatrix[i].size(); j++) {
         stream << scoreMatrix[i][j] << ", ";
       }
       stream << endl;
     }

     cout << stream.str() << endl;
  } else {
    ZSwcTree tree1;
    tree1.load(Get_String_Arg(const_cast<char*>("input1")));

    ZSwcTree tree2;
    tree2.load(Get_String_Arg(const_cast<char*>("input2")));

    ZSwcBranch *branch1 = tree1.extractFurthestBranch();
    ZSwcBranch *branch2 = tree2.extractFurthestBranch();

    bool normalized = false;

    if (Is_Arg_Matched(const_cast<char*>("--normalized"))) {
      normalized = true;
    }

    ZSwcTreeMatcher matcher;
    matcher.setShollAnalysisParameters(10, 110, 20);
    std::map<int, int> matching = matcher.matchBranchByLocalSholl(
        branch1, branch2, &tree1, &tree2, normalized);

    std::cout << "Score: " << matcher.matchingScore() << std::endl;

    if (Is_Arg_Matched(const_cast<char*>("--pair_swc"))) {
      vector<ZPoint> pointArray1 = matcher.firstPointArray();
      vector<ZPoint> pointArray2 = matcher.secondPointArray();
      vector<vector<double> > featureArray1 = matcher.firstFeatureArray();
      vector<vector<double> > featureArray2 = matcher.secondFeatureArray();

      ZSwcTree connection;
      connection.setData(New_Swc_Tree());

      int index = 0;

      switch (Get_Int_Arg(const_cast<char*>("--pair_swc"))) {
        case 1:
          tree1.setType(3);
          tree2.setType(4);
          connection.merge(tree1.data());
          connection.merge(tree2.data());
        case 0:
          for (std::map<int, int>::iterator iter = matching.begin();
              iter != matching.end(); ++iter, ++index) {
            Swc_Tree_Node *tn = ZSwcTree::makeArrow(pointArray1[iter->first],
                1.0, 0, pointArray2[iter->second], 0.0, 0, true);
            Swc_Tree_Merge_Node_As_Root(connection.data(), tn);

            double s1 = darray_sum(&(featureArray1[iter->first][0]), 
                featureArray1[iter->first].size());
            double s2 = darray_sum(&(featureArray2[iter->second][0]), 
                featureArray2[iter->second].size());
            double diff = 0.0;
            if (max(s1, s2) > 0) {
              diff = static_cast<double>(max(s1, s2) - min(s1, s2)) / 
                max(s1, s2);
            }

            if (diff > 0.5) {
              tn->node.type = 2;
              tn->first_child->node.type = 2;
            }
          }
          break;
        default:
          break;
      }

      connection.resortId();
      connection.save(Get_String_Arg(const_cast<char*>("-o")));
      cout << Get_String_Arg(const_cast<char*>("-o")) << " saved." <<
        endl;
    }
    delete branch1;
    delete branch2;
  }

  return 0;
}
