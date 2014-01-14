#ifndef ZSYNAPSEANNOTATIONMETADATA_H
#define ZSYNAPSEANNOTATIONMETADATA_H

#include <string>
#include "neurolabi_config.h"
#include "tz_json.h"

namespace FlyEm {

class ZSynapseAnnotationMetadata
{
public:
  ZSynapseAnnotationMetadata();

  void set(std::string description, std::string software,
      std::string softwareVersion, std::string date, std::string sessionPath,
      int version);

  void loadJsonObject(json_t *object);

  std::string toString(int indent = 0);

  inline void setSoftware(const std::string &software) {
    m_software = software;
  }
  inline void setSoftwareVersion(const std::string &version) {
    m_softwareVersion = version;
  }
  inline void setDescription(const std::string &description) {
    m_description = description;
  }
  inline void setDate(const std::string &date) {
    m_date = date;
  }
  inline void setSessionPath(const std::string &session) {
    m_sessionPath = session;
  }
  inline void setSourceDimension(const int *value) {
    for (int i = 0; i < 3; ++i) {
      m_sourceDimension[i] = value[i];
    }
  }
  inline void setSourceOffset(const int *value) {
    for (int i = 0; i < 3; ++i) {
      m_sourceOffset[i] = value[i];
    }
  }
  inline void setResolution(const double *value) {
    for (int i = 0; i < 3; ++i) {
      m_resolution[i] = value[i];
    }
  }

  inline int getSourceYDim() const { return m_sourceDimension[1]; }
  inline int getSourceZOffset() const { return m_sourceOffset[2]; }

  inline double getXResolution() const { return m_resolution[0]; }
  inline double getYResolution() const { return m_resolution[1]; }
  inline double getZResolution() const { return m_resolution[2]; }

private:
  std::string m_description;
  std::string m_software;
  std::string m_softwareVersion;
  std::string m_date;
  std::string m_sessionPath;
  double m_resolution[3];
  int m_sourceDimension[3];
  int m_sourceOffset[3];
  int m_version;
};

}

#endif // ZSYNAPSEANNOTATIONMETADATA_H
