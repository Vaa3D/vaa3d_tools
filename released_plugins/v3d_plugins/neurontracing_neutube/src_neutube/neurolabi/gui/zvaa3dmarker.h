#ifndef ZVAA3DMARKER_H
#define ZVAA3DMARKER_H

#include <string>
#include "tz_image_lib_defs.h"
#include "zpoint.h"
#include "tz_swc_tree.h"

//Marker field:
//x,  y,  z,  radius,  shape,  name,  comment, r, g, b,
class ZVaa3dMarker
{
public:
  ZVaa3dMarker();

  inline double x() const { return m_center.x(); }
  inline double y() const { return m_center.y(); }
  inline double z() const { return m_center.z(); }
  inline double radius() const { return m_r; }

  inline void setX(double x) { m_center.setX(x); }
  inline void setY(double y) { m_center.setY(y); }
  inline void setZ(double z) { m_center.setZ(z); }
  inline void setCenter(double x, double y, double z) {
    m_center.set(x, y, z);
  }
  inline void setRadius(double r) {
    m_r = r;
  }

  inline void setColor(uint8_t r, uint8_t g, uint8_t b) {
    m_color[0] = r; m_color[1] = g; m_color[2] = b;
  }

  void set(const std::string &line);

  inline void setName(const std::string &name) { m_name = name; }
  inline void setComment(const std::string &comment) { m_comment = comment; }
  inline void setType(int type) { m_type = type; }

  inline std::string name() const { return m_name; }
  inline ZPoint center() const { return m_center; }
  inline int type() const { return m_type; }
  inline std::string comment() const { return m_comment; }
  inline uint8_t colorR() const { return m_color[0]; }
  inline uint8_t colorG() const { return m_color[1]; }
  inline uint8_t colorB() const { return m_color[2]; }

public:
  void adjustForBaseOne();
  void moveToSurface(Swc_Tree *tree);

  virtual std::string toString();

protected:
  ZPoint m_center;
  double m_r;
  int m_type;
  std::string m_name;
  std::string m_comment;
  color_t m_color;
};

#endif // ZVAA3DMARKER_H
