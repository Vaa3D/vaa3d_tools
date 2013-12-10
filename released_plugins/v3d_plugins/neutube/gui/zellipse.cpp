/*
 * zellipse.cpp
 *
 *  Created on: Jun 1, 2009
 *      Author: zhaot
 */

#include <QtGui>
#include "tz_cdefs.h"
#include "zellipse.h"

ZEllipse::ZEllipse(const QPointF &center, double rx, double ry)
{
  m_center = center;
  m_rx = rx;
  m_ry = ry;
}

void ZEllipse::display(QPainter &painter, int z, Display_Style option) const
{
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);

  painter.setPen(QPen(QColor(255, 0, 0, 32), .7));
  painter.drawEllipse(m_center, m_rx, m_ry);
}

void ZEllipse::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZEllipse::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

ZINTERFACE_DEFINE_CLASS_NAME(ZEllipse)
