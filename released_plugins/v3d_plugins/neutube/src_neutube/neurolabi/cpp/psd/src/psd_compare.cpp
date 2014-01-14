#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <algorithm>

#include "tz_utilities.h"
#include "zargumentprocessor.h"
#include "zpoint.h"
#include "tz_fmatrix.h"
#include "tz_farray.h"
#include "tz_darray.h"
#include "zstring.h"
#include "tz_iarray.h"
#include "flyem/zsynapselocationmatcher.h"
#include "flyem/zsynapseannotationarray.h"
#include "flyem/zsynapseannotationanalyzer.h"
#include "flyem/zsynapselocationmetric.h"
#include "swctreenode.h"
#include "zswctree.h"
#include "tz_iarray.h"

using namespace std;

static string get_current_date()
{
  time_t     now = time(0);
  struct tm  tstruct;
  char       buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);

  return buf;
}

static int help(int argc, char *argv[], const char *spec[])
{
  if (argc == 2) {
    if (strcmp(argv[1], "--help") == 0) {
      printf("psd_compare ");
      Print_Argument_Spec(spec);

      printf("\nDetails\n");
      printf("psdset1: the first psd file.\n");
      printf("psdset2: the second psd file. The script would skip the comparison step if this is not provided.\n");
      printf("--resolution: size of each voxel in nm, assuming that stack is isotropic.\n");
      printf("--tbar_dist: minimum distance between any two TBars.\n");
      printf("--psdset1_verify: output file for psd verification in the first input file.\n");
      printf("--psdset2_verify: output file for psd verification in the second input file.\n");
      printf("--psdset1_agree: output file of matched PSDs in the first input file.\n");
      printf("--psdset2_agree: output file of matched PSDs in the second input file.\n");
      printf("--swc: output swc file for diplaying (dis)agreements.\n");
      printf("--metric: matching metric option: "
             "0 for angle; 1 for euclidean distance.\n");
      printf("--metric_max: threshold of possible matching"
             "(the unit is radian for angle or nm for distance).\n");
      printf("--min_inter_tbar_dist: the minimal possible distance between two TBars\n");
      printf("--min_inter_psd_dist: the minimal possible distance between two PSDs\n");
      printf("--min_tbar_psd_dist: minimum possible distance between TBar and any of its partners\n");
      printf("--max_tbar_psd_dist: maximum possible distance between TBar and any of its partners\n");
      printf("--min_psd_number: minimum possible partner number of a TBar\n");
      printf("--max_psd_number: maximum possible partner number of a TBar\n");

      return 1;
    }
  }

  return 0;
}

int main(int argc, char *argv[])
{
  const static char *version = "1.1";

  /* Takes '-ver' option and show the version */
  if (Show_Version(argc, argv, version) == 1) {
    return 0;
  }

  static const char *Spec[] = {
    "<psdset1:string> [<psdset2:string>]",
    "[--resolution <double(10.0)>]",
    "[--tbar_dist <double(100.0)>]",
    "[--psdset1_verify <string>]",
    "[--psdset1_agree <string>] [--psdset2_verify <string>]",
    "[--psdset2_agree <string>] [--swc <string>] [--config <string>]",
    "[--evaluate <string>] [--metric <int(0)>] [--metric_max]",
    "[--min_inter_tbar_dist <double(50.0)>] [--min_inter_psd_dist <double(50.0)>]",
    "[--min_tbar_psd_dist <double(50.0)>] [--max_tbar_psd_dist <double(200.0)>]",
    "[--min_psd_number <int(2)>] [--max_psd_number <int(10)>]",
    "[--report <string>] [--help]", NULL};

  if (help(argc, argv, Spec) == 1) {
    return 0;
  }

  ZArgumentProcessor::processArguments(argc, argv, Spec);

  FlyEm::SynapseLocation::ELocationSpace spaceOption =
      FlyEm::SynapseLocation::CURRENT_SPACE;

  FlyEm::ZSynapseAnnotationAnalyzer analyzer;
  if (ZArgumentProcessor::isArgMatched("--config")) {
    analyzer.loadConfig(ZArgumentProcessor::getStringArg("--config"));
    spaceOption = FlyEm::SynapseLocation::IMAGE_SPACE;
  }

  double resolution = ZArgumentProcessor::getDoubleArg("--resolution");

  vector<string> psdSetFileList;

  //Get all psd set files
  psdSetFileList.push_back(ZArgumentProcessor::getStringArg("psdset1"));

  if (ZArgumentProcessor::isArgMatched("psdset2")) {
    psdSetFileList.push_back(ZArgumentProcessor::getStringArg("psdset2"));
  }

  //Load all psds set files
  vector<FlyEm::ZSynapseAnnotationArray> allAnnotation(psdSetFileList.size());
  
  vector<vector<FlyEm::SynapseLocation*> > tbarSetArray(psdSetFileList.size());

  double minTbarDist = ZArgumentProcessor::getDoubleArg("--min_inter_tbar_dist") /
      resolution;
  double minPsdDist = ZArgumentProcessor::getDoubleArg("--min_inter_psd_dist") /
      resolution;
  double minTbarPsdDist = ZArgumentProcessor::getDoubleArg("--min_tbar_psd_dist") /
      resolution;
  double maxTbarPsdDist = ZArgumentProcessor::getDoubleArg("--max_tbar_psd_dist") /
      resolution;
  int minPsdNumber = ZArgumentProcessor::getIntArg("--min_psd_number");
  int maxPsdNumber = ZArgumentProcessor::getIntArg("--max_psd_number");

#ifdef _DEBUG_
  cout << "Min tbar dist: " << minTbarDist << endl;
  cout << "Min psd dist: " << minPsdDist << endl;
  cout << "Min tbar-psd dist: " << minTbarPsdDist << endl;
  cout << "Max tbar-psd dist: " << maxTbarPsdDist << endl;
  cout << "Min psd number: " << minPsdNumber << endl;
  cout << "Max psd number: " << maxPsdNumber << endl;
#endif

  //Examine each file and warn about abnormalities
  for (size_t i = 0; i < allAnnotation.size(); i++) {
    cout << "Loading " << psdSetFileList[i] << endl;
    allAnnotation[i].loadJson(psdSetFileList[i]);
    tbarSetArray[i] = allAnnotation[i].toTBarRefArray();
    cout << "  TBar number: " << tbarSetArray[i].size() << endl;
    cout << "  PSD number: " << allAnnotation[i].getPsdNumber() << endl;

    for (size_t j = 0; j < allAnnotation[i].size(); j++) {
      vector<FlyEm::SynapseLocation*> duplicatedPsd =
          allAnnotation[i][j].duplicatedPartner(minPsdDist);
      if (!duplicatedPsd.empty()) {
        cout << "  WARNING: Duplicated PSDs on the TBar "
             << allAnnotation[i][j].getTBarRef()->pos().toString() << "?"
             << endl;
        cout << "    ";
        for (vector<FlyEm::SynapseLocation*>::const_iterator
            iter = duplicatedPsd.begin(); iter != duplicatedPsd.end(); ++iter) {
          cout << (*iter)->pos().toString() << " ";
        }
        cout << endl;
      }

      vector<FlyEm::SynapseLocation*> proximalPsd =
          allAnnotation[i][j].getProximalPartner(minTbarPsdDist);
      if (!proximalPsd.empty()) {
        cout << "  WARNING: PSDs are too close to the TBar "
             << allAnnotation[i][j].getTBarRef()->pos().toString()
             << endl;
        cout << "    ";
        for (vector<FlyEm::SynapseLocation*>::const_iterator
            iter = proximalPsd.begin(); iter != proximalPsd.end(); ++iter) {
          cout << (*iter)->pos().toString() << " ";
        }
        cout << endl;
      }

      vector<FlyEm::SynapseLocation*> distalPsd =
          allAnnotation[i][j].getDistalPartner(maxTbarPsdDist);
      if (!distalPsd.empty()) {
        cout << "  WARNING: PSDs are too far away from the TBar "
             << allAnnotation[i][j].getTBarRef()->pos().toString()
             << endl;
        cout << "    ";
        for (vector<FlyEm::SynapseLocation*>::const_iterator
            iter = distalPsd.begin(); iter != distalPsd.end(); ++iter) {
          cout << (*iter)->pos().toString() << " ";
        }
        cout << endl;
      }


      int psdNumber = allAnnotation[i][j].partnerNumber();
      if (psdNumber < minPsdNumber) {
        cout << "  WARNING: Too few psds (" << psdNumber << ") on the TBar "
             << allAnnotation[i][j].getTBarRef()->pos().toString() << endl;
      }

      if (psdNumber > maxPsdNumber) {
        cout << "  WARNING: Too many psds (" << psdNumber << ") on the TBar "
             << allAnnotation[i][j].getTBarRef()->pos().toString() << endl;
      }
    }


    vector<pair<FlyEm::SynapseLocation*, FlyEm::SynapseLocation*> >
        duplicatedTBar = allAnnotation[i].findDuplicatedTBar(minTbarDist);
    if (!duplicatedTBar.empty()) {
        for (vector<pair<FlyEm::SynapseLocation*, FlyEm::SynapseLocation*> >::const_iterator
            iter = duplicatedTBar.begin(); iter != duplicatedTBar.end(); ++iter) {
          cout << "  WARNING: Duplicated Tbars? ";
          cout << iter->first->pos().toString() << " "
               << iter->first->pos().toString() << endl;
        }
    } 
  }

  if (allAnnotation.size() < 2) { //Stop here if there is only one file loaded
    return 0;
  }

  //Match the tbar
  //FlyEm::ZSynapseLocationMatcher matcher;
  const double tbarDistThre = ZArgumentProcessor::getDoubleArg("--tbar_dist") /
      resolution;

/*
  cout << "Matching TBars ..." << endl;
  matcher.match(tbarSet1, tbarSet2, tbarDistThre);
  matcher.setVerbose(2);
  cout << "Done." << endl;
*/

  const int tbarMatchedType[2] = {6, 3};
  const int tbarUnmatchedType = 4;
  const int psdMatchedType = 10;
  const int psdUnmatchedType = 2;

  FlyEm::ZSynapseLocationMetric *metric = NULL;

  int metricChoice = ZArgumentProcessor::getIntArg("--metric");
  double metricMax = 0.0;
  switch (metricChoice) {
  case 0:
    metric = new FlyEm::ZSynapseLocationAngleMetric;
    metricMax = 0.4;
    break;
  case 1:
    metric = new FlyEm::ZSynapseLocationEuclideanMetric;
    metricMax = 200.0;
    break;
  }

  if (ZArgumentProcessor::isArgMatched("--metric_max")) {
    metricMax = ZArgumentProcessor::getDoubleArg("--metric_max");
  }

  if (metricChoice == 1) {
    metricMax /= resolution;
  }

  FlyEm::ZSynapseLocationMatcher overallPsdMatcher;
  overallPsdMatcher.setVerbose(1);
  overallPsdMatcher.setMetric(metric);
  FlyEm::ZSynapseLocationMatcher tbarMatcher =
      overallPsdMatcher.matchPsd(allAnnotation[0], allAnnotation[1], metricMax,
      tbarDistThre);

  string agreeFile[2];
  if (ZArgumentProcessor::isArgMatched("--psdset1_agree")) {
    agreeFile[0] = ZArgumentProcessor::getStringArg("--psdset1_agree");
  }
  if (ZArgumentProcessor::isArgMatched("--psdset2_agree")) {
    agreeFile[1] = ZArgumentProcessor::getStringArg("--psdset2_agree");
  }

  string verifyFile[2];
  if (ZArgumentProcessor::isArgMatched("--psdset1_verify")) {
    verifyFile[0] = ZArgumentProcessor::getStringArg("--psdset1_verify");
  }

  if (ZArgumentProcessor::isArgMatched("--psdset2_verify")) {
    verifyFile[1] = ZArgumentProcessor::getStringArg("--psdset2_verify");
  }

  FlyEm::ZSynapseLocationMatcher::ESide matchSide[2] = {
    FlyEm::ZSynapseLocationMatcher::LEFT_SIDE,
    FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE
  };

  for (int k =0; k < 2; k++) {
    allAnnotation[k].setSoftware("psd_compare");
    allAnnotation[k].setSoftwareVersion(version);
    allAnnotation[k].setSessionPath("");
    allAnnotation[k].setDate(get_current_date());

    if (!agreeFile[k].empty()) {
      vector<vector<int> > selected(allAnnotation[k].size());

      for (size_t i = 0; i < overallPsdMatcher.size(); i++) {
        int psdIndex = overallPsdMatcher.getIndex(i, matchSide[k]);
        pair<int, int> relativeIndex =
            allAnnotation[k].relativePsdIndex(psdIndex);
        selected[relativeIndex.first].push_back(relativeIndex.second);

#ifdef _DEBUG_
        if (allAnnotation[k].getPsdIndex(relativeIndex.first,
                                         relativeIndex.second) != psdIndex) {
          cout << "bug: index unmatched" << endl;
        }
#endif
      }

      vector<vector<int> > selectedPortable;
      for (size_t i = 0; i < selected.size(); i++) {
        if (!selected[i].empty()) {
          vector<int> s(1);
          s[0] = i;
          s.insert(s.end(), selected[i].begin(), selected[i].end());
#ifdef _DEBUG_2
          iarray_print(&(s[0]), s.size());
#endif
          selectedPortable.push_back(s);
        }
      }

      allAnnotation[k].exportJsonFile(agreeFile[k], &selectedPortable);
    }

    if (!verifyFile[k].empty()) {
      vector<vector<int> > selected(allAnnotation[k].size());
      for (size_t i = 0; i < overallPsdMatcher.unmatchedSize(matchSide[k]); i++) {
        int psdIndex =
            overallPsdMatcher.getIndex(i, matchSide[k],
                                       FlyEm::ZSynapseLocationMatcher::UNMATCHED);
        pair<int, int> relativeIndex =
            allAnnotation[k].relativePsdIndex(psdIndex);
        selected[relativeIndex.first].push_back(relativeIndex.second);
      }
      vector<vector<int> > selectedPortable;
      for (size_t i = 0; i < selected.size(); i++) {
        if (!selected[i].empty()) {
          vector<int> s(1);
          s[0] = i;
          s.insert(s.end(), selected[i].begin(), selected[i].end());
#ifdef _DEBUG_2
          iarray_print(&(s[0]), s.size());
#endif
          selectedPortable.push_back(s);
        }
      }

      allAnnotation[k].exportJsonFile(verifyFile[k], &selectedPortable);
    }
  }

  FlyEm::ZSynapseLocationMatcher::ESide sideList[2] = {
    FlyEm::ZSynapseLocationMatcher::LEFT_SIDE,
    FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE
  };

  if (ZArgumentProcessor::isArgMatched("--swc")) {
    ZSwcTree tree;
    vector<Swc_Tree_Node*> tbarNodeArray[2];
    for (size_t sideIndex = 0; sideIndex < 2; sideIndex++) {
      tbarNodeArray[sideIndex].resize(allAnnotation[sideIndex].size());
    }

    //Create tbar nodes
    for (size_t tbarMatchIndex = 0; tbarMatchIndex < tbarMatcher.size();
         tbarMatchIndex++) {
      for (size_t sideIndex = 0; sideIndex < 2; sideIndex++) {
        FlyEm::ZSynapseLocationMatcher::ESide side = sideList[sideIndex];
        int tbarIndex = tbarMatcher.getIndex(tbarMatchIndex, side);
        FlyEm::SynapseLocation *loc =
            allAnnotation[sideIndex].getTBarRef(tbarIndex);
        Swc_Tree_Node *tbarNode = SwcTreeNode::makePointer(
              loc->mapPosition(analyzer.config(), spaceOption),
              loc->confidence() + 1.0);
        //tbarNodeArray[sideIndex].push_back(tbarNode);
        tbarNodeArray[sideIndex][tbarIndex] = tbarNode;
        SwcTreeNode::setType(tbarNode, tbarMatchedType[sideIndex]);
        tree.addRegularRoot(tbarNode);
      }
    }

    //Make TBar nodes
    for (size_t sideIndex = 0; sideIndex < 2; sideIndex++) {
      FlyEm::ZSynapseLocationMatcher::ESide side = sideList[sideIndex];
      for (size_t tbarMatchIndex = 0;
           tbarMatchIndex < tbarMatcher.unmatchedSize(side);
           tbarMatchIndex++) {
        int tbarIndex = tbarMatcher.getIndex(tbarMatchIndex, side);
        FlyEm::SynapseLocation *loc =
            allAnnotation[sideIndex].getTBarRef(tbarIndex);
        Swc_Tree_Node *tbarNode = SwcTreeNode::makePointer(
              loc->mapPosition(analyzer.config(), spaceOption),
              loc->confidence() + 1.0);
        tbarNodeArray[sideIndex][tbarIndex] = tbarNode;
        SwcTreeNode::setType(tbarNode, tbarUnmatchedType);
        tree.addRegularRoot(tbarNode);
      }
    }

    //Create PSD nodes
    for (size_t psdMatchIndex = 0; psdMatchIndex < overallPsdMatcher.size();
         psdMatchIndex++) {
      for (size_t sideIndex = 0; sideIndex < 2; sideIndex++) {
        FlyEm::ZSynapseLocationMatcher::ESide side = sideList[sideIndex];
        int psdIndex = overallPsdMatcher.getIndex(psdMatchIndex, side);
        pair<int, int> ridx = allAnnotation[sideIndex].relativePsdIndex(psdIndex);

        FlyEm::SynapseLocation *loc =
            allAnnotation[sideIndex].getPsdRef(ridx.first, ridx.second);
        Swc_Tree_Node *psdNode = SwcTreeNode::makePointer(
              loc->mapPosition(analyzer.config(), spaceOption),
              loc->confidence() + 1.0);
        SwcTreeNode::setType(psdNode, psdMatchedType);
        SwcTreeNode::setParent(psdNode, tbarNodeArray[sideIndex][ridx.first]);
      }
    }

    for (size_t sideIndex = 0; sideIndex < 2; sideIndex++) {
      FlyEm::ZSynapseLocationMatcher::ESide side = sideList[sideIndex];
      for (size_t psdUnmatchIndex = 0;
           psdUnmatchIndex < overallPsdMatcher.unmatchedSize(side);
           psdUnmatchIndex++) {
        int psdIndex = overallPsdMatcher.getIndex(
              psdUnmatchIndex, side, FlyEm::ZSynapseLocationMatcher::UNMATCHED);
        pair<int, int> ridx = allAnnotation[sideIndex].relativePsdIndex(psdIndex);
        FlyEm::SynapseLocation *loc =
            allAnnotation[sideIndex].getPsdRef(ridx.first, ridx.second);
        Swc_Tree_Node *psdNode = SwcTreeNode::makePointer(
              loc->mapPosition(analyzer.config(), spaceOption),
              loc->confidence() + 1.0);
        SwcTreeNode::setType(psdNode, psdUnmatchedType);
        SwcTreeNode::setParent(psdNode, tbarNodeArray[sideIndex][ridx.first]);
      }
    }

    tree.resortId();
    tree.save(ZArgumentProcessor::getStringArg("--swc"));
  }

  return 0;
}
