#ifndef ZSYNAPSELOCATIONMATCHER_H
#define ZSYNAPSELOCATIONMATCHER_H

#include <utility>
#include <vector>
#include "zsynapseannotationarray.h"
#include "tz_intpair_map.h"

namespace FlyEm {

class ZSynapseLocationMetric;

class ZSynapseLocationMatcher
{
public:
  ZSynapseLocationMatcher();
  ZSynapseLocationMatcher(const ZSynapseLocationMatcher &matcher);

  void match(const std::vector<SynapseLocation*> &loc1,
             const std::vector<SynapseLocation*> &loc2,
             double distThre);

  void clearResult();

  inline void setMetric(ZSynapseLocationMetric *metric) {
    m_metric = metric;
  }

  enum ESide {
    LEFT_SIDE, RIGHT_SIDE
  };

  enum EResult  {
    MATCHED, UNMATCHED
  };

  inline std::size_t size() const { return m_matchedPairArray.size(); }
  std::size_t unmatchedSize(ESide side);

  int getIndex(int i, ESide side, EResult result = MATCHED) const;

  double matchCost(const SynapseLocation &loc1, const SynapseLocation &loc2);

  void evaluate(const ZSynapseLocationMatcher &golden);
  void printPerformance() const;
  void exportPerformance(const std::string &filePath) const;

  void load(const ZSynapseAnnotationArray &sa1,
            const ZSynapseAnnotationArray &sa2,
            const std::string filePath);

  void matchTBar(ZSynapseAnnotationArray &sa1,
                ZSynapseAnnotationArray &sa2, double maxDist);

  ZSynapseLocationMatcher matchPsd(ZSynapseAnnotationArray &sa1,
                ZSynapseAnnotationArray &sa2, double maxDist,
                double tbarMaxDist);

  Intpair_Map* toIntPairMap() const;

  std::vector<WeightedIntPair> computePairwiseDistance(
      const std::vector<FlyEm::SynapseLocation*> &loc1,
      const std::vector<FlyEm::SynapseLocation*> &loc2,
      double maxDist);

  inline const std::vector<std::pair<int, int> >& falseMatch() {
    return m_falseMatch; }
  inline  const std::vector<std::pair<int, int> >& missingMatch() {
    return m_missingMatch; }

  inline void setVerbose(int v) { m_verbose = v; }

  void addUnmatched(ESide side, const ZSynapseAnnotationArray &sa,
                    size_t tbarIndex, size_t psdIndex);

private:
  std::vector<std::pair<int, int> > m_matchedPairArray;
  std::vector<int> m_unmatchedIndices1;
  std::vector<int> m_unmatchedIndices2;

  int m_trueMatchNumber;
  std::vector<std::pair<int, int> > m_falseMatch;
  std::vector<std::pair<int, int> > m_missingMatch;

  ZSynapseLocationMetric *m_metric;
  int m_verbose;
};

}

#endif // ZSYNAPSELOCATIONMATCHER_H
