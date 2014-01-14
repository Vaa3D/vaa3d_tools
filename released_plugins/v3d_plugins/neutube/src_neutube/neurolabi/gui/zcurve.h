/**@file zcurve.h
 * @brief Curve class
 * @author Ting Zhao
 * @date 28-MAY-2009
 */

#ifndef _ZCURVE_H_
#define _ZCURVE_H_

#include <QVector>
#include <QPointF>

class ZCurve {
public:
  ZCurve();
  ~ZCurve() {}

  inline QVector<QPointF> data() const { return m_data; }

  template<class T>
      void loadArray(const T *value, int n);

  double minX() const;
  double minY() const;
  double maxX() const;
  double maxY() const;

  void setYRange(double lower, double upper);

private:
  QVector<QPointF> m_data;
  double m_yRange[2]; /* possible range */

};

#include "zcurve_tmpl.cpp"

#endif /* ZCURVE_H_ */
