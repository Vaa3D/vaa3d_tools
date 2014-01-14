#ifndef ZSPINBOX_H
#define ZSPINBOX_H

#include <QSpinBox>

// prevent spinbox steal mouse wheel event in a scroll area

class ZSpinBoxEventFilter : public QObject
{
public:
  ZSpinBoxEventFilter(QObject *parent = 0);
protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);
};

class ZSpinBox : public QSpinBox
{
  Q_OBJECT
public:
  ZSpinBox(QWidget* parent = 0);

  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;

public slots:
  void setDataRange(int min, int max);

protected:
  virtual void focusInEvent(QFocusEvent *e);
  virtual void focusOutEvent(QFocusEvent *e);
};


class ZDoubleSpinBox : public QDoubleSpinBox
{
  Q_OBJECT
public:
  ZDoubleSpinBox(QWidget* parent = 0);

  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;

public slots:
  void setDataRange(double min, double max);

protected:
  virtual void focusInEvent(QFocusEvent *e);
  virtual void focusOutEvent(QFocusEvent *e);
};

#endif // ZSPINBOX_H
