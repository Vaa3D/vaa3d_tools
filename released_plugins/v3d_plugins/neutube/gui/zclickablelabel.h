#ifndef ZCLICKABLELABEL_H
#define ZCLICKABLELABEL_H

#include <QLabel>
class ZColorMapParameter;
class Z3DTransferFunctionParameter;
class ZVec4Parameter;
class ZVec3Parameter;
class ZDVec4Parameter;
class ZDVec3Parameter;

class ZClickableLabel : public QWidget
{
  Q_OBJECT
public:
  explicit ZClickableLabel(QWidget *parent = 0, Qt::WindowFlags f = 0);
  
signals:
  void clicked();

public slots:

protected:
  virtual void mousePressEvent(QMouseEvent *ev);
  virtual bool event(QEvent *event);
  virtual bool getTip(const QPoint &p, QRect *r, QString *s) = 0;
  // default implement is emit the signal
  virtual void labelClicked();
};

class ZClickableColorLabel : public ZClickableLabel
{
public:
  explicit ZClickableColorLabel(ZVec4Parameter *color, QWidget *parent = 0, Qt::WindowFlags f = 0);
  explicit ZClickableColorLabel(ZVec3Parameter *color, QWidget *parent = 0, Qt::WindowFlags f = 0);
  explicit ZClickableColorLabel(ZDVec4Parameter *color, QWidget *parent = 0, Qt::WindowFlags f = 0);
  explicit ZClickableColorLabel(ZDVec3Parameter *color, QWidget *parent = 0, Qt::WindowFlags f = 0);
protected:
  virtual void paintEvent(QPaintEvent * e);
  virtual QSize minimumSizeHint() const;
  ZVec4Parameter* m_vec4Color;
  ZVec3Parameter* m_vec3Color;
  ZDVec4Parameter* m_dvec4Color;
  ZDVec3Parameter* m_dvec3Color;
  virtual bool getTip(const QPoint &p, QRect *r, QString *s);
  virtual void labelClicked();
private:
  QColor toQColor();
  void fromQColor(const QColor &col);
};

class ZClickableColorMapLabel : public ZClickableLabel
{
public:
  explicit ZClickableColorMapLabel(ZColorMapParameter *colorMap, QWidget *parent = NULL,
                                   Qt::WindowFlags f = 0);
protected:
  virtual void paintEvent(QPaintEvent * e);
  virtual QSize minimumSizeHint() const;
  ZColorMapParameter *m_colorMap;
  virtual bool getTip(const QPoint &p, QRect *r, QString *s);
};

class ZClickableTransferFunctionLabel : public ZClickableLabel
{
public:
  explicit ZClickableTransferFunctionLabel(Z3DTransferFunctionParameter *transferFunc, QWidget *parent = NULL,
                                   Qt::WindowFlags f = 0);
protected:
  virtual void paintEvent(QPaintEvent * e);
  virtual QSize minimumSizeHint() const;
  Z3DTransferFunctionParameter* m_transferFunction;
  virtual bool getTip(const QPoint &p, QRect *r, QString *s);
};


#endif // ZCLICKABLELABEL_H
