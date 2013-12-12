#include "zball.h"

ZBall::ZBall()
{
  ZBall(0.0, 0.0, 0.0, 0.0);
}

ZBall::ZBall(double x, double y, double z, double r)
{
  m_x = x;
  m_y = y;
  m_z = z;
  m_r = r;
}
