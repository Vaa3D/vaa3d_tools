#include <QtGui>

#include <numeric>
#include <algorithm>
#include <sstream>

#include "zpunctum.h"
#include "zrandom.h"

ZPunctum::ZPunctum()
  : m_score(1.0)
{
  setColor(255, 255, 0, 255);
  setX(-1);
  setY(-1);
  setZ(-1);
  setRadius(2.0);
  setMaxIntensity(255);
  setMeanIntensity(255);
  setSDevOfIntensity(0);
  updateVolSize();
  updateMass();
}

ZPunctum::ZPunctum(double x, double y, double z, double r)
  : m_score(1.0)
{
  setColor(255, 255, 0, 255);
  setX(x);
  setY(y);
  setZ(z);
  setRadius(r);
  setMaxIntensity(255);
  setMeanIntensity(255);
  setSDevOfIntensity(0);
  updateVolSize();
  updateMass();
}

ZPunctum::~ZPunctum()
{
}

//void ZPunctum::display(QImage *image, int n, Display_Style style) const
//{
//  if (!isVisible())
//    return;

//  if (style == NORMAL) {
//    style = SOLID;
//  }

//  QPainter painter(image);
//  if (m_selected == true) {
//    painter.setPen(QPen(selectingColor(m_color)));
//  } else {
//    painter.setPen(QPen(m_color));
//  }

//  switch (style) {
//  case SOLID: {
//    if ((iround(m_z) == n) || (n == -1)) {
//      int half_size = iround(m_radius - 0.5);
//      int cx = iround(m_x);
//      int cy = iround(m_y);
//      painter.drawRect(cx - half_size, cy - half_size,
//                       half_size * 2 + 1, half_size * 2 + 1);
//    }
//    break;
//  }
//  case SKELETON:
//    break;
//  case BOUNDARY: {
//    double r = m_radius;
//    bool visible = false;

//    if ((iround(m_z) == n) || (n == -1)) {
//      visible = true;
//    } else if (fabs(m_z - n) < r) {
//      r = sqrt(r * r - (m_z - n) * (m_z - n));
//      visible = true;
//    }

//    if (visible) {
//      if (m_selected == true) {
//        painter.setPen(QPen(selectingColor(m_color)));
//      } else {
//        painter.setPen(QPen(m_color));
//      }
//      painter.drawEllipse(QPointF(m_x, m_y), r, r);
//    }
//    break;
//  }
//  default:
//    break;
//  }
//}

void ZPunctum::display(QPainter &painter, int n, ZStackDrawable::Display_Style style) const
{
  if (!isVisible())
    return;

  if (style == NORMAL) {
    style = SOLID;
  }

  if (m_selected == true) {
    painter.setPen(QPen(selectingColor(m_color), 1.5));
  } else {
    painter.setPen(QPen(m_color, .7));
  }

  switch (style) {
  case SOLID: {
    if ((iround(m_z) == n) || (n == -1)) {
      int half_size = iround(m_radius - 0.5);
      int cx = iround(m_x);
      int cy = iround(m_y);
      painter.drawRect(cx - half_size, cy - half_size,
                       half_size * 2 + 1, half_size * 2 + 1);
    }
    break;
  }
  case SKELETON:
    break;
  case BOUNDARY: {
    double r = m_radius;
    bool visible = false;

    if ((iround(m_z) == n) || (n == -1)) {
      visible = true;
    } else if (fabs(m_z - n) < r) {
      r = sqrt(r * r - (m_z - n) * (m_z - n));
      visible = true;
    }

    if (visible) {
      if (m_selected == true) {
        painter.setPen(QPen(selectingColor(m_color), 1.5));
      } else {
        painter.setPen(QPen(m_color, .7));
      }
      painter.drawEllipse(QPointF(m_x, m_y), r, r);
    }
    break;
  }
  default:
    break;
  }
}

QList<ZPunctum *> ZPunctum::deepCopyPunctaList(const QList<ZPunctum *> &src)
{
  QList<ZPunctum*> des;
  for (int i=0; i<src.size(); ++i) {
    des.push_back(new ZPunctum(*(src[i])));
  }
  return des;
}

void ZPunctum::setSelected(bool selected)
{
  ZStackDrawable::setSelected(selected);
}

QColor ZPunctum::highlightingColor(const QColor &color) const
{
  QColor highlight;

  highlight.setRed(imin2(255, color.red() + 96));
  highlight.setGreen(imin2(255, color.green() + 96));
  highlight.setBlue(imin2(255, color.blue() + 96));

  return highlight;
}

QColor ZPunctum::selectingColor(const QColor &color) const
{
  QColor select;

  select.setHsv((color.hue() + 60) % 360,
                /*color.saturation()*/255, 255/*color.value()*/);

  return select;
}

std::string ZPunctum::toString()
{
  std::ostringstream stream;

  stream << "Puncta(" << m_name.toStdString() << "): "
         << "(" << m_x << ", " << m_y << ", " << m_z << ")";

  return stream.str();
}

void ZPunctum::translate(double dx, double dy, double dz)
{
  m_x += dx;
  m_y += dy;
  m_z += dz;
}

void ZPunctum::translate(const ZPoint &offset)
{
  translate(offset.x(), offset.y(), offset.z());
}

void ZPunctum::setFromMarker(const ZVaa3dMarker &marker)
{
  setX(marker.x());
  setY(marker.y());
  setZ(marker.z());
  setRadius(marker.radius());
  setColor(marker.colorR(), marker.colorG(), marker.colorB());
  setComment(marker.comment().c_str());
  setName(marker.name().c_str());
  setSource(QString("%1").arg(marker.type()));
}

ZINTERFACE_DEFINE_CLASS_NAME(ZPunctum)
