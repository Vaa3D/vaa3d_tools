#ifndef ZPUNCTUM_H
#define ZPUNCTUM_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QColor>
#include <vector>
#include <string>

#include "zstackdrawable.h"
#include "zdocumentable.h"
#include "zswcexportable.h"
#include "zvrmlexportable.h"
#include "tz_swc_cell.h"
#include "tz_math.h"
#include "zpoint.h"
#include "zvaa3dmarker.h"

class ZPunctum : public ZStackDrawable, public ZDocumentable
{
public:
  ZPunctum();
  ZPunctum(double x, double y, double z, double r);
  virtual ~ZPunctum();

  virtual const std::string& className() const;

public:

  virtual void display(QPainter &painter, int n = 0, Display_Style style = NORMAL) const;

public: // I/O functions
  virtual inline void save(const char *filePath) {Q_UNUSED(filePath)}
  virtual inline void load(const char *filePath) {Q_UNUSED(filePath)}

  static QList<ZPunctum*> deepCopyPunctaList(const QList<ZPunctum*> &src);

public:
  virtual void setSelected(bool selected);

  inline double x() const { return m_x; }
  inline double y() const { return m_y; }
  inline double z() const { return m_z; }
  inline double sDevOfIntensity() const { return m_sDevOfIntensity; }
  inline double maxIntensity() const { return m_maxIntensity; }
  inline double meanIntensity() const { return m_meanIntensity; }
  inline double volSize() const { return m_volSize; }
  inline double mass() const { return m_mass; }
  inline double radius() const { return m_radius; }
  inline const QString& name() const {return m_name;}
  inline const QString& comment() const {return m_comment;}
  inline const QString& source() const {return m_source;}
  inline const QString& property1() const {return m_property1;}
  inline const QString& property2() const {return m_property2;}
  inline const QString& property3() const {return m_property3;}
  inline const QColor& color() const {return m_color;}
  inline double score() const { return m_score; }
  inline void setX(double n) {  m_x = n; }
  inline void setY(double n) {  m_y = n; }
  inline void setZ(double n) {  m_z = n; }
  inline void setSDevOfIntensity(double n) {  m_sDevOfIntensity = n; }
  inline void setMaxIntensity(double n) {  m_maxIntensity = n; }
  inline void setMeanIntensity(double n) {  m_meanIntensity = n;}
  inline void setVolSize(double n) {  m_volSize = n;}
  inline void setMass(double n) {  m_mass = n; }
  inline void setRadius(double n) {  m_radius = n;}
  inline void setName(const QString &n) { m_name = n;}
  inline void setComment(const QString &n) { m_comment = n;}
  inline void setProperty1(const QString &n) { m_property1 = n;}
  inline void setProperty2(const QString &n) { m_property2 = n;}
  inline void setProperty3(const QString &n) { m_property3 = n;}
  //inline void setColor(const QColor &n) { m_color = n;}
  inline void setSource(const QString &n) {m_source = n;}
  inline void setScore(double s) { m_score = s; }

  void setFromMarker(const ZVaa3dMarker &marker);

  void translate(double dx, double dy, double dz);
  void translate(const ZPoint &offset);

  // update radius from volSize
  inline void updateRadius() { m_radius = Cube_Root(0.75 / M_PI * m_volSize); }
  // update volSize from radius
  inline void updateVolSize() { m_volSize = M_PI * 1.333333333 * m_radius * m_radius * m_radius; }
  // update mass
  inline void updateMass() { m_mass = m_volSize * m_meanIntensity; }

  std::string toString();

private:
  QColor highlightingColor(const QColor &color) const;
  QColor selectingColor(const QColor &color) const;

private:
  QString m_name;
  QString m_comment;
  double m_maxIntensity;
  double m_meanIntensity;
  double m_x;
  double m_y;
  double m_z;
  double m_sDevOfIntensity;
  double m_volSize;
  double m_mass;
  double m_radius;   //radius
  QString m_property1;
  QString m_property2;
  QString m_property3;
  //QColor m_color;
  QString m_source;
  double m_score;  // detection score [-1.0 1.0]
};

#endif // ZPUNCTUM_H
