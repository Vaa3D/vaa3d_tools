#include "zsynapselocationmatcher.h"

#include <algorithm>
#include <iostream>
#include <fstream>

#include "zstring.h"
#include "zsynapselocationmetric.h"

using namespace std;

FlyEm::ZSynapseLocationMatcher::ZSynapseLocationMatcher() :
  m_metric(NULL), m_verbose(1)
{
}

FlyEm::ZSynapseLocationMatcher::ZSynapseLocationMatcher(
    const ZSynapseLocationMatcher &matcher) :
  m_matchedPairArray(matcher.m_matchedPairArray),
  m_unmatchedIndices1(matcher.m_unmatchedIndices1),
  m_unmatchedIndices2(matcher.m_unmatchedIndices2),
  m_trueMatchNumber(matcher.m_trueMatchNumber),
  m_falseMatch(matcher.m_falseMatch),
  m_missingMatch(matcher.m_missingMatch),
  m_metric(matcher.m_metric),
  m_verbose(matcher.m_verbose)
{
}

void FlyEm::ZSynapseLocationMatcher::clearResult()
{
  m_matchedPairArray.clear();
  m_unmatchedIndices1.clear();
  m_unmatchedIndices2.clear();
}

std::vector<FlyEm::WeightedIntPair> FlyEm::ZSynapseLocationMatcher::
computePairwiseDistance(const std::vector<FlyEm::SynapseLocation *> &loc1,
                        const std::vector<FlyEm::SynapseLocation *> &loc2,
                        double maxDist)
{
  std::vector<WeightedIntPair> distanceArray;

  for (size_t i = 0; i < loc1.size(); i++) {
    for (size_t j = 0; j < loc2.size(); j++) {
      //double dist = loc1[i]->pos().distanceTo(loc2[j]->pos());
      double dist = matchCost(*loc1[i], *loc2[j]);
      if (dist <= maxDist) {
        distanceArray.push_back(WeightedIntPair(i, j, dist));
      }
    }
  }

  return distanceArray;
}

void FlyEm::ZSynapseLocationMatcher::match(
    const std::vector<FlyEm::SynapseLocation*> &loc1,
    const std::vector<FlyEm::SynapseLocation*> &loc2,
    double maxDist)
{
  clearResult();

  std::vector<WeightedIntPair> distanceArray =
      computePairwiseDistance(loc1, loc2, maxDist);

  std::sort(distanceArray.begin(), distanceArray.end(),
            WeightedIntPairCompare());


#ifdef _DEBUG_2
  for (size_t i = 0; i < distanceArray.size(); i++) {
    cout << distanceArray[i].first() << " " << distanceArray[i].second() << " "
         << distanceArray[i].weight() << endl;
  }
#endif


  std::vector<int> matchingArray1(loc1.size(), -1);
  std::vector<int> matchingArray2(loc2.size(), -1);

  //Extract matchings
  for (size_t i = 0; i < distanceArray.size(); i++) {
    if (matchingArray1[distanceArray[i].first()] < 0 &&
        matchingArray2[distanceArray[i].second()] < 0) {
      matchingArray1[distanceArray[i].first()] = distanceArray[i].second();
      matchingArray2[distanceArray[i].second()] = distanceArray[i].first();

      m_matchedPairArray.push_back(pair<int, int>(distanceArray[i].first(),
                                                  distanceArray[i].second()));
    } else {
      if (m_verbose > 1) {
        cout << "Warning: mutliple matches detected." << endl;
        int index1 = distanceArray[i].first();
        int index2 = matchingArray1[index1];
        if (index2 < 0) {
          index2 = distanceArray[i].second();
          index1 = matchingArray2[index2];
        }
        cout << "Current match: " << loc1[index1]->pos().toString() << "--"
             << loc2[index2]->pos().toString() << endl;
        cout << "Another possibility: "
             << loc1[distanceArray[i].first()]->pos().toString() << "--"
             << loc2[distanceArray[i].second()]->pos().toString() << endl;
      }
    }
  }

  for (size_t i = 0; i < matchingArray1.size(); i++) {
    if (matchingArray1[i] < 0) {
      m_unmatchedIndices1.push_back(i);
    }
  }

  if (m_verbose > 1) {
    cout << "Unmatched size: " << m_unmatchedIndices1.size() << endl;
  }

  for (size_t i = 0; i < matchingArray2.size(); i++) {
    if (matchingArray2[i] < 0) {
      m_unmatchedIndices2.push_back(i);
    }
  }

  if (m_verbose > 1) {
    cout << "Unmatched size: " << m_unmatchedIndices2.size() << endl;
  }
}

int FlyEm::ZSynapseLocationMatcher::
getIndex(int i, FlyEm::ZSynapseLocationMatcher::ESide side,
         FlyEm::ZSynapseLocationMatcher::EResult result) const
{
  int index = -1;

  switch (result) {
  case MATCHED:
    switch (side) {
    case LEFT_SIDE:
      index = m_matchedPairArray[i].first;
      break;
    case RIGHT_SIDE:
      index = m_matchedPairArray[i].second;
      break;
    }
    break;
  case UNMATCHED:
    switch (side) {
    case LEFT_SIDE:
      index = m_unmatchedIndices1[i];
      break;
    case RIGHT_SIDE:
      index = m_unmatchedIndices2[i];
      break;
    }
    break;
  }

  return index;
}

size_t FlyEm::ZSynapseLocationMatcher::unmatchedSize(ESide side)
{
  size_t s = 0;
  switch (side) {
  case LEFT_SIDE:
    s = m_unmatchedIndices1.size();
    break;
  case RIGHT_SIDE:
    s = m_unmatchedIndices2.size();
    break;
  }

  return s;
}

double FlyEm::ZSynapseLocationMatcher::matchCost(const SynapseLocation &loc1,
                                                 const SynapseLocation &loc2)
{
  if (m_metric == NULL) {
    return loc1.pos().distanceTo(loc2.pos());
  } else {
    return m_metric->distance(loc1, loc2);
  }
}

Intpair_Map* FlyEm::ZSynapseLocationMatcher::toIntPairMap() const
{
  Intpair_Map *intpairMap = NULL;

  if (size() > 0) {
    intpairMap = New_Intpair_Map();
  }

  for (size_t i = 0; i < size(); i++) {
    int index1 = getIndex(i, LEFT_SIDE);
    int index2 = getIndex(i, RIGHT_SIDE);
    Intpair_Map_Add(intpairMap, index1, index2, i);
  }

  return intpairMap;
}

void FlyEm::ZSynapseLocationMatcher::evaluate(
    const ZSynapseLocationMatcher &golden)
{
  m_trueMatchNumber = static_cast<int>(golden.size());

  Intpair_Map *map1 = this->toIntPairMap();
  Intpair_Map *map2 = golden.toIntPairMap();

  for (size_t i = 0; i < size(); i++) {
    int index1 = getIndex(i, LEFT_SIDE);
    int index2 = getIndex(i, RIGHT_SIDE);
    if (Intpair_Map_Value(map2, index1, index2) < 0) {
      m_falseMatch.push_back(pair<int, int>(index1, index2));
    }
  }

  for (size_t i = 0; i < golden.size(); i++) {
    int index1 = golden.getIndex(i, LEFT_SIDE);
    int index2 = golden.getIndex(i, RIGHT_SIDE);
    if (Intpair_Map_Value(map1, index1, index2) < 0) {
      m_missingMatch.push_back(pair<int, int>(index1, index2));
    }
  }
}

void FlyEm::ZSynapseLocationMatcher::printPerformance() const
{
  int tp = size() - m_falseMatch.size();

  cout << "Precision: " << tp << "/" << size() << " = "
       << static_cast<double>(tp) /size() << endl;
  cout << "Recall: " << tp << "/" << m_trueMatchNumber << " = "
       << static_cast<double>(tp) /m_trueMatchNumber << endl;

  double fMeasure = (2.0 * tp) / (size() + m_trueMatchNumber);
  cout << "F-Measure: " << fMeasure << endl;
}

void FlyEm::ZSynapseLocationMatcher::exportPerformance(const string &filePath) const
{
  ofstream stream(filePath.c_str());

  stream << "<html>" << endl;
  stream << "<head><title>matching performance</title></head>" << endl;
  stream << "<body>" << endl;
  stream << "<table>" << endl;
  stream << "<tr><td></td><td>Number</td><td>Percentage</td></tr>" << endl;
  int tp = size() - m_falseMatch.size();
  stream << "<tr><td>Precision</td><td align=\"center\">" << tp << "/" << size()
         << "</td><td align=\"center\">" << static_cast<double>(tp) /size()
         << "</td>"
         << endl;
  stream << "<tr><td>Recall</td><td align=\"center\">" << tp << "/"
         << m_trueMatchNumber
         << "</td><td align=\"center\">"
         << static_cast<double>(tp) / m_trueMatchNumber
         << "</td>" << endl;
  stream << "</table>" << endl;

  double fMeasure = (2.0 * tp) / (size() + m_trueMatchNumber);
  stream << "<p>F-Measure: " << fMeasure << "</p>" << endl;
  stream << "</body>" << endl;
  stream << "</html>" << endl;

#ifdef _DEBUG_
  cout << filePath << " generated" << endl;
#endif

  stream.close();
}

void FlyEm::ZSynapseLocationMatcher::load(const ZSynapseAnnotationArray &sa1,
                                          const ZSynapseAnnotationArray &sa2,
                                          const string filePath)

{
  m_matchedPairArray.clear();
  ZString line;
  FILE *fp = fopen(filePath.c_str(), "r");

  if (ZString(filePath).endsWith(".txt")) {
    while (line.readLine(fp)) {
      vector<int> coords = line.toIntegerArray();
      if (coords.size() == 6) {
        int index1 = sa1.getPsdIndex(coords[0], coords[1], coords[2]);
        int index2 = sa2.getPsdIndex(coords[3], coords[4], coords[5]);
        if (index1 >= 0 && index2 >= 0) {
          m_matchedPairArray.push_back(pair<int, int>(index1, index2));
        }
      }
    }
  } else if (ZString(filePath).endsWith(".csv")) {
    while (line.readLine(fp)) {
      vector<int> coords = line.toIntegerArray();
      if (coords.size() == 9) {
        int index1 = sa1.getPsdIndex(coords[3], coords[4], coords[5]);
        int index2 = sa2.getPsdIndex(coords[6], coords[7], coords[8]);
        if (index1 >= 0 && index2 >= 0) {
          m_matchedPairArray.push_back(pair<int, int>(index1, index2));
        }
      }
    }
  }

  fclose(fp);
}

void FlyEm::ZSynapseLocationMatcher::matchTBar(
    FlyEm::ZSynapseAnnotationArray &sa1,
    FlyEm::ZSynapseAnnotationArray &sa2, double maxDist)
{
  vector<FlyEm::SynapseLocation*> tbarSet1 = sa1.toTBarRefArray();
  vector<FlyEm::SynapseLocation*> tbarSet2 = sa2.toTBarRefArray();
  match(tbarSet1, tbarSet2, maxDist);
}

void FlyEm::ZSynapseLocationMatcher::addUnmatched(
    ESide side, const ZSynapseAnnotationArray &sa,
    size_t tbarIndex, size_t psdIndex)
{
  int index = sa.getPsdIndex(tbarIndex, psdIndex);

  switch (side) {
  case LEFT_SIDE:
    m_unmatchedIndices1.push_back(index);
    break;
  case RIGHT_SIDE:
    m_unmatchedIndices2.push_back(index);
    break;
  default:
    break;
  }
}

FlyEm::ZSynapseLocationMatcher FlyEm::ZSynapseLocationMatcher::matchPsd(
    FlyEm::ZSynapseAnnotationArray &sa1,
    FlyEm::ZSynapseAnnotationArray &sa2,
    double maxDist, double tbarMaxDist)
{
  clearResult();

  ZSynapseLocationMatcher tbarMatcher;
  tbarMatcher.setVerbose(m_verbose);

  if (m_verbose > 0) {
    cout << "Matching TBars ..." << endl;
  }

  tbarMatcher.matchTBar(sa1, sa2, tbarMaxDist);

  if (m_verbose > 0) {
    cout << "Matching TBars done." << endl;
    if (tbarMatcher.unmatchedSize(LEFT_SIDE) > 0) {
      cout << tbarMatcher.unmatchedSize(LEFT_SIDE) << " TBars unmatched in "
           << sa1.source();
    }
    if (tbarMatcher.unmatchedSize(RIGHT_SIDE) > 0) {
      cout << tbarMatcher.unmatchedSize(RIGHT_SIDE) << " TBars unmatched in "
           << sa2.source();
    }
  }


  if (m_verbose > 0) {
    cout << "Matching PSDs ..." << endl;
  }

  ESide sideList[2] = {LEFT_SIDE, RIGHT_SIDE};
  ZSynapseAnnotationArray* pSa[2] = {&sa1, &sa2};

  for (size_t tbarMatchIndex = 0; tbarMatchIndex < tbarMatcher.size();
       tbarMatchIndex++) {
    ZSynapseLocationMatcher psdMatcher;
    psdMatcher.setVerbose(m_verbose);
    int tbarIndex[2];
    tbarIndex[0] = tbarMatcher.getIndex(tbarMatchIndex, LEFT_SIDE);
    tbarIndex[1] = tbarMatcher.getIndex(tbarMatchIndex, RIGHT_SIDE);

    vector<FlyEm::SynapseLocation*> psdSet[2];
    psdSet[0] = sa1.toPsdRefArray(tbarIndex[0]);
    psdSet[1] = sa2.toPsdRefArray(tbarIndex[1]);

    if (m_metric != NULL) {
      m_metric->setRefPoint(
            sa1.getTBarRef(tbarMatcher.getIndex(tbarMatchIndex, LEFT_SIDE))->pos());
      psdMatcher.setMetric(m_metric);
    }
    psdMatcher.match(psdSet[0], psdSet[1], maxDist);
    for (size_t psdMatchIndex = 0; psdMatchIndex < psdMatcher.size();
         psdMatchIndex++) {
      m_matchedPairArray.push_back(
            pair<int, int>(sa1.getPsdIndex(tbarIndex[0],
                           psdMatcher.getIndex(psdMatchIndex, LEFT_SIDE)),
            sa2.getPsdIndex(tbarIndex[1],
            psdMatcher.getIndex(psdMatchIndex, RIGHT_SIDE))));
    }

    for (size_t sideIndex = 0; sideIndex < 2; sideIndex++) {
      ESide side = sideList[sideIndex];
      for (size_t unmatchedIndex = 0;
           unmatchedIndex < psdMatcher.unmatchedSize(side); unmatchedIndex++) {
        addUnmatched(side, *(pSa[sideIndex]), tbarIndex[sideIndex],
                     psdMatcher.getIndex(unmatchedIndex, side, UNMATCHED));
      }
    }
  }

  if (m_verbose > 0) {
    cout << "Matching PSDs done" << endl;
    cout << "  " << size() << " pairs of PSDs matched" << endl;
    cout << "  " << sa1.getPsdNumber() - size() << " PSDs unmatched in "
         << sa1.source() << endl;
    cout << "  " << sa2.getPsdNumber() - size() << " PSDs unmatched in "
         << sa2.source() << endl;
    cout << "-----------" << endl;
    cout << "Overall agreement: " << static_cast<double>(size() * 2) /
            (sa1.getPsdNumber() + sa2.getPsdNumber()) << endl << endl;
  }

  return tbarMatcher;
}
