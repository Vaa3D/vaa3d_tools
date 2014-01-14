#include "zsynapseannotationarray.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <string.h>
#include <list>
#include <algorithm>

#include "zjsonparser.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "zsynapselocationmatcher.h"
#include "tz_error.h"
#include "zgraph.h"
#ifdef _QT_GUI_USED_
#include "zpunctum.h"
#endif
using namespace FlyEm;
using namespace std;

ZSynapseAnnotationArray::ZSynapseAnnotationArray()
{
  m_connectionGraph = NULL;
  m_currentIndex = 0;
  m_currentLocationIndex = 0;
}

ZSynapseAnnotationArray::~ZSynapseAnnotationArray()
{
  deprecate(ALL_COMPONENT);
}

bool ZSynapseAnnotationArray::loadJson(const std::string &filePath,
                                       ELoadDataMode mode)
{
  ZJsonObject jsonObject;
  jsonObject.load(filePath);

  //Two keys at the first level: "data" and "metadata"
  const char *key;
  json_t *value;

  ZJsonArray dataArray;

  json_object_foreach(jsonObject.getValue(), key, value) {
    if (strcmp(key, "metadata") == 0) {
      //Load metadata
      m_metadata.loadJsonObject(value);
    } else if (strcmp(key, "data") == 0) {
      dataArray.set(value, false);
    }
  }

  size_t startIndex = 0;
  switch (mode) {
  case OVERWRITE:
    resize(dataArray.size());
    break;
  case APPEND:
    startIndex = size();
    resize(dataArray.size() + size());
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  int tbarNumber = 0;

  //Extracts elements from the data array
  for (size_t i = 0; i < dataArray.size(); i++) {
    if ((*this)[i + startIndex].loadJsonObject(dataArray.at(i))) {
      tbarNumber++;
    }
  }

  /*
  if (m_metadata.getSourceYDim() > 0) {
    convertRavelerToImageSpace(m_metadata.getSourceZOffset(),
                               m_metadata.getSourceYDim());
  }
  */

  m_source = filePath;

  return (tbarNumber > 0);
}

string ZSynapseAnnotationArray::metadataString()
{
  return m_metadata.toString();
}

string ZSynapseAnnotationArray::toString(
    int indent, const vector<vector<int> > *selected)
{
  ostringstream stream;

  if (selected == NULL) {
    for (ZSynapseAnnotationArray::const_iterator iter = begin(); iter != end();
         ++iter) {
      if (iter != begin()) {
        stream << ',' << endl;
      }
      stream << iter->toString(indent);
    }
  } else {
    for (size_t i = 0; i < selected->size(); i++) {
      if (i != 0) {
        stream << ',' << endl;
      }

      vector<int> selectedPartner;
      for (size_t j = 1; j < (*selected)[i].size(); j++) {
        selectedPartner.push_back((*selected)[i][j]);
      }

      stream << (*this)[(*selected)[i][0]].toString(indent, &selectedPartner);
    }
  }

  return stream.str();
}

string ZSynapseAnnotationArray::toSummaryString()
{
  ostringstream stream;
  stream << m_metadata.toString() << endl;
  stream << size() << " T-Bars" << endl;

  return stream.str();
}

const ZSynapseAnnotation*
ZSynapseAnnotationArray::currentSynapseAnnotation() const
{
  if (empty()) {
    return NULL;
  }

  if (m_currentIndex >= size()) {
    return NULL;
  }

  return &((*this)[m_currentIndex]);
}

SynapseLocation* ZSynapseAnnotationArray::currentSynapseLocation()
{
  return const_cast<SynapseLocation*>(
        static_cast<const ZSynapseAnnotationArray*>(this)->
        currentSynapseLocation());
}

const SynapseLocation* ZSynapseAnnotationArray::currentSynapseLocation() const
{
  if (empty()) {
    return NULL;
  }

  if (m_currentIndex >= size()) {
    return NULL;
  }

  return (*this)[m_currentIndex].getSynapseLocationRef(m_currentLocationIndex);
}

SynapseLocation* ZSynapseAnnotationArray::beginSynapseLocation()
{
  return const_cast<SynapseLocation*>(
        static_cast<const ZSynapseAnnotationArray*>(this)->
        beginSynapseLocation());
}

const SynapseLocation* ZSynapseAnnotationArray::beginSynapseLocation() const
{
  m_currentIndex = 0;
  m_currentLocationIndex = 0;

  return currentSynapseLocation();
}


SynapseLocation* ZSynapseAnnotationArray::nextSynapseLocation()
{
  return const_cast<SynapseLocation*>(
        static_cast<const ZSynapseAnnotationArray*>(this)->
        nextSynapseLocation());
}

const SynapseLocation* ZSynapseAnnotationArray::nextSynapseLocation() const
{
  m_currentLocationIndex++;

  if (empty()) {
    return NULL;
  }

  if (currentSynapseLocation() == NULL) {
    m_currentLocationIndex = 0;
    m_currentIndex++;
  }

  return currentSynapseLocation();
}

bool ZSynapseAnnotationArray::isPartnerOnBody(int bodyId) const
{
  if (m_currentLocationIndex == 0) {
    const std::vector<SynapseLocation>* partnerArray =
        (*this)[m_currentIndex].getPartnerArrayRef();

    //Return true if at least one PSD is on the body
    for (size_t i = 0; i < partnerArray->size(); i++) {
      if ((*partnerArray)[i].getBodyId() == bodyId) {
        return true;
      }
    }
  } else {
    if ((*this)[m_currentIndex].getTBarRef()->getBodyId() == bodyId) {
      return true;
    }
  }

  return false;
}

bool ZSynapseAnnotationArray::exportMarkerFile(
    string filePath, const SynapseAnnotationConfig &config,
    SynapseLocation::ELocationSpace spaceOption,
    const SynapseDisplayConfig &displayConfig
    )
{

  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    cout << "Failed to open " << filePath << endl;
    return false;
  }

  for (SynapseLocation *synapse = beginSynapseLocation();
       synapse != NULL; synapse = nextSynapseLocation()) {
    bool display = false;
    static const int kOnBody = 4;
    static const int kPartnerOnBody = 2;
    int locationMode; //0: N; 2: P; 4: O

    if (displayConfig.bodyId < 0) {
      locationMode = kOnBody | kPartnerOnBody;
    } else if (synapse->bodyId() == displayConfig.bodyId) {
      locationMode = kOnBody;
    } else if (isPartnerOnBody(displayConfig.bodyId)) {
      locationMode = kPartnerOnBody;
    } else {
      locationMode = 0;
    }

    if (locationMode == 0) {
      display = false;
    } else {
      if (locationMode & kOnBody) {
        if ((displayConfig.mode == SynapseDisplayConfig::SYNAPSE_PAIR) ||
            (displayConfig.mode == SynapseDisplayConfig::HALF_SYNAPSE)) {
          display = true;
        } else {
          if ((displayConfig.mode == SynapseDisplayConfig::TBAR_ONLY) ||
              (displayConfig.mode == SynapseDisplayConfig::TBAR_PAIR)) {
            if (synapse->isTBar()) {
              display = true;
            }
          } else {
            if (synapse->isPartner()) {
              display = true;
            }
          }
        }
      }

      if (locationMode & kPartnerOnBody) {
        switch(displayConfig.mode) {
        case SynapseDisplayConfig::TBAR_PAIR:
          display |= synapse->isPartner();
          break;
        case SynapseDisplayConfig::PSD_PAIR:
          display |= synapse->isTBar();
          break;
        case SynapseDisplayConfig::SYNAPSE_PAIR:
          display = true;
          break;
        default:
          display |= false;
        }
      }
    }

    if (display) {
      ZPoint pt = synapse->mapPosition(config, spaceOption);
      pt += 1.0;

      if (synapse->isTBar()) {
        stream << pt.x() << ',' << pt.y() << ',' << pt.z()
               << ',' << "3,1,,,"
               << static_cast<int>(displayConfig.tBarColor.red) << ","
               << static_cast<int>(displayConfig.tBarColor.green) << ","
               << static_cast<int>(displayConfig.tBarColor.blue) << "," << endl;
      } else {
        stream << pt.x() << ',' << pt.y() << ',' << pt.z()
               << ',' << "3,1,,,"
               << static_cast<int>(displayConfig.psdColor.red) << ","
               << static_cast<int>(displayConfig.psdColor.green) << ","
               << static_cast<int>(displayConfig.psdColor.blue) << "," << endl;
      }
    }
  }

  stream.close();

  return true;
}

bool ZSynapseAnnotationArray::exportPsd(const string &filePath)
{
  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    cout << "Failed to open " << filePath << endl;
    return false;
  }

  for (SynapseLocation *synapse = beginSynapseLocation();
       synapse != NULL; synapse = nextSynapseLocation()) {
    if (synapse->isPartner()) {
      stream << synapse->bodyId() << "," << synapse->x() << "," << synapse->y()
             << "," << synapse->z() << std::endl;
    }
  }
  stream.close();

  return true;
}

bool ZSynapseAnnotationArray::exportTBar(const string &filePath)
{
  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    cout << "Failed to open " << filePath << endl;
    return false;
  }

  for (SynapseLocation *synapse = beginSynapseLocation();
       synapse != NULL; synapse = nextSynapseLocation()) {
    if (synapse->isTBar()) {
      stream << synapse->bodyId() << "," << synapse->x() << "," << synapse->y()
             << "," << synapse->z() << std::endl;
    }
  }
  stream.close();

  return true;
}

bool ZSynapseAnnotationArray::exportJsonFile(
    const string &filePath, const std::vector<std::vector<int> > *selected)
{
  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    cout << "Failed to open " << filePath << endl;
    return false;
  }

  stream << "{" << endl;
  stream << "  \"data\": [" << endl;
  //stream << "    {" << endl;
  stream << toString(4, selected) << endl;
  //stream << "    }" << endl;
  stream << "  ]," << endl;

  stream << "  \"metadata\": {" << endl;
  stream << m_metadata.toString(4) << endl;
  stream << "  }" << endl;
  stream << "}" << endl;

  return true;
}

bool
ZSynapseAnnotationArray::exportSwcFile(string filePath,
                                       const SynapseAnnotationConfig &config,
                                       SynapseLocation::ELocationSpace spaceOption,
                                       const SynapseDisplayConfig &displayConfig)
{
  return exportConnection(filePath, config, spaceOption, displayConfig);
}

void ZSynapseAnnotationArray::simplifyCommonSwcPath()
{
  SynapseLocation *synapse = beginSynapseLocation();

  if (!synapse->getSwcLocationRef()->swcFilePath.empty()) {

  }
}

bool ZSynapseAnnotationArray::exportConnection(
    string filePath, const SynapseAnnotationConfig &config,
    SynapseLocation::ELocationSpace spaceOption,
    const SynapseDisplayConfig &displayConfig)
{
  Swc_Tree_Node *root = Make_Virtual_Swc_Tree_Node();

  for (size_t i = 0; i < size(); i++) {
    SynapseLocation *tBar = (*this)[i].getTBarRef();
    vector<SynapseLocation> *partnerArray = (*this)[i].getPartnerArrayRef();
    int status = 0;

    if ((displayConfig.mode == SynapseDisplayConfig::SYNAPSE_PAIR) ||
        (displayConfig.mode == SynapseDisplayConfig::TBAR_PAIR)) {
      if (tBar->getBodyId() == displayConfig.bodyId) {
        status = 2;
      } else if (displayConfig.mode == SynapseDisplayConfig::SYNAPSE_PAIR) {
        status = 1;
      }
    } else if (displayConfig.mode == SynapseDisplayConfig::PSD_PAIR) {
      status = 1;
    }

    if (status > 0) {
      ZPoint startPos;
      ZPoint endPos;

      startPos = tBar->mapPosition(config, spaceOption);
      double startSize = 3;
      double endSize = startSize / 10.0;
      int startType = 7;
      int endType = 3;

      for (size_t j = 0; j < partnerArray->size(); j++) {
        if (status == 1) {
          //Partner on the body
          if ((*partnerArray)[j].bodyId() == displayConfig.bodyId) {
            status = 3;
          }
        }

        if (status >= 2) {
          endPos = (*partnerArray)[j].mapPosition(config, spaceOption);
          Swc_Tree_Node *arrow = ZSwcTree::makeArrow(startPos, startSize,
                                                     startType, endPos, endSize,
                                                     endType, false);
          Swc_Tree_Node_Set_Label(arrow, tBar->bodyId());
          Swc_Tree_Node_Set_Label(arrow->first_child,
                                  (*partnerArray)[j].bodyId());

          Swc_Tree_Node_Set_Parent(arrow, root);
        }

        if (status == 3) {
          status = 1;
        }
      }
    }
  }

  ZSwcTree tree;
  tree.setDataFromNode(root);
  tree.resortId();
  tree.save(filePath.c_str());

  return true;
}

vector<ZVaa3dMarker>
ZSynapseAnnotationArray::toMarkerArray(
    const SynapseAnnotationConfig &config,
    SynapseLocation::ELocationSpace spaceOption,
    const SynapseDisplayConfig &displayConfig) const
{
  vector<ZVaa3dMarker> markerArray;
  int currentTBarId = 0;

  for (const SynapseLocation *synapse = beginSynapseLocation();
       synapse != NULL; synapse = nextSynapseLocation()) {
    if (synapse->isTBar()) {
      currentTBarId = synapse->bodyId();
    }

    bool display = false;
    static const int kOnBody = 4;
    static const int kPartnerOnBody = 2;
    int locationMode; //0: N; 2: P; 4: O

    if (displayConfig.bodyId < 0) { //For all bodies
      locationMode = kOnBody | kPartnerOnBody;
    } else if (synapse->bodyId() == displayConfig.bodyId) {
      locationMode = kOnBody;
    } else if (isPartnerOnBody(displayConfig.bodyId)) {
      locationMode = kPartnerOnBody;
    } else {
      locationMode = 0;
    }

    if (locationMode == 0) { //Neither tbar nor psd hits body
      display = false;
    } else {
      if (locationMode & kOnBody) { //location hits body
        if ((displayConfig.mode == SynapseDisplayConfig::SYNAPSE_PAIR) ||
            (displayConfig.mode == SynapseDisplayConfig::HALF_SYNAPSE)) {
          display = true;
        } else {
          if ((displayConfig.mode == SynapseDisplayConfig::TBAR_ONLY) ||
              (displayConfig.mode == SynapseDisplayConfig::TBAR_PAIR)) {
            if (synapse->isTBar()) { //t-bar hits body
              display = true;
            }
          } else if ((displayConfig.mode == SynapseDisplayConfig::PSD_ONLY) ||
                     (displayConfig.mode == SynapseDisplayConfig::PSD_PAIR)) { //Show PSD only
            if (synapse->isPartner()) { //PSD-hits body
              display = true;
            }
          }
        }
      }

      if (locationMode & kPartnerOnBody) { //Partner hits body
        switch(displayConfig.mode) {
        case SynapseDisplayConfig::TBAR_PAIR:
          display |= synapse->isPartner(); //the partner is T-bar
          break;
        case SynapseDisplayConfig::PSD_PAIR:
          display |= synapse->isTBar(); //The partner is PSD
          break;
        case SynapseDisplayConfig::SYNAPSE_PAIR:
          display = true;
          break;
        default:
          display |= false;
        }
      }
    }

    if (display && displayConfig.buddyBodyId >= 0) {
      if (synapse->isTBar()) {
        display = currentSynapseAnnotation()->hasPartner(
              displayConfig.buddyBodyId);
      } else {
        display = (currentTBarId == displayConfig.buddyBodyId);
      }
    }

    if (display) {
#ifdef _DEBUG_2
      cout << synapse->toString() << endl;
#endif
      ZPoint pt = synapse->mapPosition(config, spaceOption);
      ZVaa3dMarker marker;
      marker.setCenter(pt.x(), pt.y(), pt.z());
      marker.setRadius(config.sizeScale);
      if (synapse->isTBar()) {
        marker.setColor(displayConfig.tBarColor.red,
                        displayConfig.tBarColor.green,
                        displayConfig.tBarColor.blue);
        marker.setType(1);
      } else {
        marker.setColor(displayConfig.psdColor.red,
                        displayConfig.psdColor.green,
                        displayConfig.psdColor.blue);
        marker.setType(2);
      }
      ostringstream nameStream;

      nameStream << currentTBarId;

      //marker.setName(nameStream.str());

      ostringstream commentStream;
      if (synapse->isPartner()) {
        commentStream << currentTBarId << "_";
      }
      commentStream << synapse->bodyId();

      marker.setName(commentStream.str());

      marker.setComment(commentStream.str());
      markerArray.push_back(marker);
    }
  }

  return markerArray;
}

vector<ZVaa3dApo>
ZSynapseAnnotationArray::toApoArray(
    const SynapseAnnotationConfig &config,
    SynapseLocation::ELocationSpace spaceOption,
    const SynapseDisplayConfig &displayConfig)
{
  vector<ZVaa3dApo> markerArray;

  for (SynapseLocation *synapse = beginSynapseLocation();
       synapse != NULL; synapse = nextSynapseLocation()) {
    bool display = false;
    static const int kOnBody = 4;
    static const int kPartnerOnBody = 2;
    int locationMode; //0: N; 2: P; 4: O

    if (displayConfig.bodyId < 0) {
      locationMode = kOnBody | kPartnerOnBody;
    } else if (synapse->bodyId() == displayConfig.bodyId) {
      locationMode = kOnBody;
    } else if (isPartnerOnBody(displayConfig.bodyId)) {
      locationMode = kPartnerOnBody;
    } else {
      locationMode = 0;
    }

    if (locationMode == 0) {
      display = false;
    } else {
      if (locationMode & kOnBody) {
        if ((displayConfig.mode == SynapseDisplayConfig::SYNAPSE_PAIR) ||
            (displayConfig.mode == SynapseDisplayConfig::HALF_SYNAPSE)) {
          display = true;
        } else {
          if ((displayConfig.mode == SynapseDisplayConfig::TBAR_ONLY) ||
              (displayConfig.mode == SynapseDisplayConfig::TBAR_PAIR)) {
            if (synapse->isTBar()) {
              display = true;
            }
          } else {
            if (synapse->isPartner()) {
              display = true;
            }
          }
        }
      }

      if (locationMode & kPartnerOnBody) {
        switch(displayConfig.mode) {
        case SynapseDisplayConfig::TBAR_PAIR:
          display |= synapse->isPartner();
          break;
        case SynapseDisplayConfig::PSD_PAIR:
          display |= synapse->isTBar();
          break;
        case SynapseDisplayConfig::SYNAPSE_PAIR:
          display = true;
          break;
        default:
          display |= false;
        }
      }
    }

    if (display) {
      ZPoint pt = synapse->mapPosition(config, spaceOption);
      ZVaa3dApo marker;
      marker.setCenter(pt.x(), pt.y(), pt.z());
      if (synapse->isTBar()) {
        marker.setColor(displayConfig.tBarColor.red,
                        displayConfig.tBarColor.green,
                        displayConfig.tBarColor.blue);
      } else {
        marker.setColor(displayConfig.psdColor.red,
                        displayConfig.psdColor.green,
                        displayConfig.psdColor.blue);
      }
      marker.setId(markerArray.size() + 1);
      markerArray.push_back(marker);
    }
  }

  return markerArray;
}

ZSwcTree* ZSynapseAnnotationArray::toSwcTree()
{
  Swc_Tree_Node *root = Make_Virtual_Swc_Tree_Node();

  for (size_t i = 0; i < size(); i++) {
    SynapseLocation *tBar = (*this)[i].getTBarRef();
    vector<SynapseLocation> *partnerArray = (*this)[i].getPartnerArrayRef();

    ZPoint startPos;
    ZPoint endPos;

    startPos = tBar->pos();

    int preType = 8;
    int postType = 1;
    double baseRadius = 1.0;
    Swc_Tree_Node *preNode =
        SwcTreeNode::makePointer(0, preType, startPos,
                                 tBar->confidence() + baseRadius, -1);
    SwcTreeNode::setParent(preNode, root);

    for (size_t j = 0; j < partnerArray->size(); j++) {
      endPos = (*partnerArray)[j].pos();
      Swc_Tree_Node *postNode =
          SwcTreeNode::makePointer(0, postType, endPos,
                                   (*partnerArray)[j].confidence() + baseRadius,
                                   -1);
      SwcTreeNode::setParent(postNode, preNode);
    }
  }

  ZSwcTree *tree = new ZSwcTree();
  tree->setDataFromNode(root);
  tree->resortId();
  tree->setSource(m_source);

  return tree;
}

vector<FlyEm::SynapseLocation*> ZSynapseAnnotationArray::toTBarRefArray()
{
  vector<FlyEm::SynapseLocation*> refArray(size());

  for (size_t i = 0; i < size(); i++) {
    refArray[i] = (*this)[i].getTBarRef();
  }

  return refArray;
}

vector<FlyEm::SynapseLocation*> ZSynapseAnnotationArray::toMultiTBarRefArray()
{
  vector<FlyEm::SynapseLocation*> refArray;

  for (size_t i = 0; i < size(); i++) {
    SynapseLocation *tbar = (*this)[i].getTBarRef();
    if (tbar->isMulti()) {
      refArray.push_back(tbar);
    }
  }

  return refArray;
}

vector<FlyEm::SynapseLocation*> ZSynapseAnnotationArray::toPsdRefArray()
{
  vector<FlyEm::SynapseLocation*> refArray;

  for(SynapseLocation *loc = beginSynapseLocation(); loc != NULL;
      loc = nextSynapseLocation()) {
    if (loc->isPartner()) {
      refArray.push_back(loc);
    }
  }

  return refArray;
}

vector<FlyEm::SynapseLocation*> ZSynapseAnnotationArray::toPsdRefArray(int index)
{
  std::vector<SynapseLocation> *partnerRef =
      (*this)[index].getPartnerArrayRef();

  vector<FlyEm::SynapseLocation*> refArray(partnerRef->size());

  for (size_t i = 0; i < refArray.size(); i++) {
    refArray[i] = &((*partnerRef)[i]);
  }

  return refArray;
}

vector<FlyEm::SynapseLocation*>
ZSynapseAnnotationArray::toSynapseLocationArray()
{
  vector<FlyEm::SynapseLocation*> refArray;

  for(SynapseLocation *loc = beginSynapseLocation(); loc != NULL;
      loc = nextSynapseLocation()) {
    refArray.push_back(loc);
  }

  return refArray;
}

int ZSynapseAnnotationArray::
getPsdIndex(size_t tbarIndex, size_t partnerIndex) const
{
  int index = 0;

  for (size_t i = 0; i < tbarIndex; i++) {
    index += (*this)[i].getPartnerArrayRef()->size() + 1;
  }

  index += partnerIndex + 1;

#ifdef _DEBUG_
  if (index == 138) {
    cout << (*this)[tbarIndex].getPartnerArrayRef()->size() << endl;
    cout << (*this)[tbarIndex].getTBarRef()->pos().toString() << " "
         << (*this)[tbarIndex].getPartnerRef(partnerIndex)->pos().toString() << endl;
  }
#endif

  return index;
}

pair<int, int> ZSynapseAnnotationArray::relativePsdIndex(int index)
{
  pair<int, int> sub;
  sub.first = -1;
  sub.second = -1;

  ZSynapseAnnotationArray::const_iterator iter;
  for (iter = begin(); iter != end(); ++iter) {
    index -= iter->partnerNumber() + 1;
    if (index < 0) {
      break;
    }
  }

  if (index < 0) {
    sub.first = iter - begin();
    sub.second = index + iter->partnerNumber();
  }

  return sub;
}

int ZSynapseAnnotationArray::getPsdIndex(int x, int y, int z) const
{
  int index = 0;

  const SynapseLocation *loc = beginSynapseLocation();
  while (loc != NULL) {
    if (loc->isPartner()) {
      if (loc->x() == x && loc->y() == y && loc->z() == z) {
        return index;
      }
    }
    index++;
    loc = nextSynapseLocation();
  }

  return 0;
}

void ZSynapseAnnotationArray::printSummary()
{
  cout << size() << " TBars" << endl;
  for (size_t i = 0; i < size(); i++) {
    cout << "TBar " << i << " " << (*this)[i].getPartnerArrayRef()->size() <<
            " partners" << endl;
  }
}

void ZSynapseAnnotationArray::print()
{
  cout << size() << " TBars" << endl;
  for (size_t i = 0; i < size(); i++) {
    cout << "TBar " << i + 1<< ": " << endl;
    getTBarRef(i)->print();

    cout << "Partners" << endl;
    std::vector<SynapseLocation>* partnerArray = (*this)[i].getPartnerArrayRef();
    for (size_t j = 0; j < partnerArray->size(); ++j) {
      (*partnerArray)[j].print();
    }
    cout << endl;
  }
}

SynapseLocation* ZSynapseAnnotationArray::getTBarRef(int index)
{
  return (*this)[index].getTBarRef();
}

const SynapseLocation* ZSynapseAnnotationArray::getTBarRef(int index) const
{
  return (*this)[index].getTBarRef();
}

SynapseLocation* ZSynapseAnnotationArray::getPsdRef(int tbarIndex, int psdIndex)
{
  return (*this)[tbarIndex].getPartnerRef(psdIndex);
}

const SynapseLocation* ZSynapseAnnotationArray::getPsdRef(
    int tbarIndex, int psdIndex) const
{
  return (*this)[tbarIndex].getPartnerRef(psdIndex);
}

int ZSynapseAnnotationArray::getPsdNumber() const
{
  int count = 0;

  for (size_t i = 0; i < size(); i++) {
    count += (*this)[i].getPartnerArrayRef()->size();
  }

  return count;
}

int ZSynapseAnnotationArray::getTBarNumber() const
{
  return size();
}


void ZSynapseAnnotationArray::assembleTBarSequence(ZIntTree *tbarSequence,
                            ZIntTree *tbarSequenceReverse,
                            int endIndex, int freeIndex)
{
  if ((*tbarSequence)[endIndex] >= 0) {
    (*tbarSequence)[freeIndex] = endIndex;
    (*tbarSequenceReverse)[endIndex] = freeIndex;
  } else {
    (*tbarSequenceReverse)[freeIndex] = endIndex;
    (*tbarSequence)[endIndex] = freeIndex;
  }
}

vector<vector<FlyEm::SynapseLocation*> >
ZSynapseAnnotationArray::buildTbarSequence(double maxDist)
{
  vector<SynapseLocation*> locArray1 = toMultiTBarRefArray();

#ifdef _DEBUG_
  cout << "Tbar number: " << locArray1.size() << endl;
#endif

  //Compute pairwise distances of the tbars
  ZSynapseLocationMatcher matcher;
  vector<WeightedIntPair> distanceArray =
      matcher.computePairwiseDistance(locArray1, locArray1, maxDist);

#ifdef _DEBUG_
  cout << "Neighboring pair number: " << distanceArray.size() << endl;
#endif

  //Sort the distances
  sort(distanceArray.begin(), distanceArray.end(), WeightedIntPairCompare());

  //Seed set <- all tbars
  list<WeightedIntPair> seedList;
  for (vector<WeightedIntPair>::const_iterator iter = distanceArray.begin();
       iter != distanceArray.end(); ++iter) {
    if (iter->first() != iter->second()) {
      seedList.push_back(*iter);
    }
  }

#ifdef _DEBUG_
  cout << "Seed pair number: " << seedList.size() << endl;
  for (list<WeightedIntPair>::const_iterator iter = seedList.begin();
       iter != seedList.end(); ++iter) {
    cout << iter->first() << " " << iter->second() << " " << iter->weight()
         << endl;
  }
#endif

  ZIntTree tbarSequence(size());
  ZIntTree tbarSequenceReverse(size());

  //While the seed set is not empty
  while (!seedList.empty()) {
    //Find the closest pair in the seed set
    WeightedIntPair wp = seedList.front();
    seedList.pop_front();

    //Expand the sequences
    //If neither tbar is assembled
    if (tbarSequence[wp.first()] < 0 && tbarSequence[wp.second()] < 0 &&
        tbarSequenceReverse[wp.first()] < 0 &&
        tbarSequenceReverse[wp.second()] < 0) {
      tbarSequence[wp.second()] = wp.first();
      tbarSequenceReverse[wp.first()] = wp.second();
    } else if ((tbarSequence[wp.first()] < 0 &&
                tbarSequenceReverse[wp.first()] < 0)
               || (tbarSequence[wp.second()] < 0 &&
                   tbarSequenceReverse[wp.second()] < 0)) {
      //If one tbar is assembled and one is not
      //Get the index of assembled one
      int index = wp.first();
      int freeIndex = wp.second();
      if (tbarSequence[index] < 0 && tbarSequenceReverse[index] < 0) {
        index = wp.second();
        freeIndex = wp.first();
      }
      //If the assembled tbar is a terminal
      if (tbarSequence[index] < 0 || tbarSequenceReverse[index] < 0) {
        //Get the direction of the sequence
        ZPoint seqEnd = getTBarRef(index)->pos();
        int index2 = tbarSequence[index];
        if (index2 < 0) {
          index2 = tbarSequenceReverse[index];
        }
        ZPoint seqVec = seqEnd - getTBarRef(index2)->pos();
        //Add the free tbar to the consistent direction
        ZPoint pt = getTBarRef(freeIndex)->pos();
        ZPoint vec = pt - seqEnd;
        if (vec.dot(seqVec) > 0) {
          assembleTBarSequence(&tbarSequence, &tbarSequenceReverse,
                               index, freeIndex);
        }
      }
    }
  }

  vector<vector<SynapseLocation*> > tbarSequenceArray;

  std::vector<bool> isLeaf(size(), true);
  for (size_t i = 0; i < size(); i++) {
    vector<int> trace = tbarSequence.traceBack(i);
    for (size_t j = 1; j < trace.size(); j++) {
      isLeaf[trace[j]] = false;
    }
  }

  for (size_t i = 0; i < size(); i++) {
    if (isLeaf[i]) {
      vector<int> trace = tbarSequence.traceBack(i);
      if (trace.size() > 1) {
        vector<SynapseLocation*> refArray;
        for (size_t j = 0; j < trace.size(); j++) {
          refArray.push_back(locArray1[trace[j]]);
        }
        tbarSequenceArray.push_back(refArray);
      }
    }
  }

  return tbarSequenceArray;
}

vector<pair<SynapseLocation*, SynapseLocation*> >
FlyEm::ZSynapseAnnotationArray::findDuplicatedTBar(double minDist)
{
  vector<pair<SynapseLocation*, SynapseLocation*> > duplicateSet;

  for (size_t i = 0; i < size(); ++i) {
    for (size_t j = i + 1; j < size(); ++j) {
      SynapseLocation *firstTBar = getTBarRef(i);
      SynapseLocation *secondTBar = getTBarRef(j);

      TZ_ASSERT(firstTBar != secondTBar, "Same pointer");

      double dist = firstTBar->pos().distanceTo(secondTBar->pos());
      if (dist < minDist) {
        duplicateSet.push_back(pair<SynapseLocation*, SynapseLocation*>(
                                 firstTBar, secondTBar));
      }
    }
  }

  return duplicateSet;
}

ZGraph* FlyEm::ZSynapseAnnotationArray::toGraph()
{
  ZGraph *graph = new ZGraph(ZGraph::DIRECTED_WITH_WEIGHT);

  for (size_t i = 0; i < size(); i++) {
    for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
      int tbarBodyId = getTBarRef(i)->bodyId();
      int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
      graph->addEdge(tbarBodyId, psdBodyId);
    }
  }

  return graph;
}

ZGraph* FlyEm::ZSynapseAnnotationArray::toGraph(const std::set<int> &bodySet)
{
  ZGraph *graph = new ZGraph(ZGraph::DIRECTED_WITH_WEIGHT);

  for (size_t i = 0; i < size(); i++) {
    for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
      int tbarBodyId = getTBarRef(i)->bodyId();
      int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
      if (bodySet.count(tbarBodyId) > 0 && bodySet.count(psdBodyId) > 0) {
        graph->addEdge(tbarBodyId, psdBodyId);
      }
    }
  }

  return graph;
}

vector<int> FlyEm::ZSynapseAnnotationArray::countPsd()
{
  vector<int> psdCount;

  for (size_t i = 0; i < size(); i++) {
    for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
      int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
      if (psdBodyId >= (int) psdCount.size()) {
        psdCount.resize(psdBodyId + 1, 0);
      }
      psdCount[psdBodyId]++;
    }
  }

#ifdef _DEBUG_2
  int count = 0;
  for (size_t i = 0; i < psdCount.size(); ++i) {
    count += psdCount[i];
  }

  std::cout << psdCount[0] << std::endl;
  std::cout << "Total number: " << count << std::endl;
#endif

  return psdCount;
}

int FlyEm::ZSynapseAnnotationArray::countPsd(int bodyId) const
{
  int count = 0;
  for (size_t i = 0; i < size(); i++) {
    for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
      int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
      if (psdBodyId == bodyId) {
        ++count;
      }
    }
  }

  return count;
}

bool FlyEm::ZSynapseAnnotationArray::hasPsd(int bodyId) const
{
  for (size_t i = 0; i < size(); i++) {
    for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
      int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
      if (psdBodyId == bodyId) {
        return true;
      }
    }
  }

  return false;
}

vector<int> FlyEm::ZSynapseAnnotationArray::countTBar()
{
  vector<int> tbarCount;
  for (size_t i = 0; i < size(); i++) {
    int tbarBodyId = getTBarRef(i)->bodyId();
    if (tbarBodyId >= (int) tbarCount.size()) {
      tbarCount.resize(tbarBodyId + 1, 0);
    }
    tbarCount[tbarBodyId]++;
  }

  return tbarCount;
}

int FlyEm::ZSynapseAnnotationArray::countTBar(int bodyId) const
{
  int count = 0;
  for (size_t i = 0; i < size(); i++) {
    int tbarBodyId = getTBarRef(i)->bodyId();
    if (tbarBodyId == bodyId) {
      ++count;
    }
  }

  return count;
}

bool FlyEm::ZSynapseAnnotationArray::hasTBar(int bodyId) const
{
  for (size_t i = 0; i < size(); i++) {
    int tbarBodyId = getTBarRef(i)->bodyId();
    if (tbarBodyId == bodyId) {
      return true;
    }
  }

  return false;
}

int FlyEm::ZSynapseAnnotationArray::countInputNeuron(int bodyId) const
{
  std::set<int> idSet;
  for (size_t i = 0; i < size(); i++) {
    if ((*this)[i].getTBarRef()->bodyId() == bodyId) {
      for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
        int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
        idSet.insert(psdBodyId);
      }
    }
  }

  return idSet.size();
}

int FlyEm::ZSynapseAnnotationArray::countOutputNeuron(int bodyId) const
{
  std::set<int> idSet;

  for (size_t i = 0; i < size(); i++) {
    for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
      int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
      if (psdBodyId == bodyId) {
        idSet.insert((*this)[i].getTBarRef()->bodyId());
        break;
      }
    }
  }

  return idSet.size();
}

vector<int> FlyEm::ZSynapseAnnotationArray::countSynapse()
{
  vector<int> count;

  for (size_t i = 0; i < size(); i++) {
    for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
      int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
      if (psdBodyId >= (int) count.size()) {
        count.resize(psdBodyId + 1, 0);
      }
      count[psdBodyId]++;
    }
  }

  for (size_t i = 0; i < size(); i++) {
    int tbarBodyId = getTBarRef(i)->bodyId();
    if (tbarBodyId >= (int) count.size()) {
      count.resize(tbarBodyId + 1, 0);
    }
    count[tbarBodyId]++;
  }


  return count;
}

std::vector<SynapseLocation*>
FlyEm::ZSynapseAnnotationArray::getTBarArray(int bodyId) const
{
  std::vector<SynapseLocation*> synapseArray;
  for (size_t i = 0; i < size(); i++) {
    const SynapseLocation *tbar = getTBarRef(i);
    if (bodyId == tbar->getBodyId()) {
      synapseArray.push_back(const_cast<SynapseLocation*>(tbar));
    }
  }

  return synapseArray;
}

std::vector<SynapseLocation*>
FlyEm::ZSynapseAnnotationArray::getPsdArray(int bodyId) const
{
  std::vector<SynapseLocation*> synapseArray;
  for (size_t i = 0; i < size(); i++) {
    for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
      const SynapseLocation *psd = (*this)[i].getPartnerRef(j);
      if (bodyId == psd->getBodyId()) {
        synapseArray.push_back(const_cast<SynapseLocation*>(psd));
      }
    }
  }

  return synapseArray;
}


void FlyEm::ZSynapseAnnotationArray::convertRavelerToImageSpace(
    int startZ, int height)
{
  for (SynapseLocation *synapse = beginSynapseLocation();
       synapse != NULL; synapse = nextSynapseLocation()) {
    synapse->convertRavelerToImageSpace(startZ, height);
  }
}

#ifdef _QT_GUI_USED_
vector<ZPunctum*> FlyEm::ZSynapseAnnotationArray::toPuncta(
    const SynapseAnnotationConfig &config,
    SynapseLocation::ELocationSpace spaceOption,
    const SynapseDisplayConfig &displayConfig) const
{
  vector<ZPunctum*> puncta;
  vector<ZVaa3dMarker> markerArray = toMarkerArray(
        config, spaceOption, displayConfig);
  for (vector<ZVaa3dMarker>::iterator iter = markerArray.begin();
       iter != markerArray.end(); ++iter) {
    ZPunctum *punctum = new ZPunctum;
    punctum->setFromMarker(*iter);
    //punctum->setRadius(punctum->radius() * 3.0);
    puncta.push_back(punctum);
  }

  return puncta;
}
#endif

int FlyEm::ZSynapseAnnotationArray::getStrongestInput(int bodyId) const
{
  std::map<int, int> idMap;
  int id = -1;
  int maxCount = 0;
  for (size_t i = 0; i < size(); i++) {
    if ((*this)[i].getTBarRef()->bodyId() == bodyId) {
      for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
        int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
        int count = 1;
        if (idMap.count(psdBodyId) == 0) {
          idMap[psdBodyId] = 1;
        } else {
          count = ++(idMap[psdBodyId]);
        }
        if (count > maxCount) {
          maxCount = count;
          id = psdBodyId;
        }
      }
    }
  }

  return id;
}

int FlyEm::ZSynapseAnnotationArray::getStrongestOutput(int bodyId) const
{
  std::map<int, int> idMap;
  int id = -1;
  int maxCount = 0;

  for (size_t i = 0; i < size(); i++) {
    int tbarBodyId = (*this)[i].getTBarRef()->bodyId();
    for (size_t j = 0; j < (*this)[i].getPartnerArrayRef()->size(); ++j) {
      int psdBodyId = (*this)[i].getPartnerRef(j)->bodyId();
      if (psdBodyId == bodyId) {
        int count = 1;
        if (idMap.count(tbarBodyId) == 0) {
          idMap[tbarBodyId] = 1;
        } else {
          count = ++(idMap[tbarBodyId]);
        }
        if (count > maxCount) {
          maxCount = count;
          id = tbarBodyId;
        }
      }
    }
  }

  return id;
}

void ZSynapseAnnotationArray::deprecateDependent(EComponent /*component*/)
{
}


void ZSynapseAnnotationArray::deprecate(EComponent component)
{
  deprecateDependent(component);
  switch (component) {
  case CONNECTION_GRAPH:
    delete m_connectionGraph;
    m_connectionGraph = NULL;
    break;
  case ALL_COMPONENT:
    deprecate(CONNECTION_GRAPH);
    break;
  }
}

bool ZSynapseAnnotationArray::isDeprecated(EComponent component) const
{
  switch (component) {
  case CONNECTION_GRAPH:
    return m_connectionGraph == NULL;
  case ALL_COMPONENT:
    return TRUE;
    break;
  }

  return FALSE;
}


ZGraph* ZSynapseAnnotationArray::getConnectionGraph(bool excludingSelfConnection)
{
  if (isDeprecated(CONNECTION_GRAPH)) {
    m_connectionGraph = new ZGraph(ZGraph::DIRECTED_WITH_WEIGHT);
    //For each synapse
    for (ZSynapseAnnotationArray::const_iterator iter = begin();
         iter != end(); ++iter) {
      //Add edge
      int tbarId = iter->getTBarRef()->bodyId();
      vector<const SynapseLocation*> psdArray = iter->getPartnerRef();
      for (vector<const SynapseLocation*>::const_iterator psdIter =
           psdArray.begin(); psdIter != psdArray.end(); ++psdIter) {
        int psdId = (*psdIter)->bodyId();
        bool isGoodEdge = true;
        if (excludingSelfConnection) {
          if (tbarId == psdId) {
            isGoodEdge = false;
          }
        }

        if (isGoodEdge) {
          bool edgeExisted = false;
          if (m_connectionGraph->getEdgeIndex(tbarId, psdId) >= 0) {
            edgeExisted = true;
          }

          if (!edgeExisted) {
            Graph_Add_Weighted_Edge(m_connectionGraph->getRawGraph(),
                                    tbarId, psdId, 1.0);
            m_connectionGraph->expandEdgeTable(tbarId, psdId);
          } else {
            m_connectionGraph->setEdgeWeight(
                  tbarId, psdId,
                  m_connectionGraph->getEdgeWeight(tbarId, psdId) + 1.0);
          }
        }
      }
    }
  }

  return m_connectionGraph;
}

bool FlyEm::ZSynapseAnnotationArray::exportCsvFile(const string &filePath)
{
  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    cout << "Failed to open " << filePath << endl;
    return false;
  }

  for (ZSynapseAnnotationArray::const_iterator iter = begin();
       iter != end(); ++iter) {
    //Add edge
    int tbarId = iter->getTBarRef()->bodyId();
    vector<const SynapseLocation*> psdArray = iter->getPartnerRef();
    for (vector<const SynapseLocation*>::const_iterator psdIter =
         psdArray.begin(); psdIter != psdArray.end(); ++psdIter) {
      stream << tbarId << "," << (*psdIter)->bodyId() << "," << (*psdIter)->x()
             << "," << (*psdIter)->y() << "," << (*psdIter)->z() << std::endl;
    }
  }

  stream.close();

  return true;
}
