#ifndef ZSTACKSTATISTICS_H
#define ZSTACKSTATISTICS_H

#include "zstack.hxx"

class ZStackStatistics
{
public:
  ZStackStatistics();

public:
  static void getOptimalBc(const ZStack &stack, int c,
                           double *greyOffset, double *greyScale);
  static bool getGreyMapHint(const ZStack &stack, int c,
                             double *smin, double *smax);

private:
  static const double m_lowerQuantile;
  static const double m_upperQuantile;
};

#endif // ZSTACKSTATISTICS_H
