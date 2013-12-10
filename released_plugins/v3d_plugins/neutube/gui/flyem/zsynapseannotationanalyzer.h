#ifndef ZSYNAPSEANNOTATIONANALYZER_H
#define ZSYNAPSEANNOTATIONANALYZER_H

#include "zsynapseannotationarray.h"
#include "tz_swc_tree.h"
#include <map>
#include <string>
#include <vector>

namespace FlyEm {

class ZSynapseAnnotationAnalyzer
{
public:
  ZSynapseAnnotationAnalyzer();
  ~ZSynapseAnnotationAnalyzer();

public:
  void set(const std::vector<int> bodyIdList,
           const std::vector<std::string> swcFileList);

  void setStackSize(int width, int height, int depth);
  void setDownsampleRate(int ds1, int ds2);
  void setResolution(double xRes, double yRes, double zRes);
  void setOffset(int dx, int dy, int dz);
  void setUnit(std::string unit);
  void setStartNumber(int number);

  void identifySynapseLocation(ZSynapseAnnotationArray *synapseArray);
  inline SynapseAnnotationConfig& config() { return m_config; }

public:
  void loadConfig(std::string filePath);
  void print();

private:
  SynapseAnnotationConfig m_config;
  std::map<int, int> m_bodyIdMap;
  std::vector<std::string> m_swcFileList;
  std::vector<Swc_Tree*> m_swcTreeList;

  //The distances are all in the unit of 'nm'
  double m_minTbarDistance;
  double m_minPsdDistance;
  double m_minTbarPsdDistance;
  double m_maxTbarPsdDistance;
  int m_minPartnerNumber;
  int m_maxPartnerNumber;
};

}
#endif // ZSYNAPSEANNOTATIONANALYZER_H
