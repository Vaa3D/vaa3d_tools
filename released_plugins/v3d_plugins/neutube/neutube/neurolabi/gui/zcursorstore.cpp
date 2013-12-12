#include "zcursorstore.h"

#include <QPainter>
#include <QPen>

ZCursorStore::ZCursorStore() :
  m_circleCursor5(circleCursorBitmap(5)),
  m_circleCursor10(circleCursorBitmap(10)),
  m_circleCursor15(circleCursorBitmap(15)),
  m_circleCursor20(circleCursorBitmap(20)),
  m_smallCrossCursor(smallCrossCursorBitmap())
{

}

ZCursorStore::ZCursorStore(const ZCursorStore&)
{
}

void ZCursorStore::operator=(const ZCursorStore&)
{
}

const QCursor& ZCursorStore::getCircleCursor(int radius)
{
  if (radius <= 5) {
    return m_circleCursor5;
  } else if (radius <= 10) {
    return m_circleCursor10;
  } else if (radius <= 15) {
    return m_circleCursor15;
  } else {
    return m_circleCursor20;
  }
}

int ZCursorStore::prevCircleRadius(int radius)
{
  radius -= 5;
  if (radius < minCircleCursorRadius()) {
    radius = minCircleCursorRadius();
  }

  return radius;
}

int ZCursorStore::nextCircleRadius(int radius)
{
  radius += 5;
  if (radius > maxCircleCursorRadius()) {
    radius = maxCircleCursorRadius();
  }

  return radius;
}

QBitmap ZCursorStore::circleCursorBitmap(int radius)
{
  QBitmap bitmap(32, 32);
  QPoint center = QPoint(bitmap.width() / 2, bitmap.height() / 2);
  bitmap.fill(Qt::transparent);
  QPainter painter(&bitmap);
  QPen pen;

  pen.setColor(Qt::color0);
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);
  painter.drawEllipse(center, 64, 64);
  painter.drawEllipse(center, radius, radius);
  pen.setColor(Qt::color1);
  painter.setPen(pen);
  painter.drawEllipse(center, radius - 1, radius - 1);


  return bitmap;
}

QBitmap ZCursorStore::smallCrossCursorBitmap()
{
  QBitmap bitmap(32, 32);
  bitmap.fill(Qt::transparent);
  QPainter painter(&bitmap);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(Qt::NoPen);
  painter.drawRect(0, 0, 32, 32);

  QPen pen;
  pen.setColor(Qt::color0);
  painter.setPen(pen);

  int minCorner = 15;
  int maxCorner = minCorner + 3;
  painter.drawPoint(minCorner, minCorner);
  painter.drawPoint(minCorner, maxCorner);
  painter.drawPoint(maxCorner, minCorner);
  painter.drawPoint(maxCorner, maxCorner);

  pen.setColor(Qt::color1);
  painter.setPen(pen);
  minCorner = 14;
  maxCorner = minCorner + 5;
  painter.drawPoint(minCorner, minCorner);
  painter.drawPoint(minCorner, maxCorner);
  painter.drawPoint(maxCorner, minCorner);
  painter.drawPoint(maxCorner, maxCorner);

  return bitmap;
}
