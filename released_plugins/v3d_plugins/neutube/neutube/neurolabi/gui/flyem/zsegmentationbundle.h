#ifndef ZSEGMENTATIONBUNDLE_H
#define ZSEGMENTATIONBUNDLE_H

#include <string>
#include <set>

#include "zstackfile.h"
#include "zsuperpixelmaparray.h"
#include "zsegmentmaparray.h"
#include "zgraph.h"
#include "zobject3darray.h"
#include "zjsonparser.h"

namespace FlyEm {

class ZGalaPredictionEntry {
public:
  ZGalaPredictionEntry();

  inline bool isPreserved() const { return m_preserve; }
  inline bool isFalseEdge() const { return m_falseEdge; }
  inline int getFirstNode() const { return m_node1; }
  inline int getSecondNode() const { return m_node2; }
  inline double getWeight() const { return m_weight; }

  void loadJsonObject(json_t *obj);
  void print();

private:
  bool m_preserve;
  bool m_falseEdge;
  int m_size1;
  int m_size2;
  double m_weight;
  int m_node1;
  int m_node2;
  int m_location[3];
};

class ZSegmentationBundle
{
public:
  ZSegmentationBundle();
  ~ZSegmentationBundle();

  /*
  enum EDataType {
    GREY_SCALE, SUPERPIXEL, SUPERPIXEL_MAP, ANY_DATA
  };
*/
#define setGrayScaleSource setGreyScaleSource

  void setGreyScaleSource(const ZStackFile &file);
  void setSuperpixelSource(const ZStackFile &file);
  void setSegmentMapSource(const std::string &filePath);
  void setSuperpixelMapSource(const std::string &filePath);

  bool isGreyScaleTag(const std::string &tag);
  bool isSuperpixelTag(const std::string &tag);
  bool isSuperpixelMapTag(const std::string &tag);
  bool isSegmentMapTag(const std::string &tag);

  void importJsonFile(const std::string &filePath);

  //void update(EDataType type = ANY_DATA);

  ZStack* getGreyScaleStack();
  //It won't update the data
  ZStack*& getGreyScaleStackRef();
  ZStack* getSuperpixelStack();
  ZSuperpixelMapArray* getSuperpixelMap();

  ZStack* getBodyStack();
  //It won't update the data
  ZStack*& getBodyStackRef();
  ZStack* getBodyBoundaryStack();
  ZStack* getPixelClassfication();

  ZStack* deliverGreyScaleStack();
  ZStack* deliverBodyStack();
  ZGraph* deliverBodyGraph();

  void print();

  ZGraph* getBodyGraph();

  std::vector<int>* getBodyList();
  std::map<int, size_t>* getBodyIndexMap();

  std::set<int> getAllOrphanBody();

  ZObject3dArray* getBodyObject();
  ZObject3d* getBodyObject(int id);
  ZObject3dArray* getBodyBoundaryObject();
  ZObject3dArray* getBodyBorderObject();

  ZObject3d* getBodyBoundaryObject(int id);

  ZObject3d* getBodyBorderObject(int id1, int id2);

  const std::vector<double>& getGalaProbability();
  double getGalaProbability(int id1, int id2);
  double getBoundaryProbability(int id1, int id2);
  double getMitoProbability(int id);
  double getCytoProbability(int id);

  int getBodySize(int id);
  bool isImportantBorder(int id1, int id2);

  std::vector<int>& getGroundLabel();
  void loadGroundLabel();
  void saveGroundLabel();
  std::string getGroundLabelPath();

  void trimSuperpixelMap();
  void compressBodyId(int startId = 1);

  std::string getHostDir();
  std::string getGalaSegmentationPath();

public:
  enum EComponent {
    GREY_SCALE_STACK, SUPERPIXEL_STACK, SUPERPIXEL_MAP,
    BODY_STACK, BODY_BOUNDARY_STACK, BODY_GRAPH, BODY_LIST, BODY_INDEX_MAP,
    BODY_OBJECT, BODY_BOUNDARY_OBJECT, BODY_BORDER_OBJECT, GALA_PREDICTION,
    GROUND_LABEL, PIXEL_CLASSIFICATION, ALL_COMPONENT
  };

  /*
  void deprecateBodyStack();
  void deprecateBodyBoundaryStack();
  void deprecateBodyGraph();
  void deprecateBodyList();
  void deprecateBodyIndexMap();
  void deprecateGreyScaleStack();
  void deprecateSuperpixelStack();

  bool isBodyStackDeprecated();
  bool isBodyBoundaryStackDeprecated();
  bool isBodyGraphDeprecated();
  bool isBodyListDeprecated();
  bool isBodyIndexMapDeprecated();
*/
  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);
  bool isDeprecated(EComponent component) const;

  inline std::string source() { return m_source; }
  std::string getPixelClassficationPath();
  std::string getBcfPath();
  std::string getBcfPath(const std::string &setName);

private:
  ZStackFile m_greyScaleFile;
  ZStackFile m_superpixelFile;
  std::string m_superpixelMapPath;
  std::string m_segmentMapPath;

  ZStack *m_greyScaleStack;
  ZStack *m_superpixelStack;
  ZSuperpixelMapArray *m_superpixelMap;

  ZStack *m_bodyStack;
  ZStack *m_bodyBoundaryStack;
  ZGraph *m_bodyGraph;

  std::vector<int> *m_bodyList;
  std::map<int, size_t> *m_bodyIndexMap;

  ZObject3dArray *m_bodyObject;
  ZObject3dArray *m_bodyBoundaryObject;
  ZObject3dArray *m_bodyBorderObject;

  std::vector<int> m_groundLabel;
  std::vector<double> m_galaProbability;

  ZStack *m_pixelClassification;

  std::string m_source;
};
}
#endif // ZSEGMENTATIONBUNDLE_H
