#ifndef ZFLYEMBODYANALYZER_H
#define ZFLYEMBODYANALYZER_H

#include "zpoint.h"
#include "zpointarray.h"
#include "zobject3dscan.h"

class ZFlyEmBodyAnalyzer
{
public:
  ZFlyEmBodyAnalyzer();

public:
  ZPointArray computeHoleCenter(const ZObject3dScan &obj);
  ZPointArray computeTerminalPoint(const ZObject3dScan &obj);
  ZPointArray computeLoopCenter(const ZObject3dScan &obj);

  void setDownsampleInterval(int ix, int iy, int iz);
  inline void setMinLoopSize(int n) { m_minLoopSize = n; }

private:
  int m_downsampleInterval[3];
  int m_minLoopSize;
};

#endif // ZFLYEMBODYANALYZER_H
