/**@file zslider.h
 * @brief Slider widget
 * @author Ting Zhao
 */
#ifndef _ZSLIDER_H_
#define _ZSLIDER_H_

#include <QWidget>

class QSlider;
class QSpinBox;
class QHBoxLayout;
class QScrollBar;
class QLabel;
class QToolButton;
class QPushButton;

class ZSlider : public QWidget {
  Q_OBJECT

public:
  ZSlider(bool useArrow = true, QWidget *parent = NULL);

  int value();
  int minimum();
  int maximum();

public slots:
  void setValue(int value);
  void setValueQuietly(int value);
  void setRange(int min, int max);
  void setRangeQuietly(int min, int max);
  void setText(int value);

protected slots:
  void updateArrowState(int value);
  void goLeftButtonClicked();
  void goRightButtonClicked();

signals:
  void valueChanged(int);

private:
  QHBoxLayout *m_layout;
  QScrollBar *m_slider;
  QLabel *m_label;
  //QSpinBox *m_spinBox;
  QToolButton *m_goLeftButton;
  QToolButton *m_goRightButton;

  QPushButton *m_goLeftButtonSimple;
  QPushButton *m_goRightButtonSimple;
};

#endif
