#ifndef ZSPINBOXWITHSLIDER_H
#define ZSPINBOXWITHSLIDER_H

#include <QWidget>
#include <QSlider>
#include <Qt>
class ZSpinBox;
class ZDoubleSpinBox;

class ZSliderEventFilter : public QObject
{
public:
  ZSliderEventFilter(QObject *parent = 0);
protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);
};

class ZSlider2 : public QSlider
{
  Q_OBJECT
public:
  ZSlider2(QWidget* parent = 0);
  ZSlider2(Qt::Orientation ori, QWidget* parent = 0);

public slots:
  void setDataRange(int min, int max);

protected:
  virtual void focusInEvent(QFocusEvent *event);
  virtual void focusOutEvent(QFocusEvent *event);
};

class ZSpinBoxWithSlider : public QWidget
{
  Q_OBJECT
public:
  explicit ZSpinBoxWithSlider(int value, int min, int max, int step = 1,
                              bool tracking = true, QWidget *parent = 0);
  
signals:
  void valueChanged(int);
  
public slots:
  void setValue(int v);
  void setDataRange(int min, int max);

private slots:
  void valueChangedFromSlider(int v);
  void valueChangedFromSpinBox(int v);

protected:
  void createWidget(int value, int min, int max, int step, bool tracking);

  ZSlider2* m_slider;
  ZSpinBox* m_spinBox;
};

class ZDoubleSpinBoxWithSlider : public QWidget
{
  Q_OBJECT
public:
  explicit ZDoubleSpinBoxWithSlider(double value, double min, double max, double step = .01,
                                    int decimal = 3, bool tracking = true, QWidget *parent = 0);

signals:
  void valueChanged(double);

public slots:
  void setValue(double v);
  void setDataRange(double min, double max);

private slots:
  void valueChangedFromSlider(int v);
  void valueChangedFromSpinBox(double v);

protected:
  void createWidget();

  ZSlider2* m_slider;
  ZDoubleSpinBox* m_spinBox;
  double m_value;
  double m_min;
  double m_max;
  double m_step;
  int m_decimal;
  bool m_tracking;
  int m_sliderMaxValue;
};

#endif // ZSPINBOXWITHSLIDER_H
