#include "zsynapseannotation.h"

#include <sstream>
#include <string>
#include <string.h>

#include "../zjsonparser.h"
#include "zsynapselocationmatcher.h"
#include "zsynapselocationmetric.h"

using namespace FlyEm;
using namespace std;

SynapseLocation::SynapseLocation(EType type)
{
  m_type = type;
  m_x = 0;
  m_y = 0;
  m_z = 0;
  m_bodyId = -1;
  m_confidence = 0.0;
  m_multi = false;
}

SynapseLocation::SynapseLocation(EType type, int x, int y, int z, int bodyId,
                                 double confidence, const std::string &status,
                                 bool multi)
{
  set(type, x, y, z, bodyId, confidence, status, multi);
}

void SynapseLocation::set(EType type, int x, int y, int z, int bodyId,
                          double confidence, const std::string &status,
                          bool multi)
{
  m_type = type;
  m_x = x;
  m_y = y;
  m_z = z;
  m_bodyId = bodyId;
  m_confidence = confidence;
  m_status = status;
  m_multi = multi;
}

void SynapseLocation::loadJsonObject(json_t *object, EType type)
{
  const char *key;
  json_t *value;

  m_type = type;

  json_object_foreach(object, key, value) {
    if (strcmp(key, "status") == 0) {
      m_status = json_string_value(value);
    } else if (strcmp(key, "confidence") == 0) {
      m_confidence = json_number_value(value);
    } else if (strcmp(key, "body ID") == 0) {
      m_bodyId = json_integer_value(value);
    } else if (strcmp(key, "location") == 0) {
      m_x = json_integer_value(json_array_get(value, 0));
      m_y = json_integer_value(json_array_get(value, 1));
      m_z = json_integer_value(json_array_get(value, 2));
    } else if (strcmp(key, "uid") == 0) {
      m_uid = json_string_value(value);
    } else if (strcmp(key, "multi") == 0) {
      if (strcmp(json_string_value(value), "multi") == 0) {
        m_multi = true;
      } else {
        m_multi = false;
      }
    }
  }
}

string SynapseLocation::toString(int indent) const
{
  ostringstream stream;
  string indentStr(indent, ' ');
  if (indent == 0) {
    indentStr.clear();
  }

  if (isTBar()) {
    stream << indentStr << "\"status\": " << "\"" << m_status << "\""
           << ',' << endl;
  }

  stream << indentStr << "\"confidence\": " << m_confidence << ',' << endl;
  stream << indentStr << "\"body ID\": " << m_bodyId << ',' << endl;
  stream << indentStr <<
            "\"location\": [" << m_x << ", " << m_y << ", " << m_z << ']';
  if (!m_uid.empty()) {
    stream << ',' << endl << indentStr << "\"uid\": " << m_uid;
  }

  if (!m_swcLocation.swcFilePath.empty() && m_swcLocation.bodyId >= 0) {
    stream << ',' << endl << indentStr << "\"segment\": [" << '\"' <<
              m_swcLocation.swcFilePath << "\", " << m_swcLocation.bodyId <<
              ", " << m_swcLocation.nodeId << ", " << m_swcLocation.distance <<
              "]";
  }

  return stream.str();
}

void SynapseLocation::print() const
{
  cout << toString(0) << endl;
}

ZPoint SynapseLocation::mapPosition(const SynapseAnnotationConfig &config,
                                    ELocationSpace spaceOption) const
{
  ZPoint point(m_x, m_y, m_z);

  switch (spaceOption) {
  case IMAGE_SPACE:
    point.setY(config.height - 1 - point.y());
    point.setZ(point.z() - config.startNumber);
    break;
  case ORIGINAL_SWC_SPACE:
    point = mapPosition(config, IMAGE_SPACE);
    point.setX(point.x() / config.swcDownsample1);
    point.setY(point.y() / config.swcDownsample1);
    point -= config.offset;
    point.setX(point.x() / config.swcDownsample2);
    point.setY(point.y() / config.swcDownsample2);
    break;
  case ADJUSTED_SWC_SPACE:
    point = mapPosition(config, IMAGE_SPACE);
    point.setX(point.x() / config.swcDownsample1 / config.swcDownsample2);
    point.setY(point.y() / config.swcDownsample1 / config.swcDownsample2);
    point.setZ(point.z() * config.zResolution / config.xResolution /
               config.swcDownsample1 / config.swcDownsample2);
    break;
  case PHYSICAL_SPACE:
    point = mapPosition(config, IMAGE_SPACE);
    point.setX(point.x() * config.xResolution);
    point.setY(point.y() * config.yResolution);
    point.setZ(point.z() * config.zResolution);
    break;
  default:
    break;
  }

  return point;
}

Swc_Tree_Node* SynapseLocation::findSwcNode(Swc_Tree *tree,
                           const SynapseAnnotationConfig &config,
                           ELocationSpace spaceOption)
{
  ZPoint point = mapPosition(config, spaceOption);
  double pos[3];
  point.toArray(pos);

  return Swc_Tree_Closest_Node(tree, pos);
}

void SynapseLocation::setSwcLocation(const string &filePath, int bodyId,
                                     int nodeId, double distance)
{
  m_swcLocation.swcFilePath = filePath;
  m_swcLocation.bodyId = bodyId;
  m_swcLocation.nodeId = nodeId;
  m_swcLocation.distance = distance;
}

void SynapseLocation::convertRavelerToImageSpace(int startZ, int height)
{
  m_z -= startZ;
  m_y = height - 1 - m_y;
}

ZSynapseAnnotation::ZSynapseAnnotation() : m_tBar(SynapseLocation::TBAR)
{
}

void
ZSynapseAnnotation::setTBar(int x, int y, int z, int bodyId, double confidence,
                            std::string status, bool multi)
{
  m_tBar.set(SynapseLocation::TBAR, x, y, z, bodyId, confidence, status, multi);
}

void ZSynapseAnnotation::addPartner(int x, int y, int z, int bodyId,
                               double confidence, const std::string &status)
{
  m_partnerArray.push_back(SynapseLocation(SynapseLocation::PARTNER,
                                           x, y, z, bodyId, confidence,
                                           status));
}

bool ZSynapseAnnotation::loadJsonObject(json_t *object)
{
  //The first level has two keys: "T-bar" and "partners"
  const char *key;
  json_t *value;

  bool hasTbar = false;

  json_object_foreach(object, key, value) {
    if (strcmp(key, "T-bar") == 0) {
      m_tBar.loadJsonObject(value, SynapseLocation::TBAR);
      hasTbar = true;
    } else if (strcmp(key, "partners") == 0) {
      m_partnerArray.resize(json_array_size(value));
      for (size_t i = 0; i < m_partnerArray.size(); i++) {
        m_partnerArray[i].loadJsonObject(json_array_get(value, i),
                                         SynapseLocation::PARTNER);
      }
    }
  }

  return hasTbar;
}

string ZSynapseAnnotation::toString(int indent,
                                    const vector<int> *selectedPartner) const
{
  ostringstream stream;
  string indentStr(indent, ' ');
  if (indent == 0) {
    indentStr.clear();
  }

  stream << indentStr << "{" << endl;

  stream << indentStr << "  \"T-bar\": " << '{' << endl;
  stream << m_tBar.toString(indent + 4) << endl;
  stream << indentStr << "  }";

  vector<const SynapseLocation*> partnerArray;
  if (selectedPartner == NULL) {
    partnerArray = getPartnerRef();
  } else {
    for (vector<int>::const_iterator indexIter = selectedPartner->begin();
         indexIter != selectedPartner->end(); ++indexIter) {
      partnerArray.push_back(&(m_partnerArray[*indexIter]));
    }
  }

  if (!partnerArray.empty()) {
    stream << ',' << endl;
    stream << indentStr << "  \"partners\": [" << endl;

    for (vector<const SynapseLocation*>::const_iterator iter = partnerArray.begin();
         iter != partnerArray.end(); ++iter) {
      if (iter != partnerArray.begin()) {
        stream << "," << endl;
      }
      stream << indentStr << "    {" << endl;
      stream << (*iter)->toString(indent + 6) << endl;
      stream << indentStr << "    }";
    }
    stream << endl << indentStr << "  ]" << endl;
  } else {
    stream << endl;
  }


  stream << indentStr << "}";

  return stream.str();
}

SynapseLocation* ZSynapseAnnotation::getSynapseLocationRef(size_t index)
{
  return const_cast<SynapseLocation*>(
        static_cast<const ZSynapseAnnotation*>(this)->getSynapseLocationRef(index));
}

const SynapseLocation*
ZSynapseAnnotation::getSynapseLocationRef(size_t index) const
{
  if (index == 0) {
    return &m_tBar;
  } else {
    if (index <= m_partnerArray.size()) {
      return &(m_partnerArray[index - 1]);
    }
  }

  return NULL;
}

SynapseLocation& ZSynapseAnnotation::operator [](size_t index)
{
  if ((index > 0) && (index <= m_partnerArray.size())) {
    return m_partnerArray[index - 1];
  }

  return m_tBar;
}

vector<SynapseLocation*> ZSynapseAnnotation::getPartnerRef()
{
  vector<SynapseLocation*> refArray(m_partnerArray.size());
  for (size_t i = 0; i < m_partnerArray.size(); i++) {
    refArray[i] = getPartnerRef(i);
  }

  return refArray;
}

vector<const SynapseLocation*> ZSynapseAnnotation::getPartnerRef() const
{
  vector<const SynapseLocation*> refArray(m_partnerArray.size());
  for (size_t i = 0; i < m_partnerArray.size(); i++) {
    refArray[i] = getPartnerRef(i);
  }

  return refArray;
}

vector<SynapseLocation*> ZSynapseAnnotation::duplicatedPartner(double thre)
{
  ZSynapseLocationMatcher matcher;
  ZSynapseLocationEuclideanMetric metric;
  metric.setRefPoint(getTBarRef()->pos());
  matcher.setMetric(&metric);

  vector<SynapseLocation*> duplicated;

  vector<SynapseLocation*> loc1 = getPartnerRef();
  std::vector<WeightedIntPair> distArray =
      matcher.computePairwiseDistance(loc1, loc1, thre);
  for (size_t i = 0; i < distArray.size(); i++) {
    if (distArray[i].first() < distArray[i].second()) {
      duplicated.push_back(loc1[distArray[i].first()]);
      duplicated.push_back(loc1[distArray[i].second()]);
    }
  }

  return duplicated;
}

vector<SynapseLocation*> ZSynapseAnnotation::getProximalPartner(double distThre)
{
  vector<SynapseLocation*> partnerArray;

  SynapseLocation *tbar = getTBarRef();
  for (size_t i = 0; i < m_partnerArray.size(); i++) {
    SynapseLocation *partner = getPartnerRef(i);
    if (partner->pos().distanceTo(tbar->pos()) < distThre) {
      partnerArray.push_back(partner);
    }
  }

  return partnerArray;
}

vector<SynapseLocation*> ZSynapseAnnotation::getDistalPartner(double distThre)
{
  vector<SynapseLocation*> partnerArray;

  SynapseLocation *tbar = getTBarRef();
  for (size_t i = 0; i < m_partnerArray.size(); i++) {
    SynapseLocation *partner = getPartnerRef(i);
    if (partner->pos().distanceTo(tbar->pos()) > distThre) {
      partnerArray.push_back(partner);
    }
  }

  return partnerArray;
}

bool ZSynapseAnnotation::ZSynapseAnnotation::hasPartner(int bodyId) const
{
  for (size_t i = 0; i < m_partnerArray.size(); i++) {
    const SynapseLocation *partner = getPartnerRef(i);
    if (partner->getBodyId() == bodyId) {
      return true;
    }
  }

  return false;
}
