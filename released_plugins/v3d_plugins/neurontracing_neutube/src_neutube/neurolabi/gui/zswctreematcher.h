#ifndef ZSWCTREEMATCHER_H
#define ZSWCTREEMATCHER_H

#include <map>
#include <vector>
#include <utility>
#include <math.h>
#include <queue>
#include "zmatrix.h"
#include "zrandomgenerator.h"
#include "zpoint.h"
#include "tz_swc_tree.h"
#include "zswcpath.h"

class ZSwcBranch;
class ZSwcTree;
class ZSwcFeatureAnalyzer;
class ZSwcTrunkAnalyzer;

/* Source: node 1, exclude 1, node 2, exclude 2*/
struct MatchingSource {
  Swc_Tree_Node *node1;
  Swc_Tree_Node *exclude1;
  Swc_Tree_Node *node2;
  Swc_Tree_Node *exclude2;
};

class ZSwcTreeMatcher
{
public:
    ZSwcTreeMatcher();

public:
    std::map<int, int> matchBranch(ZSwcBranch *branch1, ZSwcBranch *branch2);
    std::map<int, int> matchBranchByLocalSholl(
        ZSwcBranch *branch1, ZSwcBranch *branch2,
        ZSwcTree *host1, ZSwcTree *host2, bool normalized = false);

    std::vector<std::vector<double> > computePairwiseMatchingScore(
        std::vector<ZSwcTree*> treeArray);

    template <typename T>
    static double computeFeatureSimilarity(const std::vector<T> &feature1,
                                           const std::vector<T> &feature2,
                                           bool consideringSize = true);
    template<typename T>
    double computeFeatureArraySimilarity(
        const std::vector<std::vector<T> > &featureArray1,
        const std::vector<std::vector<T> > &featureArray2,
        bool normalized = true);

    template<typename T>
    double computeFeatureArraySimilarityZScore(
        const std::vector<std::vector<T> > &featureArray1,
        const std::vector<std::vector<T> > &featureArray2);

    void setShollAnalysisParameters(double start, double end, double radius);

    void debugOutputMatching(const std::map<int, int> &match);

    template<typename T>
    std::vector<double> computeRandomFeatureSimilarity(
        const std::vector<std::vector<T> > &featureArray1,
        const std::vector<std::vector<T> > &featureArray2,
        bool normalized = true
        );

public: //fake public routines
    std::map<int, int> dynamicProgrammingMatch(const ZMatrix &simMat,
                                               double gapPenalty = 0.5);

    std::vector<std::vector<double> > expandFeatureArray(
        const std::vector<std::vector<double> > &featureArray,
        const std::vector<double> &distanceArray, int index);

    double computeTreeSimilarity(ZSwcTree *tree1, ZSwcTree *tree2);

    inline double matchingScore() { return m_matchingScore; }

    std::vector<std::vector<double> > computeFeatureArray(
        ZSwcBranch *branch, ZSwcTree *host);

    inline std::vector<ZPoint>& firstPointArray() { return m_pointArray1; }
    inline std::vector<ZPoint>& secondPointArray() { return m_pointArray2; }

    inline std::vector<std::vector<double> >& firstFeatureArray() {
      return m_featureArray1;
    }
    inline std::vector<std::vector<double> >& secondFeatureArray() {
      return m_featureArray2;
    }

    inline std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> >&
    matchingResult() { return m_matchingResult; }

    std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> >
      match(ZSwcBranch &branch1, ZSwcBranch &branch2);
    void match(ZSwcTree &tree1, ZSwcTree &tree2);

    std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> >
    matchG(ZSwcPath &branch1, ZSwcPath &branch2, int level);

    double matchAll(ZSwcTree &tree1, ZSwcTree &tree2);
    void matchAllG(ZSwcTree &tree1, ZSwcTree &tree2, int level = 65535);

    inline void setFeatureAnalyzer(ZSwcFeatureAnalyzer *analyzer) {
      m_featureAnalyzer = analyzer;
    }

    inline void setTrunkAnalyzer(ZSwcTrunkAnalyzer *analyzer) {
      m_trunkAnalyzer = analyzer;
    }

    enum EResultExportOption {
      EXPORT_ALL_MATCHING, EXPORT_CRITICAL_MATCHING
    };

    ZSwcTree *exportResultAsSwc(
        EResultExportOption option = EXPORT_CRITICAL_MATCHING);

private:
    void updateMatchingSource(std::queue<MatchingSource> *sourceQueue,
                              const std::vector<
                              std::pair<Swc_Tree_Node*, Swc_Tree_Node*> >
                              &matching);

private:
    double m_matchingScore;
    double m_shollStart;
    double m_shollEnd;
    double m_shollRadius;
    std::vector<std::map<int, int> > m_gapMap;
    std::vector<ZPoint> m_pointArray1;
    std::vector<ZPoint> m_pointArray2;
    std::vector<std::vector<double> > m_featureArray1;
    std::vector<std::vector<double> > m_featureArray2;

    std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> > m_matchingResult;
    ZSwcFeatureAnalyzer *m_featureAnalyzer;
    ZSwcTrunkAnalyzer *m_trunkAnalyzer;
};

template <typename T>
double ZSwcTreeMatcher::computeFeatureSimilarity(
    const std::vector<T> &feature1, const std::vector<T> &feature2,
    bool consideringSize)
{
  double score = 0.0;
  if (feature1.size() > 0 && feature1.size() == feature2.size()) {
    double sum1 = 0.0;
    double sum2 = 0.0;
    for (std::size_t i = 0; i < feature1.size(); i++) {
      double fv1 = feature1[i];
      double fv2 = feature2[i];
      if ((fv1 != 0.0) || (fv2 != 0.0)) {
        score += (fv1 - fv2) * (fv1 - fv2) /
            (fv1 + fv2);
      }

      sum1 += fv1;
      sum2 += fv2;
    }

    score += fabs(sum1 - sum2);

    if (consideringSize) {
      score = sqrt(sum1 + sum2) / (1.0 + score);
    } else {
      score = 1.0 / (1.0 + score);
    }
  } else if (feature1.empty() && feature2.empty()) {
    score = 0.1;
  }

  return score;
}

template <typename T>
double ZSwcTreeMatcher::computeFeatureArraySimilarity(
    const std::vector<std::vector<T> > &featureArray1,
    const std::vector<std::vector<T> > &featureArray2,
    bool normalized)
{
  ZMatrix simMat(featureArray1.size(), featureArray2.size());
  for (int i = 0; i < simMat.getRowNumber(); i++) {
    for (int j = 0; j < simMat.getColumnNumber(); j++) {
      simMat.set(i, j, computeFeatureSimilarity(featureArray1[i],
                                                featureArray2[j]));
    }
  }

  //simMat.debugOutput();

  //Run dynamic programming
  dynamicProgrammingMatch(simMat, 0.5);

  if (normalized) {
    double sum1 = 0.0;
    double sum2 = 0.0;
    for (std::size_t i = 0; i < featureArray1.size(); i++) {
      sum1 += computeFeatureSimilarity(featureArray1[i],
                                       featureArray1[i]);
    }

    for (std::size_t i = 0; i < featureArray2.size(); i++) {
      sum2 += computeFeatureSimilarity(featureArray2[i],
                                       featureArray2[i]);
    }

    m_matchingScore /= (sum1 + sum2) * 0.5;
  }

  return m_matchingScore;
}

template <typename T>
std::vector<double> ZSwcTreeMatcher::computeRandomFeatureSimilarity(
    const std::vector<std::vector<T> > &featureArray1,
    const std::vector<std::vector<T> > &featureArray2,
    bool normalized
    )
{
  //Permute feature array2
  std::vector<std::vector<T> > permuteFeature(featureArray2.size());

  std::vector<double> similarityArray(20);

  ZRandomGenerator rand;
  for (std::size_t i = 0; i < similarityArray.size(); i++) {
    rand.setSeed(i);
    std::vector<int> perm = rand.randperm(featureArray2.size());
    for (std::size_t j = 0; j < featureArray2.size(); j++) {
      permuteFeature[j] = featureArray2[perm[j] - 1];
    }

    similarityArray[i] = computeFeatureArraySimilarity(featureArray1,
                                                       permuteFeature,
                                                       normalized);
  }


  return similarityArray;
}

template<typename T>
double ZSwcTreeMatcher::computeFeatureArraySimilarityZScore(
    const std::vector<std::vector<T> > &featureArray1,
    const std::vector<std::vector<T> > &featureArray2)
{
  double score = computeFeatureArraySimilarity(featureArray1, featureArray2);

  std::vector<double> scoreArray = computeRandomFeatureSimilarity(
        featureArray1, featureArray2);

  double mu = 0.0;
  double sigma = 0.0;

  for (std::size_t i = 0; i < scoreArray.size(); i++) {
    sigma += scoreArray[i] * scoreArray[i];
    mu += scoreArray[i];
  }

  sigma /= scoreArray.size() - 1;
  sigma -= mu * mu / scoreArray.size() / (scoreArray.size() - 1);
  mu /= scoreArray.size();
  sigma = sqrt(sigma);

  double z = (score - mu) / sigma;

  return z;
}

#endif // ZSWCTREEMATCHER_H
