#ifndef ZSYNAPSEANNOTATION_H
#define ZSYNAPSEANNOTATION_H

#include <string>
#include <vector>
#include <iostream>

#include "neurolabi_config.h"
#include "tz_json.h"
#include "zpoint.h"
#include "tz_swc_tree.h"

namespace FlyEm {

struct SynapseAnnotationConfig {
  int width;
  int height;
  int depth;
  int startNumber;
  int swcDownsample1;
  int swcDownsample2;
  double xResolution;
  double yResolution;
  double zResolution;
  double sizeScale;
  ZPoint offset;
  std::string unit;

  SynapseAnnotationConfig() : width(1024), height(1024), depth(100),
    startNumber(0), swcDownsample1(5), swcDownsample2(2),
    xResolution(1), yResolution(1), zResolution(1), sizeScale(1.0),
    unit("nm") {}

  void print() {
    std::cout << "Size: " << width << " x " << height << " x " << depth << std::endl;
    std::cout << "Start number: " << startNumber << std::endl;
    std::cout << "Downsample rate: " << swcDownsample1 << " " << swcDownsample2
              << std::endl;
    std::cout << "Resolution: " << xResolution << " x " << yResolution << " x "
              << zResolution << " " << unit << std::endl;
  }
};

struct SwcLocation {
  std::string swcFilePath;
  int bodyId;
  int nodeId;
  double distance;

  SwcLocation() : bodyId(-1), nodeId(-1), distance(0.0) {}
};

class SynapseLocation {

public:
  enum EType{
    TBAR, PARTNER, UNKNOWN_TYPE
  };

  enum ELocationSpace {
    CURRENT_SPACE, IMAGE_SPACE, ORIGINAL_SWC_SPACE, ADJUSTED_SWC_SPACE,
    PHYSICAL_SPACE
  };

public:
  SynapseLocation(EType type = UNKNOWN_TYPE);
  SynapseLocation(EType type, int x, int y, int z, int bodyId,
                  double confidence,
                  const std::string &status, bool multi = false);

  void set(EType type, int x, int y, int z, int bodyId, double confidence,
      const std::string &status, bool multi = false);

  void loadJsonObject(json_t *object, EType type);

  inline bool isTBar() const { return m_type == TBAR; }
  inline bool isPartner() const { return m_type == PARTNER; }

  std::string toString(int indent = 0) const;

  ZPoint mapPosition(const SynapseAnnotationConfig &config,
                     ELocationSpace spaceOption) const;

  Swc_Tree_Node *findSwcNode(Swc_Tree *tree,
                             const SynapseAnnotationConfig &config,
                             ELocationSpace spaceOption = ADJUSTED_SWC_SPACE);

  void setSwcLocation(const std::string &filePath, int bodyId, int nodeId,
                      double distance);
  inline SwcLocation* getSwcLocationRef() { return &m_swcLocation; }

  inline int getBodyId() const { return m_bodyId; }
  inline int x() const { return m_x; }
  inline int y() const { return m_y; }
  inline int z() const { return m_z; }
  inline int bodyId() const { return m_bodyId; }
  inline ZPoint pos() const { return ZPoint(m_x, m_y, m_z); }
  inline double confidence() const { return m_confidence; }
  bool isMulti() const { return m_multi; }

  void print() const;

  /*!
   * \brief Convert the coodindates of the synapse location from Raveler space
   *        to normal image space.
   *
   * \param startZ The start value of Z in Raveler. startZ is converted to 0
   *        in the imaeg space.
   * \param height Height of the source image to flip Y.
   */
  void convertRavelerToImageSpace(int startZ, int height);

private:
  EType m_type;
  int m_x;
  int m_y;
  int m_z;
  int m_bodyId;

  std::string m_status;
  std::string m_uid;
  double m_confidence;
  bool m_multi;

  SwcLocation m_swcLocation;
};

class ZSynapseAnnotation
{
public:
  ZSynapseAnnotation();

public:
  void setTBar(int x, int y, int z, int bodyId, double confidence,
          std::string status, bool multi = false);
  void addPartner(int x, int y, int z, int bodyId, double confidence,
             const std::string &status);

  bool loadJsonObject(json_t *object);

  std::string toString(int indent = 0,
                       const std::vector<int> *selectedPartner = NULL) const;

  SynapseLocation* getSynapseLocationRef(size_t index);
  const SynapseLocation* getSynapseLocationRef(size_t index) const;

  SynapseLocation& operator[] (size_t index);

  inline std::vector<SynapseLocation>* getPartnerArrayRef() {
    return &m_partnerArray;
  }
  inline const std::vector<SynapseLocation>* getPartnerArrayRef() const {
    return &m_partnerArray;
  }

  inline SynapseLocation* getTBarRef() {
    return &m_tBar;
  }

  inline const SynapseLocation* getTBarRef() const {
    return &m_tBar;
  }

  inline const SynapseLocation* getPartnerRef(int index) const {
    return &(m_partnerArray[index]);
  }

  inline SynapseLocation* getPartnerRef(int index) {
    return &(m_partnerArray[index]);
  }

  std::vector<SynapseLocation*> getPartnerRef();
  std::vector<const SynapseLocation*> getPartnerRef() const;

  std::vector<SynapseLocation*> duplicatedPartner(double thre);

  inline size_t partnerNumber() const { return m_partnerArray.size(); }

  std::vector<SynapseLocation*> getProximalPartner(double distThre);
  std::vector<SynapseLocation*> getDistalPartner(double distThre);

  bool hasPartner(int bodyId) const;

private:
  SynapseLocation m_tBar;
  std::vector<SynapseLocation> m_partnerArray;
};

}
#endif // ZSYNAPSEANNOTATION_H
