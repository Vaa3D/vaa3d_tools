#ifndef ZBALL_H
#define ZBALL_H

class ZBall
{
public:
  ZBall();
  ZBall(double x, double y, double z, double r);

public: //Attributes
  inline double x() const { return m_x; }
  inline double y() const { return m_y; }
  inline double z() const { return m_z; }
  inline double r() const { return m_r; }

  inline void setX(double x) { m_x = x; }
  inline void setY(double y) { m_y = y; }
  inline void setZ(double z) { m_z = z; }
  inline void setR(double r) { m_r = r; }

protected:
  double m_x;
  double m_y;
  double m_z;
  double m_r;
};

#endif // ZBALL_H
