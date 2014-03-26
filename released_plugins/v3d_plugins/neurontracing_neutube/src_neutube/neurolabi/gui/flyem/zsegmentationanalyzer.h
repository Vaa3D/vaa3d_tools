#ifndef ZSEGMENTATIONANALYZER_H
#define ZSEGMENTATIONANALYZER_H

#include <utility>
#include <vector>
#include <map>
#include <string>

#include "zsuperpixelmaparray.h"
#include "zstack.hxx"
#include "zintpairmap.h"
#include "zintmap.h"
#include "zgraph.h"
#include "zsegmentationbundle.h"

namespace FlyEm {
class ZSegmentationAnalyzer
{
public:
  ZSegmentationAnalyzer();

public:
  //ZSegmentationDifference& getSegmentationDifference();

  void compare(const ZSuperpixelMapArray &trueSeg,
               const ZSuperpixelMapArray &testSeg);
  std::vector<std::pair<int, int> > getFalseMerge();
  std::vector<std::pair<int, int> > getFalseSplit();

  ZStack* createErrorStack(const ZStack &superpixel);

  static int channelCodeToId(const std::vector<uint8_t> &channelCode);
  static int channelCodeToId(const std::vector<double> &channelCode);
  static int channelCodeToId(const uint8_t *array, int channelNumber,
                             size_t index, size_t stride);
  static std::vector<uint8_t> idToChannelCode(int id, int channelNumber);

  static std::vector<double> computeTouchFeature(
      Stack *stack, const std::vector<int> &seed);
  static std::vector<double> computeRayburstTouchFeature(
      Stack **stack, std::vector<int> seed[]);
  static std::vector<double> computeRayburstTouchFeature(
      Stack **stack, std::vector<int> seed);

  static std::vector<double> touchFeature(
      const ZStack &stack, int id1, int id2);
  static std::vector<double> touchFeature(
      const ZStack &stack, const std::vector<double> &c1,
      const std::vector<double> &c2);
  static std::vector<std::vector<double> > touchFeature(
      const ZStack &stack, const ZGraph &bodyGraph);

  static std::vector<double> touchFeature(
      FlyEm::ZSegmentationBundle &bundle, int id1, int id2);
  static std::vector<std::vector<double> > touchFeature(
      FlyEm::ZSegmentationBundle &bundle);

  static std::vector<double> touchFeature(
      const ZStack &stack, int id1, int id2, const ZStack &mask);
  static std::vector<double> touchFeature(
      const ZStack &stack, const std::vector<double> &c1,
      const std::vector<double> &c2, const ZStack &mask);
  static std::vector<std::vector<double> > touchFeature(
      const ZStack &stack, const ZGraph &bodyGraph, const ZStack &mask);

  static ZIntPairMap computeOverlap(ZStack *stack1, ZStack *stack2,
                                     ZStack *excluded = NULL);

  static ZIntMap computeBodySize(ZStack *stack, ZStack *excluded = NULL);
  static ZIntMap inferBodyCorrespondence(const ZIntPairMap &overlap,
                                         const ZIntMap &bodySize);

  static std::map<int, double> computeAverageIntensity(
      ZStack *bodyStack, ZStack *intensityStack);

  static std::vector<double> computeRayburstDirection(
      const Stack *stack, const int *seedArray, size_t seedNumber);
  static std::vector<double> computeRayburstDirection(
      const Stack *stack, int seed);

  static std::vector<double> computeTouchFeature(
      FlyEm::ZSegmentationBundle &bundle, int id1, int id2);
  static std::vector<double> computeRayburstTouchFeature(
      ZSegmentationBundle &bundle, int id1, int id2);

  static std::vector<double> computeBoundaryGrowFeature(
      FlyEm::ZSegmentationBundle &bundle, int id1, int id2);

  static double computeBorderIntensity(
      FlyEm::ZSegmentationBundle &bundle, int id1, int id2);

  static double computeBodyIntensity(
      FlyEm::ZSegmentationBundle &bundle, int id);

  enum BcfSet {
    BCF_BODY_GROW, BCF_BOUNDARY_GROW, BCF_RAYBURST, BCF_BORDER_SIZE,
    BCF_BORDER_INTENSITY, BCF_BODY_SIZE, BCF_GALA_SEGMENTATION,
    BCF_BODY_INTENSITY, BCF_BOUNDARY_PROB, BCF_MITO_PROB, BCF_CYTO_PROB
  };
  static std::vector<double> computeBcf(
      FlyEm::ZSegmentationBundle &bundle, int id1, int id2,
      BcfSet setName);

  static std::vector<double> computeBcf(
      FlyEm::ZSegmentationBundle &bundle, int id1, int id2,
      const std::vector<BcfSet> &setName);

  static std::vector<double> computeBcf(
      FlyEm::ZSegmentationBundle &bundle, int id1, int id2,
      const std::vector<std::string> &setName);

  static std::vector<std::vector<double> > computeBcf(
      FlyEm::ZSegmentationBundle &bundle, BcfSet setName);
  static std::vector<std::vector<double> > computeBcf(
      FlyEm::ZSegmentationBundle &bundle, const std::string &setName);

  static std::vector<std::vector<double> > computeBcf(
      FlyEm::ZSegmentationBundle &bundle,
      const std::vector<std::string> &setName);

  static void generateBcf(FlyEm::ZSegmentationBundle &bundle,
                          const std::string &setName);

  ZGraph* loadGalaSegmentation(const std::string filePath);
private:
  static std::map<std::string, BcfSet> createBcfSetNameMap();
  static std::map<std::string, BcfSet> m_bcfSetNameMap;
};
}

#endif // ZSEGMENTATIONANALYZER_H
