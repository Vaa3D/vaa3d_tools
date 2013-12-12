#ifndef ZBCFSET_H
#define ZBCFSET_H

#include <string>
#include <vector>

#include "zmatrix.h"

/**
 * @brief The class for body connection feature set
 *
 * Usage:
 *  FlyEm::ZBcfSet bcfSet("data/feature");
 *  ZMatrix *featureMatrix = bcfSet.load("body_size");
 *  featureMatrix->printInfo();
 */

namespace FlyEm {
class ZBcfSet
{
public:
  ZBcfSet();
  ZBcfSet(const std::string &featurePath);

  ZMatrix* load(const std::string &name);
  ZMatrix* load(const std::vector<std::string> &name);

  inline void setPath(const std::string &featurePath) {
    m_featurePath = featurePath;
  }

  static inline std::string getDatasetPath(const std::string &setName) {
    return std::string("/") + setName;
  }

private:
  std::string m_featurePath;
};
}

#endif // ZBCFSET_H
