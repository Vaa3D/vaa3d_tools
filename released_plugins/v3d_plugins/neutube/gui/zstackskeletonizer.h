#ifndef ZSTACKSKELETONIZER_H
#define ZSTACKSKELETONIZER_H

#include "tz_image_lib_defs.h"
#include "zprogressable.h"

class ZSwcTree;

class ZStackSkeletonizer : public ZProgressable
{
public:
  ZStackSkeletonizer();

  inline void setLengthThreshold(double threshold) {
    m_lengthThreshold = threshold;
  }

  inline void setDistanceThreshold(double threshold) {
    m_distanceThreshold = threshold;
  }

  inline void setRebase(bool rebase) {
    m_rebase = rebase;
  }

  inline void setInterpolating(bool inter) {
    m_interpolating = inter;
  }

  inline void setRemovingBorder(bool r) {
    m_removingBorder = r;
  }

  inline void setMinObjSize(int s) {
    m_minObjSize = s;
  }

  inline void setKeepingSingleObject(bool k) {
    m_keepingSingleObject = k;
  }

  inline void setLevel(int level) {
    m_level = level;
  }

  inline void setResolution(double xyRes, double zRes) {
    m_resolution[0] = xyRes;
    m_resolution[1] = xyRes;
    m_resolution[2] = zRes;
  }

  ZSwcTree* makeSkeleton(const Stack *stack);

  void reconnect(ZSwcTree *tree);
  inline void setConnectingBranch(bool conn) {
    m_connectingBranch = conn;
  }

private:
  double m_lengthThreshold;
  double m_distanceThreshold;
  bool m_rebase;
  bool m_interpolating;
  bool m_removingBorder;
  int m_minObjSize;
  bool m_keepingSingleObject;
  int m_level;
  bool m_connectingBranch;
  double m_resolution[3];
};

#endif // ZSTACKSKELETONIZER_H
