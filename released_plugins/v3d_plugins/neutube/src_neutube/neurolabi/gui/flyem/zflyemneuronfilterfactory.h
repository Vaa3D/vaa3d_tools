#ifndef ZFLYEMNEURONFILTERFACTORY_H
#define ZFLYEMNEURONFILTERFACTORY_H

#include <vector>
#include "zjsonobject.h"

class ZFlyEmNeuronFilter;

/*!
 * \brief The factory class of creating flyem neuron filters.
 */
class ZFlyEmNeuronFilterFactory
{
public:
  ZFlyEmNeuronFilterFactory();
  ~ZFlyEmNeuronFilterFactory();

  enum EFilterType {
    COMPOSITE, LAYER, TIP_DISTANCE, TIP_ANGLE, DEEP_ANGLE, UNKNOWN_FILTER
  };

  ZFlyEmNeuronFilter* createFilter(EFilterType type);
  ZFlyEmNeuronFilter* createFilter(ZJsonObject &jsonObj);

private:
  void registerFilter(ZFlyEmNeuronFilter *filter);
  EFilterType getType(const std::string &typeString);

private:
  std::vector<ZFlyEmNeuronFilter*> m_filterArray;
};

#endif // ZFLYEMNEURONFILTERFACTORY_H
