#ifndef Z3DROTATION_H
#define Z3DROTATION_H

#include "zpoint.h"
#include "zjsonobject.h"

class Z3DRotation
{
public:
  Z3DRotation();

  enum EAngleUnit {
    RADIAN, DEGREE
  };

  inline void setAxis(const ZPoint &point) { m_axis = point; }
  inline void setAngle(double angle) { m_angle = angle; }
  inline ZPoint getAxis() const { return m_axis; }
  inline double getAngle() const { return m_angle; }

  void setAngle(double angle, EAngleUnit unit);
  double getAngle(EAngleUnit unit) const;

  void loadJsonObject(const ZJsonObject &obj);
  void print() const;

private:
  ZPoint m_axis;
  double m_angle; //in radian
};

#endif // Z3DROTATION_H
