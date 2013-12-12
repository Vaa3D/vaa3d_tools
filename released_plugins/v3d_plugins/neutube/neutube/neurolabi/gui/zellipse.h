/**@file zellipse.h
 * @brief Ellipse class
 * @author Ting Zhao
 * @date 01-JUN-2009
 */

#ifndef _ZELLIPSE_H_
#define _ZELLIPSE_H_

#include <QPointF>

#include "zdocumentable.h"
#include "zstackdrawable.h"

class ZEllipse : public ZDocumentable, public ZStackDrawable {
public:
  ZEllipse(const QPointF &center, double rx, double ry);

  virtual const std::string& className() const;

public:
  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL)
  const;

  virtual void save(const char *filePath);
  virtual void load(const char *filePath);

private:
  QPointF m_center;
  double m_rx;
  double m_ry;
};

#endif /* _ZELLIPSE_H_ */
