#include "zstroke2d.h"

#include <QPainter>
#include <QPen>
#include "tz_math.h"

const double ZStroke2d::m_minWidth = 1.0;
const double ZStroke2d::m_maxWidth = 100.0;

ZStroke2d::ZStroke2d() : m_width(10.0), m_isEraser(false), m_isFilled(true)
{
  setEraser(m_isEraser);
}

ZStroke2d::ZStroke2d(const ZStroke2d &stroke) :
  ZInterface(stroke), ZDocumentable(stroke), ZStackDrawable(stroke)
{
  m_pointArray = stroke.m_pointArray;
  m_width = stroke.m_width;
  m_isEraser = stroke.m_isEraser;
  m_isFilled = stroke.m_isFilled;
}

ZStroke2d::~ZStroke2d()
{
}

ZINTERFACE_DEFINE_CLASS_NAME(ZStroke2d)

void ZStroke2d::save(const char */*filePath*/)
{

}

void ZStroke2d::load(const char */*filePath*/)
{

}

void ZStroke2d::append(double x, double y)
{
  if (m_pointArray.empty()) {
    m_pointArray.push_back(QPointF(x, y));
  } else {
    if (x != m_pointArray.back().x() && y != m_pointArray.back().y()) {
      m_pointArray.push_back(QPointF(x, y));
    }
  }
}

void ZStroke2d::set(const QPoint &pt)
{
  set(pt.x(), pt.y());
}

void ZStroke2d::set(double x, double y)
{
  clear();
  append(x, y);
}

void ZStroke2d::setEraser(bool isEraser)
{
  m_isEraser = isEraser;
  if (m_isEraser) {
    m_color.setRgb(255, 255, 255);
  } else {
    m_color.setRgb(255, 0, 0);
  }
  m_color.setAlpha(128);
}

void ZStroke2d::display(QPainter &painter, int z, Display_Style option) const
{
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);

  QPen pen(m_color);
  QBrush brush(m_color);

  if (m_isEraser) {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
  } else {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
  }

  if (!m_pointArray.empty()) {
    if (m_pointArray.size() == 1) {
      if (m_isFilled) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(brush);
      } else {
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
      }
      painter.drawEllipse(QPointF(m_pointArray[0]), m_width / 2, m_width / 2);
    } else {
      pen.setCapStyle(Qt::RoundCap);
      pen.setWidthF(m_width);
      painter.setPen(pen);
      painter.setBrush(Qt::NoBrush);
      painter.setOpacity(1.0);
      painter.drawPolyline(&(m_pointArray[0]), m_pointArray.size());

      /*
        painter.setPen(Qt::NoPen);
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(m_pointArray.back()), m_width / 2, m_width / 2);
        */
    }
  }
}

void ZStroke2d::clear()
{
  m_pointArray.clear();
}

bool ZStroke2d::isEmpty()
{
  return m_pointArray.empty();
}

ZStroke2d* ZStroke2d::clone()
{
  ZStroke2d *stroke = new ZStroke2d(*this);

  return stroke;
}


void ZStroke2d::addWidth(double dw)
{
  m_width += dw;
  if (m_width < m_minWidth) {
    m_width = m_minWidth;
  } else if (m_width > m_maxWidth) {
    m_width = m_maxWidth;
  }
}

bool ZStroke2d::getLastPoint(int *x, int *y) const
{
  if (m_pointArray.empty()) {
    return false;
  }

  *x = iround(m_pointArray.back().x());
  *y = iround(m_pointArray.back().y());

  return true;
}
