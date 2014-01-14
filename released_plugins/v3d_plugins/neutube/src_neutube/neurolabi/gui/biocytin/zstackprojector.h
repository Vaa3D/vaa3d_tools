#ifndef ZSTACKPROJECTOR_H
#define ZSTACKPROJECTOR_H

#include <vector>
#include <string>
#include "zprogressable.h"
#include "tz_fmatrix.h"
#include "tz_image_lib_defs.h"

class ZStack;

namespace Biocytin {
class ZStackProjector : public ZProgressable {
public:
  ZStackProjector();

  ZStack* project(const ZStack *stack, bool includingDepth = false);

  inline void setAdjustingContrast(bool adj) {
    m_adjustingConstrast = adj;
  }

  inline void setSmoothingDepth(bool stat) {
    m_smoothingDepth = stat;
  }

  inline void setSpeedLevel(int level) {
    m_speedLevel = level;
  }

  inline void setUsingExisted(bool stat) {
    m_usingExisted = stat;
  }

  inline const std::vector<int>& getDepthArray() { return m_depthArray; }

  static std::string getDefaultResultFilePath(const std::string &basePath);
private:
  inline double colorToValue(double g, double sr, double sg, double reg,
                             double redScale, double redOffset,
                             double greenScale, double greenOffset) {
    sr = sr * redScale + redOffset;
    sg = sg * greenScale + greenOffset;
    double ratio = (sg + reg) / (sr + reg);
    return g * ratio * ratio;
  }

  inline double colorToValue(double g, double sr, double sg, double reg) {
    double ratio = (sg + reg) / (sr + reg);
    return g * ratio * ratio;
  }

  double colorToValueH(double sr, double sg, double sb, double reg);

private:
  FMatrix* smoothStackNull(Stack *stack);
  FMatrix* smoothStack(Stack *stack);
  FMatrix* smoothStackGaussian(Stack *stack);

private:
  bool m_adjustingConstrast;
  bool m_smoothingDepth;
  int m_speedLevel;
  bool m_usingExisted;
  std::vector<int> m_depthArray;
};
}

#endif // ZSTACKPROJECTOR_H
