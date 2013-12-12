#include "zswcshollfeatureanalyzer.h"

#include <iostream>
#include <algorithm>

using namespace std;

ZSwcShollFeatureAnalyzer::ZSwcShollFeatureAnalyzer()
{
  m_shollStart = 5.0;
  m_shollEnd = 50.0;
  m_shollRadius = 5.0;
}

void ZSwcShollFeatureAnalyzer::setParameter(
    const std::vector<double> &parameterArray)
{
  m_shollStart = parameterArray[0];
  m_shollEnd = parameterArray[1];
  m_shollRadius = parameterArray[2];
}

struct SwcNodePair {
    Swc_Tree_Node *firstNode;
    Swc_Tree_Node *secondNode;
    double first;
    double second;
};

struct SwcNodePairLessThan {
    bool operator() (const SwcNodePair &p1, const SwcNodePair &p2)
    {
        if (p1.first < p2.first) {
            return true;
        }

        return false;
    }
};

vector<double> ZSwcShollFeatureAnalyzer::computeFeature(Swc_Tree_Node *tn)
{
  vector<SwcNodePair> distanceArray;
  vector<double> crossingNumberArray;
  ZPoint center(tn->node.x, tn->node.y, tn->node.z);

  ZSwcTree sourceTree;
  sourceTree.setDataFromNodeRoot(tn);

  //cout << sourceTree.data()->root << endl;

  sourceTree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, false);
  double maxLength = 0.0;
  for (Swc_Tree_Node *tn = sourceTree.begin(); tn != sourceTree.end();
       tn = sourceTree.next()) {
    if (Swc_Tree_Node_Is_Regular(tn) && !Swc_Tree_Node_Is_Root(tn)) {
      //Compute the central distances of the current node and its parent
      SwcNodePair distancePair;
      ZPoint v1(Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn),
                Swc_Tree_Node_Z(tn));
      ZPoint v2(Swc_Tree_Node_X(tn->parent),
                Swc_Tree_Node_Y(tn->parent),
                Swc_Tree_Node_Z(tn->parent));
      double d1 = v1.distanceTo(center);
      double d2 = v2.distanceTo(center);

      //Make sure that distancePair.first < distancePair.second
      if (d1 > d2) {
        distancePair.first = d2;
        distancePair.second = d1;
        distancePair.firstNode = tn->parent;
        distancePair.secondNode = tn;
      } else {
        distancePair.first = d1;
        distancePair.second = d2;
        distancePair.firstNode = tn;
        distancePair.secondNode = tn->parent;
      }

      //Calculate the distance between v1 and v2
      double length = v1.distanceTo(v2);
      if (length > maxLength) {
        maxLength = length;
      }
      distanceArray.push_back(distancePair);
    }
  }

  sort(distanceArray.begin(), distanceArray.end(),
       SwcNodePairLessThan());

  int startIndex = 0;
  int endIndex = 0;
  int lastIndex = int(distanceArray.size()) - 1;

  for (double r = m_shollStart; r <= m_shollEnd; r += m_shollRadius) {
    if (startIndex <= lastIndex) {
      //Update start index and end index
      while (distanceArray[startIndex].first < r - maxLength) {
        startIndex++;
        if (startIndex > lastIndex) {
          break;
        }
      }

      if (endIndex <= lastIndex) {
        while (distanceArray[endIndex].first < r) {
          endIndex++;
          if (endIndex > lastIndex) {
            break;
          }
        }
      }


      //Crossing test
      int crossingNumber = 0;
      if (startIndex <= lastIndex) {
        for (int i = startIndex; i < endIndex; ++i) {
          //If a crossing point is detected
          if (distanceArray[i].second >= r) {
            crossingNumber += 1.0;
          }
        }
      }

      crossingNumberArray.push_back(crossingNumber);
    } else {
      crossingNumberArray.push_back(0);
    }
  }

  //cout << sourceTree.data()->root << endl;

  sourceTree.setData(NULL, ZSwcTree::FREE_WRAPPER);

  return crossingNumberArray;
}

double ZSwcShollFeatureAnalyzer::computeFeatureSimilarity(
    const vector<double> &featureArray1, const vector<double> &featureArray2)
{
  double score = 0.0;
  if (featureArray1.size() > 0 && featureArray1.size() == featureArray2.size()) {
    double sum1 = 0.0;
    double sum2 = 0.0;
    for (std::size_t i = 0; i < featureArray1.size(); i++) {
      double fv1 = featureArray1[i];
      double fv2 = featureArray2[i];
      if ((fv1 != 0.0) || (fv2 != 0.0)) {
        score += (fv1 - fv2) * (fv1 - fv2) /
            (fv1 + fv2);
      }

      sum1 += fv1;
      sum2 += fv2;
    }

    score += fabs(sum1 - sum2);
    score = sqrt(sum1 + sum2) / (1.0 + score);
  } else if (featureArray1.empty() && featureArray2.empty()) {
    score = 0.1;
  }

  return score;
}
