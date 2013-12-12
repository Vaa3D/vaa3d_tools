#include "zswctreematcher.h"

#include <queue>

#include "zqtheader.h"

#include <iostream>
#if defined(_QT_GUI_USED_)
#include <QDebug>
#endif

#include "tz_utilities.h"
#include "zswcbranch.h"
#include "zmatrix.h"
#include "zswctree.h"
#include "zpoint.h"
#include "tz_int_arraylist.h"
#include "zinttree.h"
#include "zswcfeatureanalyzer.h"
#include "zswcdisttrunkanalyzer.h"

using namespace std;

ZSwcTreeMatcher::ZSwcTreeMatcher()
{
  m_matchingScore = 0.0;
  m_shollStart = 5.0;
  m_shollEnd = 50.0;
  m_shollRadius = 5.0;
  m_gapMap.resize(2);
  m_featureAnalyzer = NULL;
  m_trunkAnalyzer = NULL;
}

map<int, int> ZSwcTreeMatcher::dynamicProgrammingMatch(
    const ZMatrix &simMat, double gapPenalty)
{
  ZMatrix matchingTable(simMat.getRowNumber() + 1, simMat.getColumnNumber() + 1);
  matchingTable.setConstant(0.0);

  ZIntTree matchingTree(simMat.getSize());

  for (int i = 0; i < simMat.getRowNumber(); i++) {
    for (int j = 0; j < simMat.getColumnNumber(); j++) {
      double score = matchingTable.getValue(i, j) + simMat.getValue(i, j);
      double maxScore = score;
      matchingTree.setParent(simMat.sub2index(i, j),
                            simMat.sub2index(i - 1, j - 1));
      score = matchingTable.getValue(i + 1, j) - gapPenalty;
      if (score > maxScore) {
        maxScore = score;
        matchingTree.setParent(simMat.sub2index(i, j),
                              simMat.sub2index(i, j - 1));
      }
      score = matchingTable.getValue(i, j + 1) - gapPenalty;
      if (score > maxScore) {
        maxScore = score;
        matchingTree.setParent(simMat.sub2index(i, j),
                              simMat.sub2index(i - 1, j));
      }
      matchingTable.set(i + 1, j + 1, maxScore);
    }
  }

  //matchingTable.debugOutput();

  int bestIndex = -1;
  m_matchingScore = -gapPenalty * 10.0; //Set to a small value
  int lastColumnIndex = matchingTable.getColumnNumber() - 1;
  int lastRowIndex = matchingTable.getRowNumber() - 1;
  for (int i = 1; i < matchingTable.getRowNumber(); i++) {
    if (matchingTable.getValue(i, lastColumnIndex) > m_matchingScore) {
      m_matchingScore = matchingTable.getValue(i, lastColumnIndex);
      bestIndex = simMat.sub2index(i - 1, lastColumnIndex - 1);
    }
  }
  for (int i = 1; i < matchingTable.getColumnNumber(); i++) {
    if (matchingTable.getValue(lastRowIndex, i) > m_matchingScore) {
      m_matchingScore = matchingTable.getValue(lastRowIndex, i);
      bestIndex = simMat.sub2index(lastRowIndex - 1, i - 1);
    }
  }

#ifdef _DEBUG_2
  matchingTable.debugOutput();
  std::cout << std::endl;
#endif

  vector<int> matchingTrace = matchingTree.traceBack(bestIndex);

  map<int, int> matches;

  for (size_t i = 0; i < matchingTrace.size(); i++) {
    pair<int, int> sub = simMat.index2sub(matchingTrace[i]);
    matches[sub.first] = sub.second;
#ifdef _DEBUG_2
    std::cout << "s: " << simMat.getValue(sub.first, sub.second)
              << " " << matchingTable.getValue(sub.first + 1, sub.second + 1)
              << std::endl;
#endif
  }

  return matches;
}



void ZSwcTreeMatcher::setShollAnalysisParameters(
    double start, double end, double radius)
{
  m_shollStart = start;
  m_shollEnd = end;
  m_shollRadius = radius;
}

map<int, int> ZSwcTreeMatcher::matchBranch(ZSwcBranch *branch1,
                                           ZSwcBranch *branch2)
{
  branch1->updateAccumDistance();
  branch2->updateAccumDistance();

  //Calculate the features of the branch points
  /*
  Swc_Tree tree1;
  tree1.root = branch1->root();
  Swc_Tree_Iterator_Start(&tree1, SWC_TREE_ITERATOR_DEPTH_FIRST, false);

  Swc_Tree tree2;
  tree2.root = branch2->root();
  Swc_Tree_Iterator_Start(&tree2, SWC_TREE_ITERATOR_DEPTH_FIRST, false);
*/
  vector<Swc_Tree_Node*> branchPointArray1 = branch1->produceBranchSubTree();
  vector<double> distanceArray1(branchPointArray1.size());

  for (size_t i = 0; i < branchPointArray1.size(); i++) {
    if (i == 0) {
      distanceArray1[i] = branchPointArray1[i]->weight -
          branch1->upEnd()->weight;
    } else {
      distanceArray1[i] = branchPointArray1[i]->weight -
          branchPointArray1[i - 1]->weight;
    }
  }

  vector<Swc_Tree_Node*> branchPointArray2 = branch2->produceBranchSubTree();
  vector<double> distanceArray2(branchPointArray2.size());

  for (size_t i = 0; i < branchPointArray2.size(); i++) {
    if (i == 0) {
      distanceArray2[i] = branchPointArray2[i]->weight -
          branch2->upEnd()->weight;
    } else {
      distanceArray2[i] = branchPointArray2[i]->weight -
          branchPointArray2[i - 1]->weight;
    }
  }

  vector<vector<double> > featureArray1(branchPointArray1.size());
  vector<vector<double> > featureArray2(branchPointArray2.size());

  for (size_t i = 0; i < branchPointArray1.size(); i++) {
    std::vector<double> feature1;
    Swc_Tree_Node_Detach_Parent(branchPointArray1[i]);
    ZSwcTree tmpTree1;
    tmpTree1.setDataFromNode(branchPointArray1[i]);
    std::vector<int> shollCount =
        tmpTree1.shollAnalysis(m_shollStart, m_shollEnd, m_shollRadius,
                               ZPoint(Swc_Tree_Node_X(branchPointArray1[i]),
                                      Swc_Tree_Node_Y(branchPointArray1[i]),
                                      Swc_Tree_Node_Z(branchPointArray1[i])));
    feature1.resize(shollCount.size());
    for (size_t j = 0; j < shollCount.size(); j++) {
      feature1[j] = shollCount[j];
    }
    featureArray1[i] = feature1;
  }
  for (size_t i = 0; i < branchPointArray2.size(); i++) {
    std::vector<double> feature2;
    ZSwcTree tmpTree2;
    tmpTree2.setDataFromNode(branchPointArray2[i]);
    std::vector<int> shollCount =
        tmpTree2.shollAnalysis(m_shollStart, m_shollEnd, m_shollRadius,
                           ZPoint(Swc_Tree_Node_X(branchPointArray2[i]),
                                  Swc_Tree_Node_Y(branchPointArray2[i]),
                                  Swc_Tree_Node_Z(branchPointArray2[i])));
    feature2.resize(shollCount.size());
    for (size_t j = 0; j < shollCount.size(); j++) {
      feature2[j] = shollCount[j];
    }
    featureArray2[i] = feature2;
  }



  featureArray1 = expandFeatureArray(featureArray1, distanceArray1, 0);
  featureArray2 = expandFeatureArray(featureArray2, distanceArray2, 1);

  //Build similarity map
  ZMatrix simMat(featureArray1.size(), featureArray2.size());
  for (int i = 0; i < simMat.getRowNumber(); i++) {
    for (int j = 0; j < simMat.getColumnNumber(); j++) {
      simMat.set(i, j, computeFeatureSimilarity(featureArray1[i],
                                                featureArray2[j]));
    }
  }

  //simMat.debugOutput();

  //Run dynamic programming
  map<int, int> matchResult = dynamicProgrammingMatch(simMat, 0.5);

  map<int, int> finalMatchResult;

  //Extract result

  for (map<int, int>::const_iterator iter = matchResult.begin();
       iter != matchResult.end(); ++iter) {
    if (m_gapMap[0].count(iter->first) > 0 &&
        m_gapMap[1].count(iter->second) > 0) {
      finalMatchResult[m_gapMap[0][iter->first]] = m_gapMap[1][iter->second];
    }
  }

  return finalMatchResult;
}

vector<vector<double> > ZSwcTreeMatcher::computeFeatureArray(
    ZSwcBranch *branch, ZSwcTree *host)
{
  vector<vector<double> > featureArray;

  branch->updateIterator();
  Swc_Tree_Node *tn = branch->upEnd();
  while (tn != NULL) {
    std::vector<int> shollCount =
        host->shollAnalysis(m_shollStart, m_shollEnd, m_shollRadius,
                               ZPoint(Swc_Tree_Node_X(tn),
                                      Swc_Tree_Node_Y(tn),
                                      Swc_Tree_Node_Z(tn)));
    std::vector<double> feature;
    feature.resize(shollCount.size());
    for (size_t j = 0; j < shollCount.size(); j++) {
      feature[j] = shollCount[j];
    }
    featureArray.push_back(feature);

    tn = tn->next;
  }

  return featureArray;
}

map<int, int> ZSwcTreeMatcher::matchBranchByLocalSholl(ZSwcBranch *branch1,
                                                       ZSwcBranch *branch2,
                                                       ZSwcTree *host1,
                                                       ZSwcTree *host2,
                                                       bool normalized)
{
  m_featureArray1.clear();
  m_featureArray2.clear();

  tic();
  m_pointArray1 = branch1->sample(m_shollRadius / 2.0);

  for (size_t i = 0; i < m_pointArray1.size(); i++) {
    vector<int> shollCount =
        host1->shollAnalysis(m_shollStart, m_shollEnd, m_shollRadius,
                             m_pointArray1[i]);
    vector<double> dShollCount(shollCount.size());
    for (size_t j = 0; j < shollCount.size(); j++) {
      dShollCount[j] = shollCount[i];
    }
    m_featureArray1.push_back(dShollCount);
  }

  pmtoc("Sholl analysis on branch 1");

  tic();

  m_pointArray2 = branch2->sample(m_shollRadius / 2.0);

  ZPoint vec1 = m_pointArray1[0] - m_pointArray1[m_pointArray1.size() - 1];
  ZPoint vec2 = m_pointArray2[0] - m_pointArray2[m_pointArray2.size() - 1];

  if (vec1.dot(vec2) < 0.0) {
    //Flip vec2
    vector<ZPoint> pointArray(m_pointArray2.size());
    for (size_t i = 0; i< pointArray.size(); i++) {
      pointArray[i] = m_pointArray2[pointArray.size() - i - 1];
    }
    m_pointArray2 = pointArray;
  }

  for (size_t i = 0; i < m_pointArray2.size(); i++) {
    std::vector<int> shollCount =
        host2->shollAnalysis(m_shollStart, m_shollEnd, m_shollRadius,
                             m_pointArray2[i]);
    vector<double> dShollCount(shollCount.size());
    for (size_t j = 0; j < shollCount.size(); j++) {
      dShollCount[j] = shollCount[i];
    }
    m_featureArray2.push_back(dShollCount);
  }

  pmtoc("Sholl analysis on branch 2");

  //tic();
  //Build similarity map
  ZMatrix simMat(m_featureArray1.size(), m_featureArray2.size());
  for (int i = 0; i < simMat.getRowNumber(); i++) {
    for (int j = 0; j < simMat.getColumnNumber(); j++) {
      simMat.set(i, j, computeFeatureSimilarity(m_featureArray1[i],
                                                m_featureArray2[j]));
    }
  }
  //pmtoc("Make similarity matrix");

  //simMat.debugOutput();

  //Run dynamic programming
  tic();
  map<int, int> matchResult = dynamicProgrammingMatch(simMat, 0.5);
  pmtoc("dynamic programming");

  tic();
  if (normalized) {
    double sum1 = 0.0;
    double sum2 = 0.0;
    for (size_t i = 0; i < m_featureArray1.size(); i++) {
      sum1 += computeFeatureSimilarity(m_featureArray1[i],
                                       m_featureArray1[i]);
    }

    for (size_t i = 0; i < m_featureArray2.size(); i++) {
      sum2 += computeFeatureSimilarity(m_featureArray2[i],
                                       m_featureArray2[i]);
    }

    m_matchingScore /= (sum1 + sum2) * 0.5;
  }
  pmtoc("Compute similarity");

  return matchResult;
}

vector<vector<double> >
ZSwcTreeMatcher::computePairwiseMatchingScore(std::vector<ZSwcTree*> treeArray)
{
  //Under development

  //mark;

  //vector<ZSwcBranch*> branchArray(treeArray.size(), NULL);
  vector<vector<vector<int> > > shollFeatureArray(treeArray.size());

  for (size_t i = 0; i < treeArray.size(); i++) {
    ZSwcBranch *branch = treeArray[i]->extractFurthestBranch();
    vector<ZPoint> pointArray = branch->sample(m_shollRadius / 2.0);

    for (size_t j = 0; j < pointArray.size(); j++) {
      std::vector<int> shollCount =
          treeArray[i]->shollAnalysis(m_shollStart, m_shollEnd, m_shollRadius,
                                      pointArray[j]);
      shollFeatureArray[i].push_back(shollCount);
    }

    delete branch;
  }

  vector<vector<double> > scoreMatrix(treeArray.size());

  for (size_t i = 0; i < scoreMatrix.size() - 1; i++) {
    scoreMatrix[i].resize(treeArray.size(), 0.0);
    for (size_t j = i + 1; j < treeArray.size(); j++) {

      scoreMatrix[i][j] = computeFeatureArraySimilarity(shollFeatureArray[i],
                                                        shollFeatureArray[j],
                                                        true);
            /*
      scoreMatrix[i][j] = computeFeatureArraySimilarityZScore(shollFeatureArray[i],
                                                        shollFeatureArray[j]);
                                                        */
    }
  }

  return scoreMatrix;
}

vector<vector<double> >
ZSwcTreeMatcher::expandFeatureArray(const vector<vector<double> > &featureArray,
    const vector<double> &distanceArray, int index)
{
  vector<vector<double> > newFeatureArray;
  //newFeatureArray.push_back(featureArray[0]);

  m_gapMap[index].clear();
  //m_gapMap[index][0] = 0;
  for (size_t i = 0; i < distanceArray.size(); i++) {
    double distance = m_shollRadius;
    while (distance < distanceArray[i] + m_shollRadius * 0.5) {
      vector<double> emptyFeature(0);
      newFeatureArray.push_back(emptyFeature);
      distance += m_shollRadius;
    }
    m_gapMap[index][newFeatureArray.size()] = i;
    newFeatureArray.push_back(featureArray[i]);
  }

  return newFeatureArray;
}

double ZSwcTreeMatcher::computeTreeSimilarity(
    ZSwcTree *tree1, ZSwcTree *tree2)
{
  std::vector<int> shollCount1 =
      tree1->shollAnalysis(m_shollStart, m_shollEnd, m_shollRadius,
                          tree1->somaCenter());
  std::vector<int> shollCount2 =
      tree2->shollAnalysis(m_shollStart, m_shollEnd, m_shollRadius,
                          tree2->somaCenter());

  double score = computeFeatureSimilarity(shollCount1, shollCount2, false);

  return score;
}

void ZSwcTreeMatcher::debugOutputMatching(const map<int, int> &matching)
{
  for (map<int, int>::const_iterator iter = matching.begin();
       iter != matching.end(); ++iter) {
#if defined(_QT_GUI_USED_)
    qDebug() << iter->first << ' ' << iter->second;
#else
    cout << iter->first << ' ' << iter->second;
#endif
  }
}

vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> >
ZSwcTreeMatcher::match(ZSwcBranch &branch1, ZSwcBranch &branch2)
{
  vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> > result;

  branch1.label(0);
  branch2.label(0);

  vector<Swc_Tree_Node*> nodeArray1 = branch1.toArray();
  vector<Swc_Tree_Node*> nodeArray2 = branch2.toArray();

  m_featureArray1.resize(nodeArray1.size());
  m_featureArray2.resize(nodeArray2.size());

  for (size_t i = 0; i < nodeArray1.size(); i++) {
    m_featureArray1[i] = m_featureAnalyzer->computeFeature(nodeArray1[i]);
  }

  for (size_t i = 0; i < nodeArray2.size(); i++) {
    m_featureArray2[i] = m_featureAnalyzer->computeFeature(nodeArray2[i]);
  }

  ZMatrix simMat(m_featureArray1.size(), m_featureArray2.size());

  for (int i = 0; i < simMat.getRowNumber(); i++) {
    for (int j = 0; j < simMat.getColumnNumber(); j++) {
      simMat.set(i, j, m_featureAnalyzer->computeFeatureSimilarity(
                   m_featureArray1[i], m_featureArray2[j]));
    }
  }

#ifdef _DEBUG_2
  simMat.debugOutput();
#endif

  map<int, int> matches = dynamicProgrammingMatch(simMat, 0.1);

  pair<int, int> lastMatch;
  map<int, int>::iterator iter = matches.begin();
  lastMatch.first = iter->first;
  lastMatch.second = iter->second;
  for (++iter; iter !=  matches.end(); ++iter) {
    if (lastMatch.first != iter->first && lastMatch.second != iter->second) {
      result.push_back(
            pair<Swc_Tree_Node*, Swc_Tree_Node*>(
              nodeArray1[lastMatch.first], nodeArray2[lastMatch.second]));
#ifdef _DEBUG_2
      cout << lastMatch.first << " " << lastMatch.second << endl;
#endif

      lastMatch.first = iter->first;
      lastMatch.second = iter->second;
    }
  }
#ifdef _DEBUG_2
      cout << lastMatch.first << " " << lastMatch.second << endl;
#endif
  result.push_back(
        pair<Swc_Tree_Node*, Swc_Tree_Node*>(
          nodeArray1[lastMatch.first], nodeArray2[lastMatch.second]));

  return result;
}

void ZSwcTreeMatcher::match(ZSwcTree &tree1, ZSwcTree &tree2)
{
  ZSwcBranch *branch1 = tree1.extractFurthestBranch();
  ZSwcBranch *branch2 = tree2.extractFurthestBranch();

  tree1.setLabel(1);
  tree2.setLabel(1);

  m_matchingResult = match(*branch1, *branch2);

  delete branch1;
  delete branch2;
}

void ZSwcTreeMatcher::updateMatchingSource(
    std::queue<MatchingSource> *sourceQueue,
    const std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> > &matching)
{
  for (size_t i = 0; i < matching.size(); i++) {
    MatchingSource source;
    Swc_Tree_Node *child = matching[i].first->first_child;
    int n1 = 0;
    //Add excluded nodes
    while (child != NULL) {
      if (Swc_Tree_Node_Label(child) > 0) {
        source.exclude1 = child;
      } else {
        n1++;
      }
      child = child->next_sibling;
    }

    child = matching[i].second->first_child;
    int n2 = 0;
    while (child != NULL) {
      if (Swc_Tree_Node_Label(child) > 0) {
        source.exclude2 = child;
      } else {
        n2++;
      }
      child = child->next_sibling;
    }

    if (n1 > 0 && n2 > 0) {
      source.node1 = matching[i].first;
      source.node2 = matching[i].second;
      sourceQueue->push(source);
    }
  }
}

double ZSwcTreeMatcher::matchAll(ZSwcTree &tree1, ZSwcTree &tree2)
{
  queue<MatchingSource> sourceQueue;

  /* Initialize the source */
  MatchingSource source;
  ZSwcBranch *pBranch1 = tree1.extractFurthestBranch();
  ZSwcBranch *pBranch2 = tree2.extractFurthestBranch();

  tree1.setLabel(1);
  tree2.setLabel(1);

  //cout << "Tree size: " << tree1.size() << endl;

  //cout << "Matching ..." << endl;

  m_matchingResult = match(*pBranch1, *pBranch2);

  //cout << "Tree size: " << tree1.size() << endl;

  //cout << "Updating Matching source..." << endl;

  updateMatchingSource(&sourceQueue, m_matchingResult);

  //cout << "Tree size: " << tree1.size() << endl;

  /* while the source is not empty */
  while (!sourceQueue.empty()) {
#ifdef _DEBUG_2
    cout << sourceQueue.size() << endl;
#endif
    /* Dequeue the pair source */
    source = sourceQueue.front();
    sourceQueue.pop();

    /* Extract the trunk pair from the source */
    Swc_Tree_Node *leaf1 =
        Swc_Tree_Node_Furthest_Leaf_E(source.node1, source.exclude1);
    Swc_Tree_Node *leaf2 =
        Swc_Tree_Node_Furthest_Leaf_E(source.node2, source.exclude2);

    if (source.node1 != leaf1 && source.node2 != leaf2) {
      ZSwcBranch branch1(source.node1, leaf1);
      ZSwcBranch branch2(source.node2, leaf2);

      /* Match the trunk pair */
      vector<pair<Swc_Tree_Node*, Swc_Tree_Node*> > matchingResult =
          match(branch1, branch2);

      /* Update the source (enqueue) */
      updateMatchingSource(&sourceQueue, matchingResult);

      /* Update the matching result */
      m_matchingResult.insert(m_matchingResult.end(), matchingResult.begin(),
                              matchingResult.end());
    }
  }

  return 0.0;
}

vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> >
ZSwcTreeMatcher::matchG(ZSwcPath &branch1, ZSwcPath &branch2, int level)
{
  vector<pair<Swc_Tree_Node*, Swc_Tree_Node*> > result;

  branch1.label(level);
  branch2.label(level);

  m_featureArray1.resize(branch1.size());
  m_featureArray2.resize(branch2.size());

  for (size_t i = 0; i < branch1.size(); i++) {
    m_featureArray1[i] = m_featureAnalyzer->computeFeature(branch1[i]);
  }

  for (size_t i = 0; i < branch2.size(); i++) {
    m_featureArray2[i] = m_featureAnalyzer->computeFeature(branch2[i]);
  }

  ZMatrix simMat(m_featureArray1.size(), m_featureArray2.size());

  for (int i = 0; i < simMat.getRowNumber(); i++) {
    for (int j = 0; j < simMat.getColumnNumber(); j++) {
      simMat.set(i, j, m_featureAnalyzer->computeFeatureSimilarity(
                   m_featureArray1[i], m_featureArray2[j]));
    }
  }

#ifdef _DEBUG_2
  simMat.debugOutput();
#endif

  map<int, int> matches = dynamicProgrammingMatch(simMat, 0.1);

  for (map<int, int>::iterator iter = matches.begin(); iter !=  matches.end();
       ++iter) {
    result.push_back(
          pair<Swc_Tree_Node*, Swc_Tree_Node*>(
            branch1[iter->first], branch2[iter->second]));
#ifdef _DEBUG_2
    cout << iter->first << " - " << iter->second << ":"
         << simMat.at(iter->first, iter->second) << endl;
#endif
  }

  return result;
}


void ZSwcTreeMatcher::matchAllG(ZSwcTree &tree1, ZSwcTree &tree2, int level)
{
  if (level <= 0) {
    return;
  }

  m_trunkAnalyzer->clearBlocker();

  queue<MatchingSource> sourceQueue;

  /* Initialize the source */
  MatchingSource source;

  ZSwcPath branch1 = tree1.mainTrunk(m_trunkAnalyzer);
  ZSwcPath branch2 = tree2.mainTrunk(m_trunkAnalyzer);

  SwcTreeNode::setAsRoot(branch1[0]);
  tree1.setDataFromNodeRoot(branch1[0], ZSwcTree::LEAVE_ALONE);

  ZPoint vec1 = SwcTreeNode::pos(branch1[0]) -
      SwcTreeNode::pos(branch1.back());
  ZPoint vec2 = SwcTreeNode::pos(branch2[0]) -
      SwcTreeNode::pos(branch2.back());

  if (vec1.dot(vec2) < 0) {
    branch2.reverse();
  }

  SwcTreeNode::setAsRoot(branch2[0]);
  tree2.setDataFromNodeRoot(branch2[0], ZSwcTree::LEAVE_ALONE);

  tree1.setLabel(0);
  tree2.setLabel(0);

  //cout << "Tree size: " << tree1.size() << endl;

  //cout << "Matching ..." << endl;

  m_matchingResult = matchG(branch1, branch2, 1);

  //cout << "Tree size: " << tree1.size() << endl;

  //cout << "Updating Matching source..." << endl;

  updateMatchingSource(&sourceQueue, m_matchingResult);

  double currentScore = m_matchingScore;

  //cout << "Tree size: " << tree1.size() << endl;
  /* while the source is not empty */
  while (!sourceQueue.empty()) {
#ifdef _DEBUG_2
    cout << "Queue size: " << sourceQueue.size() << endl;
#endif

    /* Dequeue the pair source */
    source = sourceQueue.front();
    sourceQueue.pop();

    int currentLevel = SwcTreeNode::label(source.node1) + 1;
    if (currentLevel <= level) {
      m_trunkAnalyzer->clearBlocker();
      m_trunkAnalyzer->addBlocker(source.exclude1);
      m_trunkAnalyzer->addBlocker(source.exclude2);

      /* Extract the trunk pair from the source */
      branch1 = m_trunkAnalyzer->extractTrunk(&tree1, source.node1);
      branch2 = m_trunkAnalyzer->extractTrunk(&tree2, source.node2);


      vector<pair<Swc_Tree_Node*, Swc_Tree_Node*> > matchingResult =
          matchG(branch1, branch2, currentLevel);
      currentScore += m_matchingScore;

      /* Update the source (enqueue) */
      updateMatchingSource(&sourceQueue, matchingResult);

      /* Update the matching result */
      m_matchingResult.insert(m_matchingResult.end(), matchingResult.begin(),
                              matchingResult.end());
    }
  }

  m_matchingScore = currentScore;

  double s1 = tree1.length();
  double s2 = tree2.length();

  if (s1 < s2) {
    m_matchingScore /= sqrt(s2);
  } else {
    m_matchingScore /= sqrt(s1);
  }
  m_matchingScore *= 1000.0;

#if 0
    Swc_Tree_Node *leaf1 =
        Swc_Tree_Node_Furthest_Leaf_E(source.node1, source.exclude1);
    Swc_Tree_Node *leaf2 =
        Swc_Tree_Node_Furthest_Leaf_E(source.node2, source.exclude2);

    if (source.node1 != leaf1 && source.node2 != leaf2) {
      ZSwcBranch branch1(source.node1, leaf1);
      ZSwcBranch branch2(source.node2, leaf2);

      /* Match the trunk pair */
      vector<pair<Swc_Tree_Node*, Swc_Tree_Node*> > matchingResult =
          match(branch1, branch2);

      /* Update the source (enqueue) */
      updateMatchingSource(&sourceQueue, matchingResult);

      /* Update the matching result */
      m_matchingResult.insert(m_matchingResult.end(), matchingResult.begin(),
                              matchingResult.end());
    }
  }
#endif

}

ZSwcTree* ZSwcTreeMatcher::exportResultAsSwc(EResultExportOption option)
{
  if (matchingResult().empty()) {
    return NULL;
  }

  ZSwcTree *matchingSwc = new ZSwcTree;

  matchingSwc->forceVirtualRoot();

  for (size_t i = 0; i < m_matchingResult.size(); i++) {
    bool exporting = true;

    switch (option) {
    case EXPORT_CRITICAL_MATCHING:
      if (Swc_Tree_Node_Is_Continuation(m_matchingResult[i].first) &&
          Swc_Tree_Node_Is_Continuation(m_matchingResult[i].second)) {
        exporting = false;
      }
      break;
    default:
      break;
    }

    if (exporting) {
      ZPoint startPos(m_matchingResult[i].first->node.x,
                      m_matchingResult[i].first->node.y,
                      m_matchingResult[i].first->node.z);
      ZPoint endPos(m_matchingResult[i].second->node.x,
                    m_matchingResult[i].second->node.y,
                    m_matchingResult[i].second->node.z);

      Swc_Tree_Node *tn =
          ZSwcTree::makeArrow(startPos, 1, 6, endPos, 0.2, 0);
      Swc_Tree_Node_Set_Parent(tn, matchingSwc->data()->root);
    }
  }

  matchingSwc->resortId();

  return matchingSwc;
}
