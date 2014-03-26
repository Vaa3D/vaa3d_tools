#include "zsegmentationbundle.h"

#include <iostream>

#include "tz_stack_neighborhood.h"
#include "zjsonparser.h"
#include "zstring.h"
#include "flyem/zsegmentationanalyzer.h"
#include "tz_error.h"
#include "tz_stack_utils.h"
#include "tz_iarray.h"

using namespace std;

FlyEm::ZGalaPredictionEntry::ZGalaPredictionEntry() :
  m_preserve(false), m_falseEdge(false), m_size1(0), m_size2(0),
  m_weight(0.0), m_node1(-1), m_node2(-1)
{
  m_location[0] = 0;
  m_location[1] = 0;
  m_location[2] = 0;
}

void FlyEm::ZGalaPredictionEntry::loadJsonObject(json_t *obj)
{
  const char *key;
  json_t *value;
  json_object_foreach(obj, key, value) {
    if (eqstr(key, "preserve")) {
      m_preserve = ZJsonParser::booleanValue(value);
    } else if (eqstr(key, "false_edge")) {
      m_falseEdge = ZJsonParser::booleanValue(value);
    } else if (eqstr(key, "weight")) {
      m_weight = ZJsonParser::numberValue(value);
    } else if (eqstr(key, "size1")) {
      m_size1 = ZJsonParser::integerValue(value);
    } else if (eqstr(key, "size2")) {
      m_size2 = ZJsonParser::integerValue(value);
    } else if (eqstr(key, "node1")) {
      m_node1 = ZJsonParser::integerValue(value);
    } else if (eqstr(key, "node2")) {
      m_node2 = ZJsonParser::integerValue(value);
    } else if (eqstr(key, "location")) {
      for (int k = 0; k < 3; ++k) {
        m_location[k] = ZJsonParser::integerValue(value, k);
      }
    }
  }
}

void FlyEm::ZGalaPredictionEntry::print()
{
  cout << getFirstNode() << " - " << getSecondNode() << ": " << getWeight()
       << endl;
}

FlyEm::ZSegmentationBundle::ZSegmentationBundle() :
  m_greyScaleStack(NULL), m_superpixelStack(NULL), m_superpixelMap(NULL),
  m_bodyStack(NULL),
  m_bodyBoundaryStack(NULL), m_bodyGraph(NULL), m_bodyList(NULL),
  m_bodyIndexMap(NULL), m_bodyObject(NULL), m_bodyBoundaryObject(NULL),
  m_bodyBorderObject(NULL), m_pixelClassification(NULL)
{
}

FlyEm::ZSegmentationBundle::~ZSegmentationBundle()
{
  delete m_greyScaleStack;
  delete m_superpixelStack;
  delete m_superpixelMap;
  delete m_bodyStack;
  delete m_bodyBoundaryStack;
  delete m_bodyGraph;
  delete m_bodyList;
  delete m_bodyIndexMap;
  delete m_bodyObject;
  delete m_bodyBoundaryObject;
  delete m_bodyBorderObject;
}

void FlyEm::ZSegmentationBundle::setGreyScaleSource(const ZStackFile &file)
{
  deprecate(GREY_SCALE_STACK);
  m_greyScaleFile = file;
}

void FlyEm::ZSegmentationBundle::setSuperpixelSource(const ZStackFile &file)
{
  deprecate(SUPERPIXEL_STACK);
  m_superpixelFile = file;
}

void FlyEm::ZSegmentationBundle::setSuperpixelMapSource(
    const std::string &filePath)
{
  deprecate(SUPERPIXEL_MAP);
  m_superpixelMapPath = filePath;
}

void FlyEm::ZSegmentationBundle::setSegmentMapSource(
    const std::string &filePath)
{
  deprecate(SUPERPIXEL_MAP);
  m_segmentMapPath = filePath;
}

ZStack* FlyEm::ZSegmentationBundle::getGreyScaleStack()
{
  if (isDeprecated(GREY_SCALE_STACK)) {
    m_greyScaleStack = m_greyScaleFile.readStack();
  }

  return m_greyScaleStack;
}

ZStack*& FlyEm::ZSegmentationBundle::getGreyScaleStackRef()
{
  return m_greyScaleStack;
}

ZStack* FlyEm::ZSegmentationBundle::getSuperpixelStack()
{
  if (isDeprecated(SUPERPIXEL_STACK)) {
    m_superpixelStack = m_superpixelFile.readStack();
  }

  return m_superpixelStack;
}

ZSuperpixelMapArray* FlyEm::ZSegmentationBundle::getSuperpixelMap()
{
  if (isDeprecated(SUPERPIXEL_MAP)) {
    m_superpixelMap = new ZSuperpixelMapArray;
    m_superpixelMap->load(m_superpixelMapPath);
    ZIntMap segmentMap;
    segmentMap.load(m_segmentMapPath);
    m_superpixelMap->setBodyId(segmentMap);
  }

  return m_superpixelMap;
}

void FlyEm::ZSegmentationBundle::trimSuperpixelMap()
{
  ZSuperpixelMapArray *superpixelMap =  getSuperpixelMap();
  if (superpixelMap != NULL) {
    ZStack *superpixelStack = getSuperpixelStack();
    if (superpixelStack != NULL) {
      TZ_ASSERT(superpixelStack->kind() == GREY, "Unsupported stack kind");

      int minPlaneId = superpixelMap->minPlaneId();

      size_t planeVoxelNumber =
          superpixelStack->getVoxelNumber(ZStack::SINGLE_PLANE);
      size_t planeByteNumber =
          superpixelStack->getByteNumber(ZStack::SINGLE_PLANE);
      size_t voxelNumber = superpixelStack->getVoxelNumber(ZStack::WHOLE_DATA);

      vector<set<int> > superpixelSet(superpixelStack->depth());

      for (int z = 0; z < superpixelStack->depth(); ++z) {
        for (size_t voxelIndex = 0; voxelIndex < planeVoxelNumber; ++voxelIndex) {
          uint8_t *codeArray = superpixelStack->array8() + z * planeByteNumber;
          int superpixelId = FlyEm::ZSegmentationAnalyzer::channelCodeToId(
                codeArray, superpixelStack->channelNumber(), voxelIndex,
                voxelNumber);
          superpixelSet[z].insert(superpixelSet[z].end(), superpixelId);
        }
      }

      vector<bool> existed(superpixelMap->size(), false);

      size_t count = 0;
      for (size_t i = 0; i < superpixelMap->size(); ++i) {
        if (superpixelSet[(*superpixelMap)[i].planeId() - minPlaneId].
            count((*superpixelMap)[i].superpixelId()) > 0) {
          existed[i] = true;
          ++count;
        }
      }

      if (count < superpixelMap->size() - 1) {
        if (count == 0) {
          TZ_WARN(ERROR_DATA_VALUE);
          superpixelMap->clear();
        } else {
          size_t lastIndex = 0;
          for (size_t i = 0; i < existed.size(); ++i) {
            if (existed[i]) {
              (*superpixelMap)[lastIndex++] = (*superpixelMap)[i];
            }
          }
          TZ_ASSERT(lastIndex == count, "Inconsistent value.");
          superpixelMap->resize(count);
        }
        deprecateDependent(SUPERPIXEL_MAP);
      }
    }
  }
}

void FlyEm::ZSegmentationBundle::compressBodyId(int startId)
{
  ZSuperpixelMapArray *superpixelMap =  getSuperpixelMap();
  if (superpixelMap != NULL) {
    superpixelMap->compressBodyId(startId);
    deprecateDependent(SUPERPIXEL_MAP);
  }
}

bool FlyEm::ZSegmentationBundle::isDeprecated(EComponent component) const
{
  bool state = false;

  switch (component) {
  case GREY_SCALE_STACK:
    state = m_greyScaleStack == NULL;
    break;
  case SUPERPIXEL_STACK:
    state = m_superpixelStack == NULL;
    break;
  case SUPERPIXEL_MAP:
    state = m_superpixelMap == NULL;
    break;
  case BODY_STACK:
    state = m_bodyStack == NULL;
    break;
  case BODY_BOUNDARY_STACK:
    state = m_bodyBoundaryStack == NULL;
    break;
  case BODY_GRAPH:
    state = m_bodyGraph == NULL;
    break;
  case BODY_LIST:
    state = m_bodyList == NULL;
    break;
  case BODY_INDEX_MAP:
    state = m_bodyIndexMap == NULL;
    break;
  case BODY_OBJECT:
    state = m_bodyObject == NULL;
    break;
  case BODY_BOUNDARY_OBJECT:
    state = m_bodyBoundaryObject == NULL;
    break;
  case BODY_BORDER_OBJECT:
    state = m_bodyBorderObject == NULL;
    break;
  case GALA_PREDICTION:
    state = m_galaProbability.empty();
    break;
  case GROUND_LABEL:
    state = m_groundLabel.empty();
    break;
  case PIXEL_CLASSIFICATION:
    state = m_pixelClassification == NULL;
    break;
  default:
    break;
  }

  return state;
}

void FlyEm::ZSegmentationBundle::deprecateDependent(EComponent component)
{
  switch (component) {
  case GREY_SCALE_STACK:
    break;
  case SUPERPIXEL_STACK:
    deprecate(BODY_STACK);
    break;
  case SUPERPIXEL_MAP:
    deprecate(BODY_STACK);
    break;
  case BODY_STACK:
    deprecate(BODY_BOUNDARY_STACK);
    deprecate(BODY_GRAPH);
    deprecate(BODY_LIST);
    deprecate(BODY_OBJECT);
    deprecate(BODY_BOUNDARY_OBJECT);
    deprecate(BODY_BORDER_OBJECT);
    deprecate(GALA_PREDICTION);
    deprecate(GROUND_LABEL);
    break;
  case BODY_BOUNDARY_STACK:
    break;
  case BODY_GRAPH:
    break;
  case BODY_LIST:
    deprecate(BODY_INDEX_MAP);
    break;
  case BODY_INDEX_MAP:
    break;
  case ALL_COMPONENT:
    deprecate(GREY_SCALE_STACK);
    deprecate(SUPERPIXEL_STACK);
    deprecate(SUPERPIXEL_MAP);
    deprecate(PIXEL_CLASSIFICATION);
    break;
  default:
    break;
  }
}

void FlyEm::ZSegmentationBundle::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case GREY_SCALE_STACK:
    delete m_greyScaleStack;
    m_greyScaleStack = NULL;
    break;
  case SUPERPIXEL_STACK:
    delete m_superpixelStack;
    m_superpixelStack = NULL;
    break;
  case SUPERPIXEL_MAP:
    delete m_superpixelMap;
    m_superpixelMap = NULL;
    break;
  case BODY_STACK:
    delete m_bodyStack;
    m_bodyStack = NULL;
    break;
  case BODY_BOUNDARY_STACK:
    delete m_bodyBoundaryStack;
    m_bodyBoundaryStack = NULL;
    break;
  case BODY_GRAPH:
    delete m_bodyGraph;
    m_bodyGraph = NULL;
    break;
  case BODY_LIST:
    delete m_bodyList;
    m_bodyList = NULL;
    break;
  case BODY_INDEX_MAP:
    delete m_bodyIndexMap;
    m_bodyIndexMap = NULL;
    break;
  case BODY_OBJECT:
    delete m_bodyObject;
    m_bodyObject = NULL;
    break;
  case BODY_BOUNDARY_OBJECT:
    delete m_bodyBoundaryObject;
    m_bodyBoundaryObject = NULL;
    break;
  case BODY_BORDER_OBJECT:
    delete m_bodyBorderObject;
    m_bodyBorderObject = NULL;
    break;
  case GALA_PREDICTION:
    m_galaProbability.clear();
    break;
  case GROUND_LABEL:
    m_groundLabel.clear();
    break;
  case PIXEL_CLASSIFICATION:
    delete m_pixelClassification;
    m_pixelClassification = NULL;
    break;
  case ALL_COMPONENT:
    break;
  default:
    break;
  }
}

ZStack* FlyEm::ZSegmentationBundle::getPixelClassfication()
{
  if (m_pixelClassification == NULL) {
    string filePath = getPixelClassficationPath();
    if (fexist(filePath.c_str())) {
      cout << "Loading gala pixel output ..." << endl;
      ZStackFile file;
      file.importJsonFile(filePath);
      m_pixelClassification = file.readStack();
    }
  }

  return m_pixelClassification;
}

ZStack* FlyEm::ZSegmentationBundle::getBodyStack()
{
  if (getSuperpixelStack() == NULL) {
    cout << "FlyEm::ZSegmentationBundle::createBodyStack() failed because "
            " the superpixel stack does not exist." << endl;

    return NULL;
  }

  if (isDeprecated(BODY_STACK)) {
    cout << "Constructing body stack ..." << endl;
    ZSuperpixelMapArray *superpixelMap = getSuperpixelMap();
    if (superpixelMap == NULL) {
      cout << "FlyEm::ZSegmentationBundle::createBodyStack() failed because "
              " the superpixel map does not exist." << endl;
      return NULL;
    } else {
      m_bodyStack = superpixelMap->mapStack(*getSuperpixelStack());
    }
  }

  return m_bodyStack;
}

ZStack*& FlyEm::ZSegmentationBundle::getBodyStackRef()
{
  return m_bodyStack;
}

ZStack* FlyEm::ZSegmentationBundle::getBodyBoundaryStack()
{
  if (m_superpixelStack == NULL) {
    cout << "FlyEm::ZSegmentationBundle::getBodyBoundaryStack() failed because "
            "m_superpixelStack is NULL" << endl;
    return NULL;
  }

  if (isDeprecated(BODY_BOUNDARY_STACK)) {
    ZStack *bodyStack = getBodyStack();

    m_bodyBoundaryStack = new ZStack(GREY, bodyStack->width(),
                                      bodyStack->height(), bodyStack->depth(), 1);

    int neighborOffset[26];
    int isInBound[26];
    int nnbr = 26;
    Stack_Neighbor_Offset(nnbr, bodyStack->width(), bodyStack->height(),
                          neighborOffset);

    uint8_t *boundaryArray = m_bodyBoundaryStack->array8();

    int cwidth = bodyStack->width() - 1;
    int cheight = bodyStack->height() - 1;
    int cdepth = bodyStack->depth() - 1;
    size_t channelOffset = bodyStack->getByteNumber(ZStack::SINGLE_CHANNEL);

    size_t voxelIndex = 0;
    for (int k = 0; k <= cdepth; k++) {
      for (int j = 0; j <= cheight; j++) {
        for (int i = 0; i <= cwidth; i++) {
          int n = Stack_Neighbor_Bound_Test_S(nnbr, cwidth, cheight, cdepth,
                                              i, j, k, isInBound);
          boundaryArray[voxelIndex] = 0;
          if (n != nnbr) {
            for (int neighborIndex = 0; neighborIndex < nnbr; ++neighborIndex) {
              if (isInBound[neighborIndex]) {
                if (!bodyStack->hasSameValue(
                      voxelIndex, voxelIndex + neighborOffset[neighborIndex],
                      channelOffset)) {
                  boundaryArray[voxelIndex] = 255;
                  break;
                }
              }
            }
          } else {
            for (int neighborIndex = 0; neighborIndex < nnbr; ++neighborIndex) {
              if (!bodyStack->hasSameValue(
                    voxelIndex, voxelIndex + neighborOffset[neighborIndex],
                    channelOffset)) {
                boundaryArray[voxelIndex] = 255;
                break;
              }
            }
          }
          ++voxelIndex;
        }
      }
    }
  }

  return m_bodyBoundaryStack;
}

bool FlyEm::ZSegmentationBundle::isGreyScaleTag(const std::string &tag)
{
  return (tag == "greyscale") || (tag == "grayscale");
}

bool FlyEm::ZSegmentationBundle::isSuperpixelMapTag(const std::string &tag)
{
  return (tag == "superpixel_map") || (tag == "superpixelmap") ||
      (tag == "superpixel map");
}

bool FlyEm::ZSegmentationBundle::isSuperpixelTag(const std::string &tag)
{
  return tag == "superpixel";
}

bool FlyEm::ZSegmentationBundle::isSegmentMapTag(const std::string &tag)
{
  return tag == "segmentmap" || tag == "segment_map" || tag == "segment map";
}

void FlyEm::ZSegmentationBundle::importJsonFile(const std::string &filePath)
{
  ZJsonObject rootObject;
  rootObject.load(filePath);

  const char *key = NULL;
  json_t *value = NULL;

  json_object_foreach(rootObject.getValue(), key, value) {
    if (isGreyScaleTag(key)) {
      m_greyScaleFile.loadJsonObject(value, filePath);
    } else if (isSuperpixelTag(key)) {
      m_superpixelFile.loadJsonObject(value, filePath);
    } else if (isSuperpixelMapTag(key)) {
      ZString url = ZJsonParser::stringValue(value);
      m_superpixelMapPath =
          url.absolutePath(ZString(filePath).dirPath()).c_str();
    } else if (isSegmentMapTag(key)) {
      ZString url = ZJsonParser::stringValue(value);
      m_segmentMapPath =
          url.absolutePath(ZString(filePath).dirPath()).c_str();
    }
  }

  m_source = filePath;

  deprecate(ALL_COMPONENT);
}

ZStack* FlyEm::ZSegmentationBundle::deliverGreyScaleStack()
{
  ZStack *stack = getGreyScaleStack();

  m_greyScaleStack = NULL;

  return stack;
}

ZStack* FlyEm::ZSegmentationBundle::deliverBodyStack()
{
  ZStack *stack = getBodyStack();
  //To avoid deprecateBodyStack() deleting the returned stack
  m_bodyStack = NULL;
  //deprecateBodyStack();
  deprecate(BODY_STACK);

  return stack;
}

ZGraph* FlyEm::ZSegmentationBundle::deliverBodyGraph()
{
  ZGraph *graph = getBodyGraph();
  m_bodyGraph = NULL;
  deprecate(BODY_GRAPH);

  return graph;
}

void FlyEm::ZSegmentationBundle::print()
{
  cout << "Grey scale file:" << endl;
  m_greyScaleFile.print();

  cout << "Superpixel file:" << endl;
  m_superpixelFile.print();

  cout << "Superpixel to segment map: " << m_superpixelMapPath << endl;
  cout << "Segment to body map: " << m_segmentMapPath << endl;
}

ZGraph* FlyEm::ZSegmentationBundle::getBodyGraph()
{
  if (isDeprecated(BODY_GRAPH)) {
    m_bodyGraph  = new ZGraph(ZGraph::UNDIRECTED_WITH_WEIGHT);

    ZStack *bodyStack = getBodyStack();

    cout << "Constructing body graph ..." << endl;

    size_t volume = bodyStack->getVoxelNumber();
    int neighborOffset[26];
    int isInBound[26];

    int width = bodyStack->width();
    int height = bodyStack->height();
    int depth = bodyStack->depth();
    size_t channelOffset = bodyStack->getByteNumber(ZStack::SINGLE_CHANNEL);

    Stack_Neighbor_Offset(26, bodyStack->width(),
                          bodyStack->height(), neighborOffset);

    vector<uint8_t> channelCode1(bodyStack->channelNumber());
    vector<uint8_t> channelCode2(bodyStack->channelNumber());

    for (size_t voxelIndex = 0; voxelIndex < volume; voxelIndex++) {
      int nbound = Stack_Neighbor_Bound_Test_I(26, width, height, depth, voxelIndex,
                                               isInBound);
      for (int neighborIndex = 0; neighborIndex < 26; neighborIndex++) {
        if (nbound == 26 || isInBound[neighborIndex]) {
          size_t neighborVoxelIndex = voxelIndex + neighborOffset[neighborIndex];
          if (!bodyStack->hasSameValue(voxelIndex, neighborVoxelIndex, channelOffset)) {
            for (int c = 0; c < bodyStack->channelNumber(); ++c) {
              channelCode1[c] = *(bodyStack->array8(c) + voxelIndex);
              channelCode2[c] =
                  *(bodyStack->array8(c) + neighborVoxelIndex);
            }
            int id1 =
                FlyEm::ZSegmentationAnalyzer::channelCodeToId(channelCode1);

            int id2 =
                FlyEm::ZSegmentationAnalyzer::channelCodeToId(channelCode2);
            m_bodyGraph->addEdge(id1, id2);
          }
        }
      }
    }
  }

  return m_bodyGraph;
}

vector<int>* FlyEm::ZSegmentationBundle::getBodyList()
{
  if (isDeprecated(BODY_LIST)) {
    m_bodyList = new vector<int>;
    TZ_ASSERT(m_bodyIndexMap == NULL, "Invalid pointer.");
    m_bodyIndexMap = new map<int, size_t>;
    ZStack *bodyStack = getBodyStack();
    size_t volume = bodyStack->getVoxelNumber();
    for (size_t voxelIndex = 0; voxelIndex < volume; voxelIndex++) {
      int id = FlyEm::ZSegmentationAnalyzer::channelCodeToId(
            bodyStack->color(voxelIndex));

      if (m_bodyIndexMap->count(id) == 0) {
        (*m_bodyIndexMap)[id] = m_bodyList->size();
        m_bodyList->push_back(id);
      }
    }
  }

  return m_bodyList;
}

map<int, size_t>* FlyEm::ZSegmentationBundle::getBodyIndexMap()
{
  if (isDeprecated(BODY_INDEX_MAP)) {
    vector<int> *bodyList = getBodyList();
#ifdef _DEBUG_2
    deprecateBodyIndexMap();
#endif
    if (isDeprecated(BODY_INDEX_MAP)) { //the index map is actually created in
                                      //getBodyList() in current implementation
      for (size_t i = 0; i < bodyList->size(); ++i) {
        (*m_bodyIndexMap)[(*bodyList)[i]] = i;
      }
    }
  }

  return m_bodyIndexMap;
}

set<int> FlyEm::ZSegmentationBundle::getAllOrphanBody()
{
  set<int> orphanSet;

  vector<int> *bodyList = getBodyList();
  map<int, size_t> *bodyIndexMap = getBodyIndexMap();
  vector<bool> isEdgeBody(bodyList->size(), false);

  ZStack *bodyStack = getBodyStack();
  int x = 0;
  int y = 0;
  int z = 0;
  int width = bodyStack->width();
  int height = bodyStack->height();
  int depth = bodyStack->depth();
  int area = width * height;
  int volume = area * depth;

  size_t voxelIndex = 0;
  for (int k = 0; k < 2; ++k) {
    for (z = 0; z < depth; ++z) {
      for (y = 0; y < height; ++y) {
        int channelId = FlyEm::ZSegmentationAnalyzer::channelCodeToId(
              bodyStack->color(voxelIndex));
        isEdgeBody[(*bodyIndexMap)[channelId]] = true;

#ifdef _DEBUG_
        int tmpX, tmpY, tmpZ;
        Stack_Util_Coord(voxelIndex, width, height, &tmpX, &tmpY, &tmpZ);
        if (k == 0 && tmpX != 0) {
          cout << "Inconsistent edge index: " << voxelIndex << endl;
        }
        if (k == 1 && tmpX != width - 1) {
          cout << "Inconsistent edge index: " << voxelIndex << endl;
        }
#endif

        voxelIndex += width;
      }
    }
    voxelIndex = width - 1;
  }

  voxelIndex = 0;
  for (int k = 0; k < 2; ++k) {
    for (z = 0; z < depth; ++z) {
      for (x = 0; x < height; ++x) {
        int channelId = FlyEm::ZSegmentationAnalyzer::channelCodeToId(
              bodyStack->color(voxelIndex));
        isEdgeBody[(*bodyIndexMap)[channelId]] = true;
#ifdef _DEBUG_
        int tmpX, tmpY, tmpZ;
        Stack_Util_Coord(voxelIndex, width, height, &tmpX, &tmpY, &tmpZ);
        if (k == 0 && tmpY != 0) {
          cout << "Inconsistent edge index: " << voxelIndex << endl;
        }
        if (k == 1 && tmpY != height - 1) {
          cout << "Inconsistent edge index: " << voxelIndex;
        }
#endif
        ++voxelIndex;
      }
      voxelIndex += area - width;
    }
    voxelIndex = area - width;
  }

  voxelIndex = 0;
  for (int k = 0; k < 2; ++k) {
    for (y = 0; y < depth; ++y) {
      for (x = 0; x < height; ++x) {
        int channelId = FlyEm::ZSegmentationAnalyzer::channelCodeToId(
              bodyStack->color(voxelIndex));
        isEdgeBody[(*bodyIndexMap)[channelId]] = true;
#ifdef _DEBUG_
        int tmpX, tmpY, tmpZ;
        Stack_Util_Coord(voxelIndex, width, height, &tmpX, &tmpY, &tmpZ);
        if (k == 0 && tmpZ != 0) {
          cout << "Inconsistent edge index: " << voxelIndex << endl;
        }
        if (k == 1 && tmpZ != depth - 1) {
          cout << "Inconsistent edge index: " << voxelIndex;
        }
#endif
        ++voxelIndex;
      }
    }
    voxelIndex = volume - area;
  }

  for (size_t i = 0; i < isEdgeBody.size(); ++i) {
    if (!isEdgeBody[i]) {
      orphanSet.insert(orphanSet.end(), (*bodyList)[i]);
    }
  }

  return orphanSet;
}

ZObject3d* FlyEm::ZSegmentationBundle::getBodyObject(int id)
{
  map<int, size_t> *bodyIndexMap = getBodyIndexMap();
  ZObject3dArray *objectArray = getBodyObject();

  if (objectArray == NULL) {
    return NULL;
  }

  if (bodyIndexMap->count(id) == 0) {
    return NULL;
  }

  return (*objectArray)[(*bodyIndexMap)[id]];
}

ZObject3d* FlyEm::ZSegmentationBundle::getBodyBoundaryObject(int id)
{
  map<int, size_t> *bodyIndexMap = getBodyIndexMap();
  ZObject3dArray *objectArray = getBodyBoundaryObject();

  if (objectArray == NULL) {
    return NULL;
  }

  if (bodyIndexMap->count(id) == 0) {
    return NULL;
  }

  return (*objectArray)[(*bodyIndexMap)[id]];
}

ZObject3dArray* FlyEm::ZSegmentationBundle::getBodyObject()
{
  if (isDeprecated(BODY_OBJECT)) {
    cout << "Constructing body objects ..." << endl;
    m_bodyObject = new ZObject3dArray;
    ZStack *bodyStack = getBodyStack();

    map<int, size_t> *bodyIndexMap = getBodyIndexMap();
    m_bodyObject->resize(bodyIndexMap->size(), NULL);

    int width = bodyStack->width();
    int height = bodyStack->height();
    int depth = bodyStack->depth();
    size_t voxelIndex = 0;

    m_bodyObject->setSourceSize(width, height, depth);

    for (int z = 0; z < depth; ++z) {
      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          int id = FlyEm::ZSegmentationAnalyzer::channelCodeToId(
                bodyStack->color(voxelIndex));
          size_t objectIndex = (*bodyIndexMap)[id];

          if ((*m_bodyObject)[objectIndex] == NULL) {
            (*m_bodyObject)[objectIndex] = new ZObject3d;
          }

          (*m_bodyObject)[objectIndex]->append(x, y, z);

          ++voxelIndex;
        }
      }
    }
  }

  return m_bodyObject;
}

ZObject3dArray* FlyEm::ZSegmentationBundle::getBodyBoundaryObject()
{
  if (isDeprecated(BODY_BOUNDARY_OBJECT)) {
    cout << "Constructing body boundaries ..." << endl;
    m_bodyBoundaryObject = new ZObject3dArray;
    ZStack *bodyStack = getBodyStack();
    ZStack *bodyBoundaryStack = getBodyBoundaryStack();

    map<int, size_t> *bodyIndexMap = getBodyIndexMap();
    m_bodyBoundaryObject->resize(bodyIndexMap->size(), NULL);

    int width = bodyStack->width();
    int height = bodyStack->height();
    int depth = bodyStack->depth();
    size_t voxelIndex = 0;

    m_bodyBoundaryObject->setSourceSize(width, height, depth);

    for (int z = 0; z < depth; ++z) {
      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          int id = FlyEm::ZSegmentationAnalyzer::channelCodeToId(
                bodyStack->color(voxelIndex));
          int objectIndex = (*bodyIndexMap)[id];

          if ((*m_bodyBoundaryObject)[objectIndex] == NULL) {
            (*m_bodyBoundaryObject)[objectIndex] = new ZObject3d;
          }

          if (bodyBoundaryStack->value8(voxelIndex) > 0) {
            (*m_bodyBoundaryObject)[objectIndex]->append(x, y, z);
          }

          ++voxelIndex;
        }
      }
    }
  }

  return m_bodyBoundaryObject;
}

ZObject3d* FlyEm::ZSegmentationBundle::getBodyBorderObject(int id1, int id2)
{
  ZObject3dArray *objArray =  getBodyBorderObject();
  ZGraph *bodyGraph = getBodyGraph();
  int index = bodyGraph->getEdgeIndex(id1, id2);
  if (index >= 0) {
    return (*objArray)[index];
  }

  return NULL;

  /*
  ZObject3d *border = NULL;

  map<int, size_t> *bodyIndexMap = getBodyIndexMap();

  if (bodyIndexMap->count(id1) != 0 || bodyIndexMap->count(id2) != 0) {
    ZObject3dArray *boundaryObjectArray = getBodyBoundaryObject();
    int index1 = (*bodyIndexMap)[id1];
    int index2 = (*bodyIndexMap)[id2];

    ZObject3d *obj1 = (*boundaryObjectArray)[index1];
    ZObject3d *obj2 = (*boundaryObjectArray)[index2];

    ZObject3d *mobj = obj1;
    int sid = id2;
    if (obj1->size() > obj2->size()) {
      mobj = obj2;
      sid = id1;
    }

    ZStack *bodyStack = getBodyStack();

    int nnbr = 26;
    int neighborOffset[26];
    int isInBound[26];
    int width = bodyStack->width();
    int height = bodyStack->height();
    int depth = bodyStack->depth();
    Stack_Neighbor_Offset(nnbr, width, height, neighborOffset);

    vector<size_t> voxelIndexArray = mobj->toIndexArray(
          bodyStack->width(), bodyStack->height(), bodyStack->depth(),
          0, 0, 0);

    border = new ZObject3d;
    for (size_t i = 0; i < voxelIndexArray.size(); ++i) {
      size_t voxelIndex = voxelIndexArray[i];

      int nInBound = Stack_Neighbor_Bound_Test_I(nnbr, width, height,
                                                 depth, voxelIndex, isInBound);
      bool isValid = false;
      for (int neighborIndex = 0; neighborIndex < nnbr; ++neighborIndex) {
        if (nInBound == nnbr || isInBound[neighborIndex]) {
          size_t neighborVoxelIndex = voxelIndex + neighborOffset[neighborIndex];
          if (FlyEm::ZSegmentationAnalyzer::
              channelCodeToId(bodyStack->color(neighborVoxelIndex)) == sid) {
            int x, y, z;
            Stack_Util_Coord(neighborVoxelIndex, width, height, &x, &y, &z);
            border->append(x, y, z);
            isValid = true;
          }
        }
      }
      if (isValid) {
        border->append(mobj->x(i), mobj->y(i), mobj->z(i));
      }
    }
  }

  return border;
  */
}

ZObject3dArray* FlyEm::ZSegmentationBundle::getBodyBorderObject()
{
  if (isDeprecated(BODY_BORDER_OBJECT)) {
    cout << "Constructing body borders ..." << endl;

    ZGraph *graph = getBodyGraph();
    m_bodyBorderObject = new ZObject3dArray;
    m_bodyBorderObject->resize(graph->size(), NULL);

    ZStack *bodyStack = getBodyStack();

    size_t volume = bodyStack->getVoxelNumber();
    int neighborOffset[26];
    int isInBound[26];

    int width = bodyStack->width();
    int height = bodyStack->height();
    int depth = bodyStack->depth();

    Stack_Neighbor_Offset(26, bodyStack->width(),
                          bodyStack->height(), neighborOffset);

    vector<uint8_t> channelCode1(bodyStack->channelNumber());
    vector<uint8_t> channelCode2(bodyStack->channelNumber());
    size_t channelOffset = bodyStack->getByteNumber(ZStack::SINGLE_CHANNEL);

    for (size_t voxelIndex = 0; voxelIndex < volume; voxelIndex++) {
      int nbound = Stack_Neighbor_Bound_Test_I(26, width, height, depth, voxelIndex,
                                               isInBound);
      int x, y, z;
      Stack_Util_Coord(voxelIndex, width, height, &x, &y, &z);
      for (int neighborIndex = 0; neighborIndex < 26; neighborIndex++) {
        if (nbound == 26 || isInBound[neighborIndex]) {
          size_t neighborVoxelIndex = voxelIndex + neighborOffset[neighborIndex];
          if (!bodyStack->hasSameValue(voxelIndex, neighborVoxelIndex, channelOffset)) {
            for (int c = 0; c < bodyStack->channelNumber(); ++c) {
              channelCode1[c] = *(bodyStack->array8(c) + voxelIndex);
              channelCode2[c] =
                  *(bodyStack->array8(c) + neighborVoxelIndex);
            }
            int id1 =
                FlyEm::ZSegmentationAnalyzer::channelCodeToId(channelCode1);
            int id2 =
                FlyEm::ZSegmentationAnalyzer::channelCodeToId(channelCode2);
            size_t edgeIndex = graph->getEdgeIndex(id1, id2);

            if ((*m_bodyBorderObject)[edgeIndex] == NULL) {
              (*m_bodyBorderObject)[edgeIndex] = new ZObject3d;
            }

            if ((*m_bodyBorderObject)[edgeIndex]->size() > 0) {
              if ((*m_bodyBorderObject)[edgeIndex]->lastX() == x &&
                  (*m_bodyBorderObject)[edgeIndex]->lastY() == y &&
                  (*m_bodyBorderObject)[edgeIndex]->lastZ() == z) {
                continue;
              }
            }

            (*m_bodyBorderObject)[edgeIndex]->append(x, y, z);

            //m_bodyGraph->addEdge(id1, id2);
          }
        }
      }
    }
  }

  return m_bodyBorderObject;
}

string FlyEm::ZSegmentationBundle::getHostDir()
{
  ZString str(m_superpixelMapPath);

  return str.dirPath();
}

string FlyEm::ZSegmentationBundle::getGalaSegmentationPath()
{
  return getHostDir() + "/gala-segmentation/seg_data/agglom-0-graphv1.json";
}

string FlyEm::ZSegmentationBundle::getBcfPath()
{
  return getHostDir() + "/Bcf";
}

string FlyEm::ZSegmentationBundle::getBcfPath(const string &setName)
{
  return getBcfPath() + "/" + setName + ".h5";
}

string FlyEm::ZSegmentationBundle::getPixelClassficationPath()
{
  return getHostDir() + "/gala-pixel-output.json";
}

const vector<double>& FlyEm::ZSegmentationBundle::getGalaProbability()
{
  if (isDeprecated(GALA_PREDICTION)) {
    ZGraph *bodyGraph = getBodyGraph();

    ZJsonObject rootObject;
    rootObject.load(getGalaSegmentationPath());

    const char *key;
    json_t *value;

    ZJsonObject_foreach(rootObject, key, value) {
      if (eqstr(key, "edge_list")) {
        size_t edgeNumber = ZJsonParser::arraySize(value);
        m_galaProbability.resize(bodyGraph->size(), 0.0);
        for (size_t i = 0; i < edgeNumber; ++i) {
          json_t *edge = ZJsonParser::arrayValue(value, i);
          ZGalaPredictionEntry entry;
          entry.loadJsonObject(edge);

          int edgeIndex = bodyGraph->getEdgeIndex(
                entry.getFirstNode(), entry.getSecondNode());
          if (edgeIndex < 0) {
            cout << "Invalid edge: " << entry.getFirstNode() << " "
                 << entry.getSecondNode() << endl;
          }
          m_galaProbability[edgeIndex] = entry.getWeight();
        }
      }
    }
  }

  return m_galaProbability;
}

double FlyEm::ZSegmentationBundle::getGalaProbability(int id1, int id2)
{
  ZGraph *bodyGraph = getBodyGraph();

  const std::vector<double>& gala = getGalaProbability();
  int edgeIndex = bodyGraph->getEdgeIndex(id1, id2);

  return gala[edgeIndex];
}

double FlyEm::ZSegmentationBundle::getBoundaryProbability(int id1, int id2)
{
  ZStack *stack = getPixelClassfication();
  double intensity = 0.0;

  if (stack != NULL) {
    ZObject3d *border = getBodyBorderObject(id1, id2);

    intensity = border->averageIntensity(stack->c_stack());
  }

  return intensity;
}

double FlyEm::ZSegmentationBundle::getMitoProbability(int id)
{
  ZStack *stack = getPixelClassfication();
  double intensity = 0.0;

  if (stack != NULL) {
    ZObject3d *obj = getBodyObject(id);

    intensity = obj->averageIntensity(stack->c_stack(2));
  }

  return intensity;
}

double FlyEm::ZSegmentationBundle::getCytoProbability(int id)
{
  ZStack *stack = getPixelClassfication();
  double intensity = 0.0;

  if (stack != NULL) {
    ZObject3d *obj = getBodyObject(id);

    intensity = obj->averageIntensity(stack->c_stack(1));
  }

  return intensity;
}

int FlyEm::ZSegmentationBundle::getBodySize(int id)
{
  return getBodyObject(id)->size();
}

bool FlyEm::ZSegmentationBundle::isImportantBorder(int id1, int id2)
{
  const int bodySizeThresh = 5000;

  bool isImportant = false;

  if (getBodySize(id1) > bodySizeThresh && getBodySize(id2) > bodySizeThresh) {
    if (getCytoProbability(id1) > getMitoProbability(id1) &&
        getCytoProbability(id2) > getMitoProbability(id2)) {
      isImportant = true;
    }
  }

  return isImportant;
}

vector<int>& FlyEm::ZSegmentationBundle::getGroundLabel()
{
  if (isDeprecated(GROUND_LABEL)) {
    loadGroundLabel();
  }

  return m_groundLabel;
}

string FlyEm::ZSegmentationBundle::getGroundLabelPath()
{
  return getHostDir() + "/body_connection_label.txt";
}

void FlyEm::ZSegmentationBundle::loadGroundLabel()
{
  FILE *fp  = fopen(getGroundLabelPath().c_str(), "r");

  if (fp != NULL) {
    deprecate(GROUND_LABEL);

    TZ_ASSERT(m_groundLabel.empty(), "The array must be empty.");

    ZString str;
    while (str.readLine(fp)) {
      if (str.containsDigit()) {
        m_groundLabel.push_back(str.firstInteger());
      }
    }

    fclose(fp);
  } else {
    cout << "Cannot load " << getGroundLabelPath() << endl;
  }
}

void FlyEm::ZSegmentationBundle::saveGroundLabel()
{
  FILE *fp  = fopen(getGroundLabelPath().c_str(), "w");

  if (fp != NULL) {
    for (size_t i = 0; i < m_groundLabel.size(); ++i) {
      fprintf(fp, "%d\n", m_groundLabel[i]);
    }
    fclose(fp);
  } else {
    cout << "Cannot open " << getGroundLabelPath() << endl;
  }
}
