#ifndef ZFLYEMQUALITYANALYZER_H
#define ZFLYEMQUALITYANALYZER_H

#include "flyem/zintcuboidarray.h"
#include "zobject3dscan.h"
#include "flyem/zflyemneuronrange.h"
#include "flyem/zflyemneuron.h"

class ZFlyEmQualityAnalyzer
{
public:
  ZFlyEmQualityAnalyzer();

  bool isExitingOrphanBody(const ZObject3dScan &obj);

  bool isStitchedOrphanBody(const ZObject3dScan &obj);

  bool isOrphanBody(const ZObject3dScan &obj);

  void setSubstackRegion(const FlyEm::ZIntCuboidArray &roi);

  /*!
   * \brief Label SWC node that is out of range.
   *
   * Labels any node in the model of \a neuron with the value \a label if the
   * node is out of the range.
   */
  static void labelSwcNodeOutOfRange(const ZFlyEmNeuron &neuron,
                                     const ZFlyEmNeuronRange &range, int label);

private:
  FlyEm::ZIntCuboidArray m_substackRegion;

};

#endif // ZFLYEMQUALITYANALYZER_H
