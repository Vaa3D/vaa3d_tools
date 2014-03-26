#include "z3drotation.h"

#include <iostream>
#include "tz_constant.h"
#include "zjsonparser.h"

using namespace std;

Z3DRotation::Z3DRotation() : m_axis(0.0, 1.0, 0.0), m_angle(0.0)
{
}

void Z3DRotation::setAngle(double angle, EAngleUnit unit)
{
  m_angle = angle;

  if (unit == DEGREE) {
    m_angle *= TZ_PI / 180.0;
  }
}

double Z3DRotation::getAngle(EAngleUnit unit) const
{
  double angle = m_angle;

  if (unit == DEGREE) {
    angle *= 180.0 / TZ_PI;
  }

  return angle;
}

void Z3DRotation::loadJsonObject(const ZJsonObject &obj)
{
  setAngle(ZJsonParser::numberValue(obj["angle"]), DEGREE);
  const json_t *axisObj = obj["axis"];
  setAxis(ZPoint(ZJsonParser::numberValue(axisObj, 0),
                 ZJsonParser::numberValue(axisObj, 1),
                 ZJsonParser::numberValue(axisObj, 2)));
}

void Z3DRotation::print() const
{
  std::cout << "axis: " << endl;
  cout << m_axis.toString() << endl;
  cout << "Angle: " << m_angle << endl;
}
