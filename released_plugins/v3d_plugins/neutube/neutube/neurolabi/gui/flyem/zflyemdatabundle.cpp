#include "zflyemdatabundle.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include "zjsonparser.h"
#include "zstring.h"
#include "tz_error.h"
#include "zgraph.h"
#include "c_json.h"
#include "zswctree.h"
#include "swctreenode.h"

using namespace std;

const char *ZFlyEmDataBundle::m_synapseKey = "synapse";
const char *ZFlyEmDataBundle::m_grayScaleKey = "gray_scale";
const char *ZFlyEmDataBundle::m_configKey = "config";
const char *ZFlyEmDataBundle::m_neuronColorKey = "neuron_color";
const char *ZFlyEmDataBundle::m_synapseScaleKey = "synapse_scale";
const char *ZFlyEmDataBundle::m_sourceOffsetKey = "source_offset";
const char *ZFlyEmDataBundle::m_sourceDimensionKey = "source_dimension";
const char *ZFlyEmDataBundle::m_imageResolutionKey = "image_resolution";
const char *ZFlyEmDataBundle::m_neuronKey = "neuron";
const char *ZFlyEmDataBundle::m_swcResolutionKey = "swc_resolution";
const char *ZFlyEmDataBundle::m_matchThresholdKey = "match_threshold";

const int ZFlyEmDataBundle::m_layerNumber = 10;
const double ZFlyEmDataBundle::m_layerRatio[11] = {
  0.0, 0.1, 0.2, 0.3, 0.35, 0.43, 0.54, 0.66, 0.73, 0.91, 1.0};

ZFlyEmDataBundle::ZFlyEmDataBundle() : m_synapseScale(10.0),
  m_synaseAnnotation(NULL), m_colorMap(NULL)
{
  for (int k = 0; k < 3; ++k) {
    m_swcResolution[k] = 1.0;
    m_imageResolution[k] = 1.0;
    m_sourceOffset[k] = 0;
    m_sourceDimension[k] = 0;
  }
}

ZFlyEmDataBundle::~ZFlyEmDataBundle()
{
  deprecate(ALL_COMPONENT);
}

bool ZFlyEmDataBundle::isDeprecated(EComponent comp) const
{
  switch (comp) {
  case SYNAPSE_ANNOTATION:
    return (m_synaseAnnotation == NULL);
  case COLOR_MAP:
    return (m_colorMap == NULL);
  default:
    break;
  }

  return false;
}

void ZFlyEmDataBundle::deprecate(EComponent comp)
{
  deprecateDependent(comp);

  switch (comp) {
  case SYNAPSE_ANNOTATION:
    delete m_synaseAnnotation;
    m_synaseAnnotation = NULL;
    break;
  case COLOR_MAP:
    delete m_colorMap;
    m_colorMap = NULL;
    break;
  case ALL_COMPONENT:
    deprecate(SYNAPSE_ANNOTATION);
    deprecate(COLOR_MAP);
    break;
  default:
    break;
  }
}

void ZFlyEmDataBundle::deprecateDependent(EComponent comp)
{
  switch (comp) {
  case SYNAPSE_ANNOTATION:
    break;
  default:
    break;
  }
}

bool ZFlyEmDataBundle::loadJsonFile(const std::string &filePath)
{
  m_neuronArray.clear();
  deprecate(ALL_COMPONENT);

  m_source = filePath;
  ZJsonObject bundleObject;
  if (bundleObject.load(filePath)) {
    m_synapseAnnotationFile = ZJsonParser::stringValue(bundleObject["synapse"]);
    m_grayScalePath = ZJsonParser::stringValue(bundleObject["gray_scale"]);
    //m_configFile = ZJsonParser::stringValue(bundleObject["config"]);
    m_neuronColorFile = ZJsonParser::stringValue(bundleObject["neuron_color"]);

    json_t *obj = bundleObject["synapse_scale"];
    if (obj != NULL) {
      m_synapseScale = ZJsonParser::numberValue(obj);
    }

    json_t *sourceOffsetObj = bundleObject["source_offset"];
    if (sourceOffsetObj != NULL) {
      if (ZJsonParser::isArray(sourceOffsetObj)) {
        for (size_t i = 0; i < 3; ++i) {
          m_sourceOffset[i] = ZJsonParser::integerValue(sourceOffsetObj, i);
        }
      }
    }

    json_t *sourceDimensionObj = bundleObject["source_dimension"];
    if (sourceDimensionObj != NULL) {
      if (ZJsonParser::isArray(sourceDimensionObj)) {
        for (size_t i = 0; i < 3; ++i) {
          m_sourceDimension[i] = ZJsonParser::integerValue(sourceDimensionObj, i);
        }
      }
    }

    json_t *imgResObj = bundleObject["image_resolution"];
    if (imgResObj != NULL) {
      if (ZJsonParser::isArray(imgResObj)) {
        for (size_t i = 0; i < 3; ++i) {
          m_imageResolution[i] = ZJsonParser::numberValue(imgResObj, i);
        }
      }
    }

    ZJsonArray neuronJsonArray;
    neuronJsonArray.set(bundleObject["neuron"], false);

    m_neuronArray.resize(neuronJsonArray.size());
    for (size_t i = 0; i < neuronJsonArray.size(); ++i) {
      ZJsonObject neuronJson(neuronJsonArray.at(i), false);
      m_neuronArray[i].loadJsonObject(neuronJson, getSource());
      m_neuronArray[i].setSynapseAnnotation(getSynapseAnnotation());
      m_neuronArray[i].setSynapseScale(m_synapseScale);
    }



    json_t *swcResObj = bundleObject["swc_resolution"];
    if (swcResObj != NULL) {
      TZ_ASSERT(ZJsonParser::isArray(swcResObj), "Array object expected.");
      for (size_t i = 0; i < 3; ++i) {
        m_swcResolution[i] =
            ZJsonParser::numberValue(bundleObject["swc_resolution"], i);
      }
      for (size_t i = 0; i < neuronJsonArray.size(); ++i) {
        m_neuronArray[i].setResolution(m_swcResolution);
      }
    }

    json_t *matchThreObj = bundleObject["match_threshold"];
    if (matchThreObj != NULL) {
      FILE *fp = fopen(ZJsonParser::stringValue(matchThreObj), "r");
      if (fp != NULL) {
        ZString line;
        while (line.readLine(fp)) {
          std::string str = line.firstQuotedWord();
          double thre = line.lastDouble();
          m_matchThreshold[str] = thre;
        }
        fclose(fp);
      }
    }
    m_source = filePath;

    updateNeuronConnection();

    return true;
  }

  return false;
}

string ZFlyEmDataBundle::toSummaryString() const
{
  ostringstream stream;
  stream << "Source: " << m_source << endl;
  stream << m_neuronArray.size() << " Neurons" << endl;
  stream << "Synapse: " << m_synapseAnnotationFile << endl;
  stream << "Gray scale: " << m_grayScalePath << endl;
  //stream << "Config: " << m_configFile << endl;
  stream << "SWC resolution: " << m_swcResolution[0] << " x "
         << m_swcResolution[1] << " x " << m_swcResolution[2] << endl;

  return stream.str();
}

int ZFlyEmDataBundle::countClass() const
{
  std::set<std::string> classSet;
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = getNeuronArray().begin();
       iter != getNeuronArray().end(); ++iter) {
    if (!iter->getClass().empty()) {
      classSet.insert(iter->getClass());
    }
  }

  return classSet.size();
}

int ZFlyEmDataBundle::countNeuronByClass(const string &className) const
{
  int count = 0;

  for (std::vector<ZFlyEmNeuron>::const_iterator iter = getNeuronArray().begin();
       iter != getNeuronArray().end(); ++iter) {
    if (className == "?") {
      if (iter->getClass().empty()) {
        ++count;
      }
    } else {
      if (iter->getClass() == className) {
        ++count;
      }
    }
  }

  return count;
}

string ZFlyEmDataBundle::toDetailString() const
{
  ostringstream stream;
  //stream << "Source: " << m_source << endl;
  //stream << m_neuronArray.size() << " neurons" << endl;
  stream << "Details: " << endl;
  stream << "  " << countClass() << " types" << endl;
  stream << "  " << countNeuronByClass("?") << " unknown types"
         << endl;
  if (getSynapseAnnotation() != NULL) {
    stream << "  " << getSynapseAnnotation()->getTBarNumber() << " TBars" << endl;
    stream << "  " << getSynapseAnnotation()->getPsdNumber() << " PSDs" << endl;
  }

  return stream.str();
}

void ZFlyEmDataBundle::print() const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    iter->print();
  }

  cout << "Synapse: " << m_synapseAnnotationFile << endl;
  cout << "Gray scale: " << m_grayScalePath << endl;
  //cout << "Config: " << m_configFile << endl;
}

string ZFlyEmDataBundle::getModelPath(int bodyId) const
{
  string modelPath;

  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getId() == bodyId) {
      modelPath = iter->getModelPath();
    }
  }

  return modelPath;
}

string ZFlyEmDataBundle::getName(int bodyId) const
{
  string name;

  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getId() == bodyId) {
      name = iter->getName();
    }
  }

  return name;
}

int ZFlyEmDataBundle::getIdFromName(const string &name) const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getName() == name) {
      return iter->getId();
    }
  }

  return -1;
}

bool ZFlyEmDataBundle::hasNeuronName(const string &name) const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getName() == name) {
      return true;
    }
  }

  return false;
}
const ZFlyEmNeuron* ZFlyEmDataBundle::getNeuron(int bodyId) const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getId() == bodyId) {
      return &(*iter);
    }
  }

  return NULL;
}

ZFlyEmNeuron* ZFlyEmDataBundle::getNeuron(int bodyId)
{
  return const_cast<ZFlyEmNeuron*>(
        static_cast<const ZFlyEmDataBundle&>(*this).getNeuron(bodyId));
}

const ZFlyEmNeuron* ZFlyEmDataBundle::getNeuronFromName(const string &name) const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getName() == name) {
      return &(*iter);
    }
  }

  return NULL;
}

ZSwcTree* ZFlyEmDataBundle::getModel(int bodyId) const
{
  const ZFlyEmNeuron *neuron = getNeuron(bodyId);
  if (neuron == NULL) {
    return NULL;
  }

  return neuron->getModel();
}

FlyEm::ZSynapseAnnotationArray* ZFlyEmDataBundle::getSynapseAnnotation() const
{
  if (isDeprecated(SYNAPSE_ANNOTATION)) {
    ZString path = m_synapseAnnotationFile;
    if (!path.empty()) {
      if (!path.isAbsolutePath()) {
        path = ZString::absolutePath(ZString(m_source).dirPath(), path);
      }

      if (fexist(path.c_str())) {
        m_synaseAnnotation = new FlyEm::ZSynapseAnnotationArray;
        m_synaseAnnotation->loadJson(path);
        m_synaseAnnotation->setResolution(m_imageResolution);
        m_synaseAnnotation->setSourceOffset(m_sourceOffset);
        m_synaseAnnotation->setSourceDimension(m_sourceDimension);
      }
    }
  }

  return m_synaseAnnotation;
}

map<int, QColor>* ZFlyEmDataBundle::getColorMap() const
{
  if (isDeprecated(COLOR_MAP)) {
    m_colorMap = new map<int, QColor>;
    ZString colorStr;
    FILE *fp = fopen(m_neuronColorFile.c_str(), "r");
    if (fp != NULL) {
      while (colorStr.readLine(fp)) {
        vector<int> value = colorStr.toIntegerArray();
        if (value.size() >= 4) {
          QColor color;
          color.setRed(value[1]);
          color.setGreen(value[2]);
          color.setBlue(value[3]);
          if (value.size() > 4) {
            color.setAlpha(value[4]);
          }
          (*m_colorMap)[value[0]] = color;
        }
      }
      fclose(fp);
    }
  }

  return m_colorMap;
}

void ZFlyEmDataBundle::updateNeuronConnection()
{
  for (std::vector<ZFlyEmNeuron>::iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    iter->clearConnection();
  }

  if (getSynapseAnnotation() != NULL) {
    ZGraph *graph = getSynapseAnnotation()->getConnectionGraph();
    for (int i = 0; i < graph->getEdgeNumber(); ++i) {
      int inputId = graph->getEdgeBegin(i);
      int outputId = graph->getEdgeEnd(i);
      double weight = graph->getEdgeWeight(i);
      ZFlyEmNeuron *inputNeuron = getNeuron(inputId);
      if (inputNeuron != NULL) {
        ZFlyEmNeuron *outputNeuron = getNeuron(outputId);
        if (outputNeuron != NULL) {
          inputNeuron->appendOutputNeuron(outputNeuron, weight);
          outputNeuron->appendInputNeuron(inputNeuron, weight);
        }
      }
    }
  }
}

double ZFlyEmDataBundle::getImageResolution(NeuTube::EAxis axis)
{
  switch (axis) {
  case NeuTube::X_AXIS:
    return m_imageResolution[0];
  case NeuTube::Y_AXIS:
    return m_imageResolution[1];
  case NeuTube::Z_AXIS:
    return m_imageResolution[2];
  }

  return 1.0;
}

double ZFlyEmDataBundle::getSwcResolution(NeuTube::EAxis axis)
{
  switch (axis) {
  case NeuTube::X_AXIS:
    return m_swcResolution[0];
  case NeuTube::Y_AXIS:
    return m_swcResolution[1];
  case NeuTube::Z_AXIS:
    return m_swcResolution[2];
  }

  return 1.0;
}

void ZFlyEmDataBundle::exportJsonFile(const string &path) const
{
  ZJsonObject jsonObj(C_Json::makeObject(), true);

  json_t *neuronArray = C_Json::makeArray();
  ZJsonArray neuronArrayWrapper(neuronArray, false);

  //Add each neuron
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    neuronArrayWrapper.append(iter->makeJsonObject());
  }

  jsonObj.setEntry(m_neuronKey, neuronArray);

  //Set meta data
  if (!m_synapseAnnotationFile.empty()) {
    jsonObj.setEntry(m_synapseKey, m_synapseAnnotationFile);
  }

  if (!m_grayScalePath.empty()) {
    jsonObj.setEntry(m_grayScaleKey, m_grayScalePath);
  }

  if (!m_neuronColorFile.empty()) {
    jsonObj.setEntry(m_neuronColorKey, m_neuronColorFile);
  }

  jsonObj.setEntry(m_synapseScaleKey, m_synapseScale);
  jsonObj.setEntry(m_sourceDimensionKey, m_sourceDimension, 3);
  jsonObj.setEntry(m_imageResolutionKey, m_imageResolution, 3);
  jsonObj.setEntry(m_swcResolutionKey, m_swcResolution, 3);

  jsonObj.dump(path);
}

double ZFlyEmDataBundle::getLayerStart(int layer)
{
  if (layer >= 1 && layer <= m_layerNumber) {
    return m_layerRatio[layer - 1] * m_sourceDimension[2] * m_imageResolution[2];
  }

  return 0.0;
}

double ZFlyEmDataBundle::getLayerEnd(int layer)
{
  if (layer >= 1 && layer <= m_layerNumber) {
    return m_layerRatio[layer] * m_sourceDimension[2] * m_imageResolution[2];
  }

  return 0.0;
}

bool ZFlyEmDataBundle::hitsLayer(
    int bodyId, int top, int bottom, bool isExclusive)
{
  ZFlyEmNeuron *neuron = getNeuron(bodyId);
  if (neuron != NULL) {
    return hitsLayer(*neuron, top, bottom, isExclusive);
  }

  return false;
}

bool ZFlyEmDataBundle::hitsLayer(
    const ZFlyEmNeuron &neuron, int top, int bottom, bool isExclusive)
{
  ZSwcTree *tree = neuron.getModel();
  if (tree != NULL) {
    if (top == 0 && bottom == m_layerNumber) {
      return true;
    }
    double topZ = getLayerStart(top);
    double bottomZ = getLayerEnd(bottom);
    const std::vector<Swc_Tree_Node*>& nodeArray = tree->getSwcTreeNodeArray();
    for (std::vector<Swc_Tree_Node*>::const_iterator iter = nodeArray.begin();
         iter != nodeArray.end(); ++iter) {
      if (SwcTreeNode::z(*iter) >= topZ && SwcTreeNode::z(*iter) <= bottomZ) {
        if (!isExclusive) {
          return true;
        }
      }
      if (isExclusive) {
        if (SwcTreeNode::z(*iter) < topZ || SwcTreeNode::z(*iter) > bottomZ) {
          return false;
        }
      }
    }
    if (isExclusive) {
      return true;
    }
  }

  return false;
}
