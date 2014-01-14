#include "zflyemneuronfilterfactory.h"
#include "zflyemneuronfilter.h"
#include "zjsonparser.h"

ZFlyEmNeuronFilterFactory::ZFlyEmNeuronFilterFactory()
{
}

ZFlyEmNeuronFilterFactory::~ZFlyEmNeuronFilterFactory()
{
  for (std::vector<ZFlyEmNeuronFilter*>::iterator iter = m_filterArray.begin();
       iter != m_filterArray.end(); ++iter) {
    delete *iter;
  }
}

void ZFlyEmNeuronFilterFactory::registerFilter(ZFlyEmNeuronFilter *filter)
{
  m_filterArray.push_back(filter);
}

ZFlyEmNeuronFilter* ZFlyEmNeuronFilterFactory::createFilter(EFilterType type)
{
  ZFlyEmNeuronFilter *filter = NULL;

  switch (type) {
  case COMPOSITE:
    filter = new ZFlyEmNeuronCompositeFilter;
    break;
  case LAYER:
    filter = new ZFlyEmNeuronLayerFilter;
    break;
  case TIP_DISTANCE:
    filter = new ZFlyEmNeuronTipDistanceFilter;
    break;
  case TIP_ANGLE:
    filter = new ZFlyEmNeuronTipAngleFilter;
    break;
  case DEEP_ANGLE:
    filter = new ZFlyEmNeuronDeepAngleFilter;
    break;
  default:
    break;
  }

  if (filter != NULL) {
    registerFilter(filter);
  }

  return filter;
}

ZFlyEmNeuronFilterFactory::EFilterType ZFlyEmNeuronFilterFactory::getType(
    const std::string &typeString)
{
  if (typeString == "composite") {
    return COMPOSITE;
  } else if (typeString == "layer") {
    return LAYER;
  } else if (typeString == "tip_distance") {
    return TIP_DISTANCE;
  } else if (typeString == "tip_angle") {
    return TIP_ANGLE;
  } else if (typeString == "deep_angle") {
    return DEEP_ANGLE;
  }

  return UNKNOWN_FILTER;
}

ZFlyEmNeuronFilter* ZFlyEmNeuronFilterFactory::createFilter(ZJsonObject &jsonObj)
{
  EFilterType type = getType(ZJsonParser::stringValue(jsonObj["type"]));

  ZFlyEmNeuronFilter *filter = createFilter(type);
  ZJsonValue config(jsonObj["config"], false);

  if (type == COMPOSITE) {
    ZFlyEmNeuronCompositeFilter *compositeFilter =
        dynamic_cast<ZFlyEmNeuronCompositeFilter*>(filter);
    if (config.isArray()) {
      std::vector<ZJsonValue> filterArrayObj = config.toArray();
      for (std::vector<ZJsonValue>::iterator iter = filterArrayObj.begin();
           iter != filterArrayObj.end(); ++iter) {
        ZJsonObject obj(iter->getValue(), false);
        if (!obj.isEmpty()) {
          ZFlyEmNeuronFilter *subFilter = createFilter(obj);
          compositeFilter->appendFilter(subFilter);
        }
      }
    }
  } else {
    filter->configure(config);
  }

  return filter;
}
