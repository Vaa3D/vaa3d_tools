/**@file zcircle.h
 * @brief Circle class
 * @author Ting Zhao
 */
#ifndef ZCIRCLE_H
#define ZCIRCLE_H

#include "zqtheader.h"

#include "zpoint.h"
#include "zdocumentable.h"
#include "zstackdrawable.h"

class ZCircle : public ZDocumentable, public ZStackDrawable {
public:
  ZCircle();
  ZCircle(double x, double y, double z, double r);
  virtual ~ZCircle() {}
  void set(double x, double y, double z, double r);

  virtual const std::string& className() const;

public:

  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL) const;

  virtual void save(const char *filePath);
  virtual void load(const char *filePath);

  void display(QPainter *painter, int n, Display_Style style) const;

private:
  ZPoint m_center;
  double m_r;
};

#endif // ZCIRCLE_H
