#include "zflyemneuronfilter.h"
#include <iomanip>
#include "zflyemneuron.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "tz_error.h"
#include "swc/zswcterminalsurfacemetric.h"
#include "swc/zswcterminalanglemetric.h"

ZFlyEmNeuronFilter::ZFlyEmNeuronFilter() : m_refNeuron(NULL)
{
}

ZFlyEmNeuronFilter::~ZFlyEmNeuronFilter()
{

}

void ZFlyEmNeuronFilter::setReference(const ZFlyEmNeuron *neuron)
{
  m_refNeuron = neuron;
}

void ZFlyEmNeuronFilter::appendFilter(ZFlyEmNeuronFilter */*filter*/)
{

}


std::vector<ZFlyEmNeuron*> ZFlyEmNeuronFilter::filter(
    const std::vector<ZFlyEmNeuron *> input) const
{
  std::vector<ZFlyEmNeuron*> filtered;
  for (std::vector<ZFlyEmNeuron*>::const_iterator iter = input.begin();
       iter != input.end(); ++iter) {
    if (isPassed(*(*iter))) {
      filtered.push_back(*iter);
    }
  }

  return filtered;
}

ZFlyEmNeuronCompositeFilter::ZFlyEmNeuronCompositeFilter()
{

}


void ZFlyEmNeuronCompositeFilter::appendFilter(ZFlyEmNeuronFilter *filter)
{
  if (filter != NULL) {
    m_filterArray.push_back(filter);
  }
}

bool ZFlyEmNeuronCompositeFilter::isPassed(const ZFlyEmNeuron &neuron) const
{
  for (std::vector<ZFlyEmNeuronFilter*>::const_iterator iter = m_filterArray.begin();
       iter != m_filterArray.end(); ++iter) {
    if (!(*iter)->isPassed(neuron)) {
      return false;
    }
  }

  return true;
}

void ZFlyEmNeuronCompositeFilter::setReference(const ZFlyEmNeuron *neuron)
{
  for (std::vector<ZFlyEmNeuronFilter*>::const_iterator iter = m_filterArray.begin();
       iter != m_filterArray.end(); ++iter) {
    (*iter)->setReference(neuron);
  }
}

bool ZFlyEmNeuronCompositeFilter::configure(ZJsonValue &/*config*/)
{
  return false;
}

void ZFlyEmNeuronCompositeFilter::print(int indent) const
{
  std::cout << std::setfill(' ') << std::setw(indent) << "";
  std::cout << "Composite filter:" << std::endl;
  if (m_filterArray.empty()) {
    std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
    std::cout << "No component." << std::endl;
  }

  for (std::vector<ZFlyEmNeuronFilter*>::const_iterator
       iter = m_filterArray.begin(); iter != m_filterArray.end(); ++iter) {
    (*iter)->print(indent + 2);
  }
}
/////////ZLayerFlyEmNeuronFilter/////////////////

const int ZFlyEmNeuronLayerFilter::m_layerNumber = 10;
double ZFlyEmNeuronLayerFilter::m_layerRatio[] = {
  0.0, 0.1, 0.2, 0.3, 0.35, 0.43, 0.54, 0.66, 0.73, 0.91, 1.0};

ZFlyEmNeuronLayerFilter::ZFlyEmNeuronLayerFilter() :
  m_top(0), m_bottom(m_layerNumber), m_isExclusive(false)
{
  m_layerStart = 0;
  m_layerLength = 60000;
}

double ZFlyEmNeuronLayerFilter::getLayerStart(int layer) const
{
  if (layer >= 1 && layer <= m_layerNumber) {
    return m_layerRatio[layer - 1] * m_layerLength + m_layerStart;
  }

  return 0.0;
}

double ZFlyEmNeuronLayerFilter::getLayerEnd(int layer) const
{
  if (layer >= 1 && layer <= m_layerNumber) {
    return  m_layerRatio[layer] * m_layerLength + m_layerStart;
  }

  return 0.0;
}

bool ZFlyEmNeuronLayerFilter::isPassed(const ZFlyEmNeuron &neuron) const
{
  ZSwcTree *tree = neuron.getModel();
  if (tree != NULL) {
    if (m_top == 0 && m_bottom == m_layerNumber) {
      return true;
    }

    double topZ = getLayerStart(m_top);
    double bottomZ = getLayerEnd(m_bottom);
    const std::vector<Swc_Tree_Node*>& nodeArray = tree->getSwcTreeNodeArray();

    if (!nodeArray.empty()) {
      for (std::vector<Swc_Tree_Node*>::const_iterator
           iter = REGULAR_SWC_NODE_BEGIN(nodeArray[0], nodeArray.begin());
           iter != nodeArray.end(); ++iter) {
        TZ_ASSERT(SwcTreeNode::isRegular(*iter), "Unexpected virtual node");
        if (SwcTreeNode::z(*iter) >= topZ && SwcTreeNode::z(*iter) <= bottomZ) {
          if (!m_isExclusive) {
            return true;
          }
        }
        if (m_isExclusive) {
          if (SwcTreeNode::z(*iter) < topZ || SwcTreeNode::z(*iter) > bottomZ) {
            return false;
          }
        }
      }
      if (m_isExclusive) {
        return true;
      }
    }
  }

  return false;
}

bool ZFlyEmNeuronLayerFilter::configure(ZJsonValue &config)
{
  ZJsonObject obj(config.getValue(), false);
  if (!obj.isEmpty()) {
    if (obj.hasKey("start")) {
      m_layerStart = ZJsonParser::numberValue(obj["start"]);
    }

    if (obj.hasKey("length")) {
      m_layerLength = ZJsonParser::numberValue(obj["length"]);
    }

    if (obj.hasKey("top")) {
      m_top = ZJsonParser::integerValue(obj["top"]);
    }

    if (obj.hasKey("bottom")) {
      m_bottom = ZJsonParser::integerValue(obj["bottom"]);
    }

    if (obj.hasKey("exclusive")) {
      m_isExclusive = ZJsonParser::booleanValue(obj["exclusive"]);
    }

    return true;
  }

  return false;
}

void ZFlyEmNeuronLayerFilter::print(int indent) const
{
  std::cout << std::setfill(' ') << std::setw(indent) << "";
  std::cout << "Layer filter:" << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Layer start: " << m_layerStart << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Layer lenght: " << m_layerLength << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Top layer: " << m_top << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Bottom layer: " << m_bottom << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Exclusive: " << m_isExclusive << std::endl;
  if (m_refNeuron == NULL) {
    std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
    std::cout << "No reference neuron" << std::endl;
  }
}

///////////////////////////
ZFlyEmNeuronTipDistanceFilter::ZFlyEmNeuronTipDistanceFilter() :
  m_minDist(100.0)
{
}

bool ZFlyEmNeuronTipDistanceFilter::isPassed(const ZFlyEmNeuron &neuron) const
{
  ZSwcTree *refTree = NULL;
  if (m_refNeuron != NULL) {
    refTree = m_refNeuron->getModel();
  }

  if (refTree != NULL) {
    ZSwcTree *tree = neuron.getModel();
    if (tree != NULL) {
      ZSwcTerminalSurfaceMetric metric;
      double dist = metric.measureDistance(tree, refTree);

      if (dist <= m_minDist){
        return true;
      }
    }
  }

  return false;
}

bool ZFlyEmNeuronTipDistanceFilter::configure(ZJsonValue &config)
{
  ZJsonObject obj(config.getValue(), false);
  if (!obj.isEmpty()) {
    if (obj.hasKey("min_dist")) {
      m_minDist = ZJsonParser::numberValue(obj["min_dist"]);
    }
    return true;
  }

  return false;
}

void ZFlyEmNeuronTipDistanceFilter::print(int indent) const
{
  std::cout << std::setfill(' ') << std::setw(indent) << "";
  std::cout << "Tip distance filter:" << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Min dist: " << m_minDist << std::endl;
  if (m_refNeuron == NULL) {
    std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
    std::cout << "No reference neuron" << std::endl;
  }
}

///////////////ZFlyEmNeuronTipAngleFilter/////////////////////
ZFlyEmNeuronTipAngleFilter::ZFlyEmNeuronTipAngleFilter() : m_minDist(1.0)
{
}

bool ZFlyEmNeuronTipAngleFilter::isPassed(const ZFlyEmNeuron &neuron) const
{
  ZSwcTree *refTree = NULL;
  if (m_refNeuron != NULL) {
    refTree = m_refNeuron->getModel();
  }

  if (refTree != NULL) {
    ZSwcTree *tree = neuron.getModel();
    if (tree != NULL) {
      ZSwcTerminalAngleMetric metric;
      double dist = metric.measureDistance(tree, refTree);

      if (dist <= m_minDist){
        return true;
      }
    }
  }

  return false;
}

bool ZFlyEmNeuronTipAngleFilter::configure(ZJsonValue &config)
{
  ZJsonObject obj(config.getValue(), false);
  if (!obj.isEmpty()) {
    if (obj.hasKey("min_dist")) {
      m_minDist = ZJsonParser::numberValue(obj["min_dist"]);
    }
    return true;
  }

  return false;
}

void ZFlyEmNeuronTipAngleFilter::print(int indent) const
{
  std::cout << std::setfill(' ') << std::setw(indent) << "";
  std::cout << "Tip angle filter:" << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Min dist: " << m_minDist << std::endl;
  if (m_refNeuron == NULL) {
    std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
    std::cout << "No reference neuron" << std::endl;
  }
}

///////////////ZFlyEmNeuronDeepAngleFilter/////////////////////
ZFlyEmNeuronDeepAngleFilter::ZFlyEmNeuronDeepAngleFilter() : m_minDist(1.0)
{
}

bool ZFlyEmNeuronDeepAngleFilter::isPassed(const ZFlyEmNeuron &neuron) const
{
  ZSwcTree *refTree = NULL;
  if (m_refNeuron != NULL) {
    refTree = m_refNeuron->getModel();
  }

  if (refTree != NULL) {
    ZSwcTree *tree = neuron.getModel();
    if (tree != NULL) {
      double dist = m_metric.measureDistance(tree, refTree);

      if (dist <= m_minDist){
        return true;
      }
    }
  }

  return false;
}

bool ZFlyEmNeuronDeepAngleFilter::configure(ZJsonValue &config)
{
  ZJsonObject obj(config.getValue(), false);
  if (!obj.isEmpty()) {
    if (obj.hasKey("min_dist")) {
      m_minDist = ZJsonParser::numberValue(obj["min_dist"]);
    }
    if (obj.hasKey("deep_dist")) {
      m_metric.setMinDist(ZJsonParser::numberValue(obj["deep_dist"]));
    }
    if (obj.hasKey("deep_level")) {
      m_metric.setLevel(ZJsonParser::integerValue(obj["deep_level"]));
    }

    return true;
  }

  return false;
}

void ZFlyEmNeuronDeepAngleFilter::print(int indent) const
{
  std::cout << std::setfill(' ') << std::setw(indent) << "";
  std::cout << "Deep angle filter:" << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Min dist: " << m_minDist << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Deep level: " << m_metric.getLevel() << std::endl;
  std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
  std::cout << "Deep dist: " << m_metric.getMinDist() << std::endl;
  if (m_refNeuron == NULL) {
    std::cout << std::setfill(' ') << std::setw(indent + 2) << "";
    std::cout << "No reference neuron" << std::endl;
  }
}
