#include "zflyemneuron.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include "zjsonparser.h"
#include "zstring.h"
#include "zswctree.h"
#include "zobject3dscan.h"
#include "swctreenode.h"
#include "c_json.h"

using namespace std;

const int ZFlyEmNeuron::TopMatchCapacity = 10;

#define CONSTRUCTOR_INIT m_sourceId(0), m_id(0), m_synapseScale(10.0), m_model(NULL), \
  m_buddyModel(NULL), m_body(NULL), m_synapseAnnotation(NULL)

const char *ZFlyEmNeuron::m_idKey = "id";
const char *ZFlyEmNeuron::m_nameKey = "name";
const char *ZFlyEmNeuron::m_classKey = "class";
const char *ZFlyEmNeuron::m_modelKey = "model";
const char *ZFlyEmNeuron::m_volumeKey = "volume";

ZFlyEmNeuron::ZFlyEmNeuron() : CONSTRUCTOR_INIT
{
  for (int i = 0; i < 3; ++i) {
    m_resolution[i] = 1.0;
  }
}

ZFlyEmNeuron::ZFlyEmNeuron(const ZFlyEmNeuron &neuron) : CONSTRUCTOR_INIT
{
  m_sourceId = neuron.m_sourceId;
  m_id = neuron.m_id;
  m_name = neuron.m_name;
  m_class = neuron.m_class;
  m_modelPath = neuron.m_modelPath;
  for (int i = 0; i < 3; ++i) {
    m_resolution[i] = neuron.m_resolution[i];
  }
}

ZFlyEmNeuron::~ZFlyEmNeuron()
{
  deprecate(ALL_COMPONENT);
}

bool ZFlyEmNeuron::isDeprecated(EComponent comp) const
{
  switch (comp) {
  case MODEL:
    return (m_model == NULL);
  case BODY:
    return (m_body == NULL);
  case BUDDY_MODEL:
    return (m_buddyModel == NULL);
  default:
    break;
  }

  return false;
}

void ZFlyEmNeuron::deprecate(EComponent comp)
{
  deprecateDependent(comp);

  switch (comp) {
  case MODEL:
    delete m_model;
    m_model = NULL;
    break;
  case BODY:
    delete m_body;
    m_body = NULL;
    break;
  case BUDDY_MODEL:
    delete m_buddyModel;
    m_buddyModel = NULL;
    break;
  case ALL_COMPONENT:
    deprecate(MODEL);
    deprecate(BODY);
    deprecate(BUDDY_MODEL);
    break;
  default:
    break;
  }
}

void ZFlyEmNeuron::deprecateDependent(EComponent comp)
{
  switch (comp) {
  case MODEL:
    deprecate(BUDDY_MODEL);
    break;
  default:
    break;
  }
}

ZSwcTree* ZFlyEmNeuron::getResampleBuddyModel(double rs) const
{
  if (isDeprecated(BUDDY_MODEL)) {
    ZSwcTree *model = getModel();
    if (model != NULL) {
      m_buddyModel = model->clone();
      m_buddyModel->resample(rs);
      m_buddyModel->setLabel(0);
    }
  }

  return m_buddyModel;
}

ZSwcTree* ZFlyEmNeuron::getModel(const string &bundleSource) const
{
  if (isDeprecated(MODEL)) {
    if (!m_modelPath.empty()) {
      m_model = new ZSwcTree;
      ZString path(m_modelPath);
      if (!path.isAbsolutePath()) {
        path = path.absolutePath(ZString::dirPath(bundleSource));
      }
      m_model->load(path.c_str());
      if (m_model->data() == NULL) {
        delete m_model;
        m_model = NULL;
      }
    }

    if (m_model != NULL) {
      m_model->rescale(m_resolution[0], m_resolution[1], m_resolution[2]);
    }
  }

  return m_model;
}

ZObject3dScan* ZFlyEmNeuron::getBody() const
{
  if (isDeprecated(BODY)) {
    if (!m_volumePath.empty()) {
      m_body = new ZObject3dScan;
      m_body->load(m_volumePath);
    }
  }

  return m_body;
}


void ZFlyEmNeuron::setResolution(const double *res)
{
  if (m_resolution[0] != res[0] || m_resolution[1] != res[1] ||
      m_resolution[2] != res[2]) {
    for (int i = 0; i < 3; ++i) {
      m_resolution[i] = res[i];
    }
    deprecate(MODEL);
  }
}

string ZFlyEmNeuron::getAbsolutePath(const ZString &path, const string &source)
{
  if (!path.isAbsolutePath()) {
    return path.absolutePath(ZString::dirPath(source));
  }

  return path;
}

void ZFlyEmNeuron::loadJsonObject(ZJsonObject &obj, const string &source)
{
  m_id = ZJsonParser::integerValue(obj["id"]);
  m_name = ZJsonParser::stringValue(obj["name"]);
  m_class = ZJsonParser::stringValue(obj["class"]);

  m_modelPath = ZJsonParser::stringValue(obj["model"]);
  if (!m_modelPath.empty()) {
    m_modelPath = getAbsolutePath(m_modelPath, source);
  }

  m_volumePath = ZJsonParser::stringValue(obj["volume"]);
  if (!m_volumePath.empty()) {
    m_volumePath = getAbsolutePath(m_volumePath, source);
  }

  deprecate(ALL_COMPONENT);
}

json_t* ZFlyEmNeuron::makeJsonObject() const
{
  json_t *obj = C_Json::makeObject();

  ZJsonObject objWrapper(obj, false);

  objWrapper.setEntry(m_idKey, m_id);
  if (!m_name.empty()) {
    objWrapper.setEntry(m_nameKey, m_name);
  }

  if (!m_class.empty()) {
    objWrapper.setEntry(m_classKey, m_class);
  }

  if (!m_modelPath.empty()) {
    objWrapper.setEntry(m_modelKey, m_modelPath);
  }

  if (!m_volumePath.empty()) {
    objWrapper.setEntry(m_volumeKey, m_volumePath);
  }

  return obj;
}

void ZFlyEmNeuron::print() const
{
  print(cout);
}

void ZFlyEmNeuron::print(ostream &stream) const
{
  stream << "Neuron: " << endl;
  stream << "  Id: " << m_id << endl;
  stream << "  Name: " << m_name << endl;
  stream << "  Class: " << m_class << endl;
  stream << "  Model: " << m_modelPath << endl;
}

void ZFlyEmNeuron::setId(const string &str)
{
  setId(atoi(str.c_str()));
}

void ZFlyEmNeuron::printJson(ostream &stream, int indent) const
{
  stream << setfill(' ');
  stream << setw(indent) << "";
  stream << "{" << endl;
  stream << setw(indent) << "";
  stream << "  \"id\": " << m_id << "," << endl;
  stream << setw(indent) << "";
  stream << "  \"name\": \"" << m_name << "\"," << endl;
  stream << setw(indent) << "";
  stream << "  \"class\": \"" << m_class << "\"," << endl;
  stream << setw(indent) << "";
  stream << "  \"model\": \"" << m_modelPath << "\"" << endl;
  stream << setw(indent) << "";
  stream << '}' << endl;
}

bool ZFlyEmNeuron::hasSimilarName(const string &name) const
{
  ZString newName = m_name;

  for (ZString::iterator iter = newName.begin(); iter != newName.end();
       ++iter) {
    if ((!tz_isletter(*iter)) && (!isdigit(*iter))) {
      *iter = '_';
    }
  }

  ZString queryName = name;
  for (ZString::iterator iter = queryName.begin(); iter != queryName.end();
       ++iter) {
    if ((!tz_isletter(*iter)) && (!isdigit(*iter))) {
      *iter = '_';
    }
  }

#ifdef _DEBUG_2
  cout << newName << endl;
#endif

  return newName.contains(queryName.c_str());
}

std::vector<ZPunctum*> ZFlyEmNeuron::getSynapse() const
{
  /*
  std::vector<ZPunctum*> synapse;
  if (m_synapseAnnotation != NULL) {
    FlyEm::SynapseAnnotationConfig config;
    config.startNumber = m_synapseAnnotation->getMetaData().getSourceZOffset();
    config.height = m_synapseAnnotation->getMetaData().getSourceYDim();
    config.xResolution = m_synapseAnnotation->getMetaData().getXResolution();
    config.yResolution = m_synapseAnnotation->getMetaData().getYResolution();
    config.zResolution = m_synapseAnnotation->getMetaData().getZResolution();
    config.swcDownsample1 = 1;
    config.swcDownsample2 = 1;
    config.sizeScale = m_synapseScale;

    FlyEm::SynapseDisplayConfig displayConfig;
    displayConfig.mode = FlyEm::SynapseDisplayConfig::HALF_SYNAPSE;
    displayConfig.tBarColor.red = 255;
    displayConfig.tBarColor.green = 255;
    displayConfig.tBarColor.blue = 0;
    displayConfig.psdColor.red = 0;
    displayConfig.psdColor.green = 0;
    displayConfig.psdColor.blue = 0;
    displayConfig.bodyId = getId();

    return m_synapseAnnotation->toPuncta(
          config, FlyEm::SynapseLocation::PHYSICAL_SPACE, displayConfig);
  }

  return synapse;
  */

  return getSynapse(-1);
}

std::vector<ZPunctum*> ZFlyEmNeuron::getSynapse(
    int buddyBodyId) const
{
  std::vector<ZPunctum*> synapse;
  if (m_synapseAnnotation != NULL) {
    FlyEm::SynapseAnnotationConfig config;
    config.startNumber = m_synapseAnnotation->getMetaData().getSourceZOffset();
    config.height = m_synapseAnnotation->getMetaData().getSourceYDim();
    config.xResolution = m_synapseAnnotation->getMetaData().getXResolution();
    config.yResolution = m_synapseAnnotation->getMetaData().getYResolution();
    config.zResolution = m_synapseAnnotation->getMetaData().getZResolution();
    config.swcDownsample1 = 1;
    config.swcDownsample2 = 1;
    config.sizeScale = m_synapseScale;

    FlyEm::SynapseDisplayConfig displayConfig;
    displayConfig.mode = FlyEm::SynapseDisplayConfig::HALF_SYNAPSE;
    displayConfig.tBarColor.red = 255;
    displayConfig.tBarColor.green = 255;
    displayConfig.tBarColor.blue = 0;
    displayConfig.psdColor.red = 0;
    displayConfig.psdColor.green = 0;
    displayConfig.psdColor.blue = 0;
    displayConfig.bodyId = getId();
    displayConfig.buddyBodyId = buddyBodyId;

    return m_synapseAnnotation->toPuncta(
          config, FlyEm::SynapseLocation::PHYSICAL_SPACE, displayConfig);
  }

  return synapse;
}

int ZFlyEmNeuron::getTBarNumber() const
{
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->countTBar(getId());
  }

  return 0;
}

int ZFlyEmNeuron::getPsdNumber() const
{
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->countPsd(getId());
  }

  return 0;
}

int ZFlyEmNeuron::getInputNeuronNumber() const
{
  /*
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->countInputNeuron(getId());
  }

  return 0;
  */

  return m_input.size();
}

int ZFlyEmNeuron::getOutputNeuronNumber() const
{
  /*
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->countOutputNeuron(getId());
  }

  return 0;
  */

  return m_output.size();
}

const ZFlyEmNeuron *ZFlyEmNeuron::getStrongestInput() const
{
  if (m_input.empty()) {
    return NULL;
  }

  size_t maxIndex = 0;
  double maxWeight = m_inputWeight[0];

  for (size_t i = 1; i < m_input.size(); ++i) {
    if (maxWeight < m_inputWeight[i]) {
      maxIndex = i;
      maxWeight = m_inputWeight[i];
    }
  }

  return m_input[maxIndex];
/*
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->getStrongestInput(getId());
  }
  return -1;

  return NULL;
  */
}

const ZFlyEmNeuron* ZFlyEmNeuron::getStrongestOutput() const
{
  if (m_output.empty()) {
    return NULL;
  }

  size_t maxIndex = 0;
  double maxWeight = m_outputWeight[0];

  for (size_t i = 1; i < m_output.size(); ++i) {
    if (maxWeight < m_outputWeight[i]) {
      maxIndex = i;
      maxWeight = m_outputWeight[i];
    }
  }

  return m_output[maxIndex];
  /*
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->getStrongestOutput(getId());
  }
  return -1;

  return NULL;
  */
}

void ZFlyEmNeuron::clearConnection()
{
  m_input.clear();
  m_output.clear();
  m_inputWeight.clear();
  m_outputWeight.clear();
}

const ZFlyEmNeuron* ZFlyEmNeuron::getInputNeuron(size_t index) const
{
  if (index >= m_input.size()) {
    return NULL;
  }

  return m_input[index];
}

const ZFlyEmNeuron* ZFlyEmNeuron::getOutputNeuron(size_t index) const
{
  if (index >= m_output.size()) {
    return NULL;
  }

  return m_output[index];
}

string ZFlyEmNeuron::toString() const
{
  ostringstream stream;
  stream << m_id << " (" << m_class << ") : " << m_modelPath;

  return stream.str();
}

double ZFlyEmNeuron::getBodyVolume() const
{
  return getBody()->getVoxelNumber()/* * m_resolution[0] * m_resolution[1] *
      m_resolution[2]*/;
#if 0
  if (m_volumePath.empty()) {
    return 0.0;
  }

  ZObject3dScan obj;
  if (obj.load(m_volumePath)) {
    return obj.getVoxelNumber() * m_resolution[0] * m_resolution[1] *
        m_resolution[2];
  }

  return 0.0;
#endif
}

ZFlyEmNeuronAxis ZFlyEmNeuron::getAxis() const
{
  ZSwcTree *tree = getModel();

  ZFlyEmNeuronAxis axis;

  if (tree != NULL) {
    double dz = m_resolution[2] * 5.0;

    //Get all nodes sorted by z
    const std::vector<Swc_Tree_Node *> &nodeArray = tree->getSwcTreeNodeArray(
        ZSwcTree::Z_SORT_ITERATOR);

    //Build axis
    ZPoint center;
    double currentZ = SwcTreeNode::z(nodeArray[0]);
    double weight = 0.0;
    for (size_t i = 0; i < nodeArray.size(); ++i) {
      double z = SwcTreeNode::z(nodeArray[i]);
      if (z >= currentZ - dz && z < currentZ + dz) {
        double decay = (z - currentZ) / dz;
        double v = SwcTreeNode::radius(nodeArray[i]) *
            SwcTreeNode::radius(nodeArray[i]) *
            SwcTreeNode::radius(nodeArray[i]) * exp(-(decay * decay));
        center += SwcTreeNode::pos(nodeArray[i]) * v;
        weight += v;
      } else {
        if (weight > 0.0) {
          center /= weight;
          center.setZ(currentZ);
          axis.setCenter(center);
        }
        center.set(0, 0, 0);
        weight = 0.0;
        currentZ += dz * 2.0;
      }
    }
  }

  return axis;
}

ZFlyEmNeuronRange ZFlyEmNeuron::getRange(double xyRes, double zRes) const
{
  ZFlyEmNeuronRange range;

  ZObject3dScan *obj = getBody();


  int minZ = obj->getMinZ();
  int maxZ = obj->getMaxZ();

  for (int z = minZ; z <= maxZ; ++z) {
    ZObject3dScan slice = obj->getSlice(z);

    if (!slice.isEmpty()) {
      ZPoint center = slice.getCentroid();
      double maxDist = 0.0;
      for (size_t i = 0; i < slice.getStripeNumber(); ++i) {
        int nseg = slice.getStripe(i).getSegmentNumber();
        int y = slice.getStripe(i).getY();
        for (int j = 0; j < nseg; ++j) {
          int x1 = slice.getStripe(i).getSegmentStart(j);
          int x2 = slice.getStripe(i).getSegmentEnd(j);
          double dist1 = center.distanceTo(x1, y, z);
          double dist2 = center.distanceTo(x2, y, z);
          maxDist = max(dist1, dist2);
        }
      }

      range.setPlaneRange(z * zRes, maxDist  * xyRes);
    }
  }

  return range;
}
