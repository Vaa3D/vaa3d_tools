#if defined(_QT_GUI_USED_)
#include <QPainter>
#endif

#include <math.h>
#include "zcircle.h"
#include "tz_math.h"


const ZCircle::TVisualEffect ZCircle::VE_NONE = 0;
const ZCircle::TVisualEffect ZCircle::VE_DASH_PATTERN = 1;
const ZCircle::TVisualEffect ZCircle::VE_BOUND_BOX = 2;
const ZCircle::TVisualEffect ZCircle::VE_NO_CIRCLE = 4;

ZCircle::ZCircle() : m_visualEffect(ZCircle::VE_NONE)
{
  set(0, 0, 0, 1);
}

ZCircle::ZCircle(double x, double y, double z, double r) :
  m_visualEffect(ZCircle::VE_NONE)
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
#if _QT_GUI_USED_
  QPen pen(m_color, m_defaultPenWidth);
  if (hasVisualEffect(VE_DASH_PATTERN)) {
    pen.setStyle(Qt::DotLine);
  }

  painter.setPen(pen);

  display(&painter, n, style);
#endif
}

bool ZCircle::isCuttingPlane(double z, double r, double n)
{
  double h = fabs(z - n);
  if (r > h) {
    return true;
  } else if (iround(z) == iround(n)) {
    return true;
  }

  return false;
}

void ZCircle::display(QPainter *painter, int n, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if defined(_QT_GUI_USED_)
  double adjustedRadius = m_r + m_defaultPenWidth * 0.5;
  QRectF rect;
  if (hasVisualEffect(VE_BOUND_BOX)) {
    rect.setLeft(m_center.x() - adjustedRadius);
    rect.setTop(m_center.y() - adjustedRadius);
    rect.setWidth(adjustedRadius + adjustedRadius);
    rect.setHeight(adjustedRadius + adjustedRadius);
  }

  bool visible = false;

  if (n == -1) {
    visible = true;
  } else {
    if (isCuttingPlane(m_center.z(), m_r, n)) {
      double h = fabs(m_center.z() - n);
      if (m_r > h) {
        double r = sqrt(m_r * m_r - h * h);
        adjustedRadius = r + m_defaultPenWidth * 0.5;
        visible = true;
      } else { //too small, show at least one plane
        adjustedRadius = m_defaultPenWidth * 0.5;
        visible = true;
      }
    }
  }

  if (visible) {
    if (hasVisualEffect(VE_BOUND_BOX)) {
      painter->drawRect(rect);
    }

    if (!hasVisualEffect(VE_NO_CIRCLE)) {
      painter->drawEllipse(QPointF(m_center.x(), m_center.y()),
                           adjustedRadius, adjustedRadius);
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
