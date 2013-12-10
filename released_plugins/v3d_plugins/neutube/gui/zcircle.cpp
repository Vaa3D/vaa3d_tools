#if defined(_QT_GUI_USED_)
#include <QPainter>
#endif

#include <math.h>
#include "zcircle.h"
#include "tz_math.h"

ZCircle::ZCircle()
{
  set(0, 0, 0, 1);
}

ZCircle::ZCircle(double x, double y, double z, double r)
{
  set(x, y, z, r);
}

void ZCircle::set(double x, double y, double z, double r)
{
  m_center.set(x, y, z);
  m_r = r;
}

//void ZCircle::display(QImage *image, int n, Display_Style style) const
//{
//#if defined(_QT_GUI_USED_)
//  QPainter painter(image);
//  painter.setPen(m_color);
//  display(&painter, n, style);
//#endif
//}

void ZCircle::display(QPainter &painter, int n,
                      ZStackDrawable::Display_Style style) const
{
  UNUSED_PARAMETER(style);

  painter.setPen(QPen(m_color, m_defaultPenWidth));
  display(&painter, n, style);
}

void ZCircle::display(QPainter *painter, int n, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if defined(_QT_GUI_USED_)
  if (n == -1) {
    double adjustedRadius = m_r + m_defaultPenWidth * 0.5;
    painter->drawEllipse(QPointF(m_center.x(), m_center.y()),
                         adjustedRadius, adjustedRadius);
  } else {
    double h = fabs(m_center.z() - n);
    if (m_r > h) {
      double r = sqrt(m_r * m_r - h * h);
      double adjustedRadius = r + m_defaultPenWidth * 0.5;
      painter->drawEllipse(QPointF(m_center.x(), m_center.y()), adjustedRadius,
                           adjustedRadius);
      /*
      if (r >= 0.5) {
        painter->drawEllipse(QPointF(m_center.x(), m_center.y()), r, r);
      }
      */
    }
  }
#endif
}

void ZCircle::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZCircle::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

ZINTERFACE_DEFINE_CLASS_NAME(ZCircle)
