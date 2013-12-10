#ifndef ZSYNAPSEANNOTATIONARRAY_H
#define ZSYNAPSEANNOTATIONARRAY_H

#include "zsynapseannotation.h"
#include "zsynapseannotationmetadata.h"

#include <string>
#include <set>
#include "zsynapseannotation.h"
#include "zvaa3dmarker.h"
#include "zvaa3dapo.h"
#include "zinttree.h"

class ZSwcTree;
class ZGraph;
class ZPunctum;

namespace FlyEm {

class WeightedIntPair {
public:
  WeightedIntPair(int t1, int t2, double w) :
    m_first(t1), m_second(t2), m_weight(w)
  {}
  WeightedIntPair(const WeightedIntPair &wp) :
    m_first(wp.m_first), m_second(wp.m_second), m_weight(wp.m_weight)
  {}

  inline int first() const { return m_first; }
  inline int second() const { return m_second; }
  inline double weight() const { return m_weight; }

private:
  int m_first;
  int m_second;
  double m_weight;
};

struct WeightedIntPairCompare {
  bool operator() (const WeightedIntPair &p1, const WeightedIntPair &p2) {
    return (p1.weight() < p2.weight());
  }
};

struct Color {
  unsigned char red;
  unsigned char green;
  unsigned char blue;

  Color() : red(0), green(0), blue(0) {}
  Color(unsigned char r, unsigned char g, unsigned b) : red(r), green(g),
    blue(b) {}
};

struct SynapseDisplayConfig {
  enum EMode {
    TBAR_ONLY, //T-bar on the target body
    PSD_ONLY,  //PSD on the target body
    TBAR_PAIR, //T-bar on the target body and its corresponding PSD
    PSD_PAIR,  //PSD on the target body and its corresponding T-bar
    SYNAPSE_PAIR, //Synapse on the target body
    HALF_SYNAPSE  //Either T-bar or PSD on the target body
  };

  EMode mode;
  Color tBarColor;
  Color psdColor;
  int bodyId;
  int buddyBodyId;

  SynapseDisplayConfig() : mode(HALF_SYNAPSE),
    tBarColor(255, 255, 0), psdColor(32, 32, 32),
    bodyId(-1), buddyBodyId(-1) {}
};

class ZSynapseAnnotationArray : public std::vector<ZSynapseAnnotation>
{
public:
  ZSynapseAnnotationArray();
  ~ZSynapseAnnotationArray();

  enum ELoadDataMode { APPEND, OVERWRITE };
  enum EComponent { ALL_COMPONENT, CONNECTION_GRAPH };

  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);
  bool isDeprecated(EComponent component) const;

public:
  bool loadJson(const std::string &filePath, ELoadDataMode mode = OVERWRITE);

  std::string metadataString();
  std::string toString(int indent = 0,
                       const std::vector<std::vector<int> > *selected = NULL);
  std::string toSummaryString();

  SynapseLocation* beginSynapseLocation();
  const SynapseLocation* beginSynapseLocation() const;
  SynapseLocation* currentSynapseLocation();
  const SynapseLocation* currentSynapseLocation() const;
  SynapseLocation* nextSynapseLocation();
  const SynapseLocation* nextSynapseLocation() const;

  const ZSynapseAnnotation* currentSynapseAnnotation() const;

  void simplifyCommonSwcPath();

  std::vector<SynapseLocation*> toSynapseLocationArray();
  std::vector<SynapseLocation*> toTBarRefArray();
  std::vector<SynapseLocation*> toMultiTBarRefArray();
  std::vector<SynapseLocation*> toPsdRefArray();
  std::vector<SynapseLocation*> toPsdRefArray(int index);

  /*!
   * \brief Convert the coodindates of each synapse location from Raveler space
   *        to normal image space.
   *
   * \param startZ The start value of Z in Raveler. startZ is converted to 0
   *        in the imaeg space.
   * \param height Height of the source image to flip Y.
   */
  void convertRavelerToImageSpace(int startZ, int height);

public:
  bool exportMarkerFile(std::string filePath,
                        const SynapseAnnotationConfig &config,
                        SynapseLocation::ELocationSpace spaceOption,
                        const SynapseDisplayConfig &displayConfig);
  bool exportConnection(std::string filePath,
                        const SynapseAnnotationConfig &config,
                        SynapseLocation::ELocationSpace spaceOption,
                        const SynapseDisplayConfig &displayConfig);
  bool exportApoFile(std::string filePath,
                     const SynapseAnnotationConfig &config,
                     SynapseLocation::ELocationSpace spaceOption);
 // bool exportJsonFile(std::string filePath);
  //For each array of <selected>, the first element is the tbar-index and the
  //following elements are the psd indices.
  bool exportJsonFile(const std::string &filePath,
                      const std::vector<std::vector<int> > *selected = NULL);

  bool exportSwcFile(std::string filePath,
                     const SynapseAnnotationConfig &config,
                     SynapseLocation::ELocationSpace spaceOption,
                     const SynapseDisplayConfig &displayConfig);

  /*!
   * \brief Export TBar as CSV
   * \param filePath saving path
   * \return true iff the file is saved.
   */
  bool exportTBar(const std::string &filePath);

  /*!
   * \brief Export Psd as CSV
   * \param filePath saving path
   * \return true iff the file is saved
   */
  bool exportPsd(const std::string &filePath);

  /*!
   * \brief Export syanpses in CSV
   *
   * The CSV file has five columns, Cell id for tbar, cell id for psd, x, y, z
   *
   * \param filePath saving path
   * \return true iff the file is saved
   */
  bool exportCsvFile(const std::string &filePath);

  std::vector<ZVaa3dMarker> toMarkerArray(
      const SynapseAnnotationConfig &config,
      SynapseLocation::ELocationSpace spaceOption,
      const SynapseDisplayConfig &displayConfig
      ) const;

  std::vector<ZVaa3dApo> toApoArray(
      const SynapseAnnotationConfig &config,
      SynapseLocation::ELocationSpace spaceOption,
      const SynapseDisplayConfig &displayConfig
      );

#ifdef _QT_GUI_USED_
  std::vector<ZPunctum*> toPuncta(const SynapseAnnotationConfig &config,
                                  SynapseLocation::ELocationSpace spaceOption,
                                  const SynapseDisplayConfig &displayConfig) const;
#endif

  ZSwcTree* toSwcTree(const SynapseAnnotationConfig &config,
                      SynapseLocation::ELocationSpace spaceOption,
                      const SynapseDisplayConfig &displayConfig
                      );

  ZSwcTree* toSwcTree();

  ZGraph* toGraph();
  ZGraph* toGraph(const std::set<int> &bodySet);

  int getPsdIndex(size_t tbarIndex, size_t partnerIndex) const;
  int getPsdIndex(int x, int y, int z) const;

  void printSummary();
  void print();

  std::vector<std::vector<SynapseLocation*> > buildTbarSequence(double maxDist);

  const SynapseLocation* getTBarRef(int index) const;
  SynapseLocation* getTBarRef(int index);
  const SynapseLocation* getPsdRef(int tbarIndex, int psdIndex) const;
  SynapseLocation* getPsdRef(int tbarIndex, int psdIndex);

  int getPsdNumber() const;
  int getTBarNumber() const;

  inline std::string source() { return m_source; }

  std::pair<int, int> relativePsdIndex(int index);

  inline void setSoftware(const std::string &software) {
    m_metadata.setSoftware(software);
  }
  inline void setSoftwareVersion(const std::string &version) {
    m_metadata.setSoftwareVersion(version);
  }
  inline void setDescription(const std::string &description) {
    m_metadata.setDescription(description);
  }
  inline void setDate(const std::string &date) {
    m_metadata.setDate(date);
  }
  inline void setSessionPath(const std::string &session) {
    m_metadata.setSessionPath(session);
  }
  inline void setSourceDimension(const int *value) {
    m_metadata.setSourceDimension(value);
  }
  inline void setSourceOffset(const int *value) {
    m_metadata.setSourceOffset(value);
  }
  inline void setResolution(const double *value) {
    m_metadata.setResolution(value);
  }

  std::vector<std::pair<SynapseLocation*, SynapseLocation*> >
  findDuplicatedTBar(double minDist);

  std::vector<int> countPsd();
  std::vector<int> countTBar();

  /*!
   * \brief Count synapse of all bodies
   * \return An array of which the ith element is the number of synapses of the
   *         body with ID i.
   */
  std::vector<int> countSynapse();

  int countPsd(int bodyId) const;
  int countTBar(int bodyId) const;
  /*!
   * \brief Test if a body has any PSD
   * \return true iff the body \a bodyId has any PSD on it.
   */
  bool hasPsd(int bodyId) const;

  /*!
   * \brief Test if a body has any TBar
   * \return true iff the body \a bodyId has any TBar on it.
   */
  bool hasTBar(int bodyId) const;

  int countInputNeuron(int bodyId) const;
  int countOutputNeuron(int bodyId) const;

  int getStrongestInput(int bodyId) const;
  int getStrongestOutput(int bodyId) const;

  std::vector<SynapseLocation*> getTBarArray(int bodyId) const;
  std::vector<SynapseLocation*> getPsdArray(int bodyId) const;

  inline const ZSynapseAnnotationMetadata& getMetaData() const {
    return m_metadata;
  }

  ZGraph* getConnectionGraph(bool excludingSelfConnection = true);

private:
  bool isPartnerOnBody(int bodyId) const;
  static void assembleTBarSequence(ZIntTree *tbarSequence,
                                   ZIntTree *tbarSequenceReverse,
                                   int endIndex, int freeIndex);

private:
  ZSynapseAnnotationMetadata m_metadata;

  ZGraph *m_connectionGraph;
  mutable size_t m_currentIndex;
  mutable size_t m_currentLocationIndex; //0 for T-Bar, >=1 for PSD partners
  std::string m_source;
};

}

#endif // ZSYNAPSEANNOTATIONARRAY_H
