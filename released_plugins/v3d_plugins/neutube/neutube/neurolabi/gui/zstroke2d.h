#ifndef ZSTROKE2D_H
#define ZSTROKE2D_H

#include <QPointF>
#include "zdocumentable.h"
#include "zstackdrawable.h"
#include <vector>

class ZStroke2d : public ZDocumentable, public ZStackDrawable {
public:
  ZStroke2d();
  ZStroke2d(const ZStroke2d &stroke);
  virtual ~ZStroke2d();

public:
  virtual void save(const char *filePath);
  virtual void load(const char *filePath);

  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL) const;

  virtual const std::string& className() const;

  inline void setWidth(double width) { m_width = width; }
  void append(double x, double y);
  void set(const QPoint &pt);
  void set(double x, double y);

  void clear();

  bool isEmpty();

  ZStroke2d* clone();

  void addWidth(double dw);

  void setEraser(bool isEraser);
  inline bool isEraser() { return m_isEraser; }
  inline void setFilled(bool isFilled) {
    m_isFilled = isFilled;
  }

  double inline getWidth() { return m_width; }

  bool getLastPoint(int *x, int *y) const;

  inline size_t getPointNumber() const { return m_pointArray.size(); }

private:
  std::vector<QPointF> m_pointArray;
  double m_width;
  bool m_isEraser;
  bool m_isFilled;

  static const double m_minWidth;
  static const double m_maxWidth;
};

#endif // ZSTROKE2D_H
