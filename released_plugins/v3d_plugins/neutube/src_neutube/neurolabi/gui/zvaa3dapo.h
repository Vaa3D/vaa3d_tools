#ifndef ZVAA3DAPO_H
#define ZVAA3DAPO_H

#include "zvaa3dmarker.h"
#include "tz_constant.h"

class ZVaa3dApo : public ZVaa3dMarker
{
public:
  ZVaa3dApo();

public:
  virtual std::string toString();

  inline void setId(int id) { m_id = id; }

  inline double volSize() { return TZ_PI * 1.333333333 * m_r * m_r * m_r; }

private:
  int m_id;
  int m_orderInfo;
  double m_maxIntensity;
  double m_intensity;
  double m_sdev;
  double m_mass;
};

#endif // ZVAA3DAPO_H
