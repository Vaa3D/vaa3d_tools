#ifndef ZKMEANSCLUSTERING_H
#define ZKMEANSCLUSTERING_H

#include <vector>
#include "zdoublevector.h"
#include "zobject3d.h"

class ZKmeansClustering
{
public:
  ZKmeansClustering();

public:
  std::vector<int>& runKmeans(const ZObject3d &obj, int k,
                              const ZDoubleVector &weightArray);
  std::vector<int>& runKmeans(const Object_3d *obj, int k,
                              const ZDoubleVector &weightArray);

  inline void setSse(double sse) { m_sse = sse; }
  inline void setMaxIter(int maxIter) { m_maxIter = maxIter; }
  inline void setMinDelta(double minDelta) { m_minDelta = minDelta; }

  inline std::vector<int>& labelArray() { return m_labelArray; }

private:
  void vggKmiter(const Object_3d *obj,
                 const ZDoubleVector &weightArray);

private:
  std::vector<ZDoubleVector> m_centerArray;
  std::vector<int> m_labelArray;
  double m_sse;
  int m_maxIter;
  double m_minDelta;
};

#endif // ZKMEANSCLUSTERING_H
