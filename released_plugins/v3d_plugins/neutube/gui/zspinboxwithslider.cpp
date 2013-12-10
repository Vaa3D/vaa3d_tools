#include "zspinboxwithslider.h"
#include <QHBoxLayout>
#include "zspinbox.h"
#include <QEvent>
#include "QsLog.h"

ZSliderEventFilter::ZSliderEventFilter(QObject *parent)
  : QObject(parent)
{
}

bool ZSliderEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  if(event->type() == QEvent::Wheel &&
     qobject_cast<QAbstractSlider*>(obj))
  {
    if(qobject_cast<QAbstractSlider*>(obj)->focusPolicy() == Qt::WheelFocus)
    {
      event->accept();
      return false;
    }
    else
    {
      event->ignore();
      return true;
    }
  }
  return QObject::eventFilter(obj, event);
}

ZSlider2::ZSlider2(QWidget *parent)
  : QSlider(Qt::Horizontal, parent)
{
  installEventFilter(new ZSliderEventFilter(this));
  setFocusPolicy(Qt::StrongFocus);
}

ZSlider2::ZSlider2(Qt::Orientation ori, QWidget *parent)
  : QSlider(ori, parent)
{
}

void ZSlider2::setDataRange(int min, int max)
{
  setRange(min, max);
}

void ZSlider2::focusInEvent(QFocusEvent *e)
{
  QSlider::focusInEvent(e);
  setFocusPolicy(Qt::WheelFocus);
}

void ZSlider2::focusOutEvent(QFocusEvent *e)
{
  QSlider::focusOutEvent(e);
  setFocusPolicy(Qt::StrongFocus);
}

ZSpinBoxWithSlider::ZSpinBoxWithSlider(int value, int min, int max, int step, bool tracking, QWidget *parent)
  : QWidget(parent)
{
  createWidget(value, min, max, step, tracking);
}

void ZSpinBoxWithSlider::setValue(int v)
{
  m_slider->setValue(v);
  m_spinBox->setValue(v);
}

void ZSpinBoxWithSlider::valueChangedFromSlider(int v)
{
  m_spinBox->blockSignals(true);
  m_spinBox->setValue(v);
  m_spinBox->blockSignals(false);
  emit valueChanged(v);
}

void ZSpinBoxWithSlider::valueChangedFromSpinBox(int v)
{
  m_slider->blockSignals(true);
  m_slider->setValue(v);
  m_slider->blockSignals(false);
  emit valueChanged(v);
}

void ZSpinBoxWithSlider::setDataRange(int min, int max)
{
  m_slider->setRange(min, max);
  m_spinBox->setRange(min, max);
}

void ZSpinBoxWithSlider::createWidget(int value, int min, int max, int step, bool tracking)
{
  m_slider = new ZSlider2();
  m_slider->setRange(min, max);
  m_slider->setValue(value);
  m_slider->setSingleStep(step);
  m_slider->setTracking(tracking);
  m_spinBox = new ZSpinBox();
  m_spinBox->setRange(min, max);
  m_spinBox->setValue(value);
  m_spinBox->setSingleStep(step);
  QHBoxLayout* lo = new QHBoxLayout(this);
  lo->setContentsMargins(0,0,0,0);
  lo->addWidget(m_spinBox);
  lo->addWidget(m_slider);
  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(valueChangedFromSlider(int)));
  connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(valueChangedFromSpinBox(int)));
}

ZDoubleSpinBoxWithSlider::ZDoubleSpinBoxWithSlider(double value, double min, double max, double step,
                                                   int decimal, bool tracking, QWidget *parent)
  : QWidget(parent), m_value(value), m_min(min),
    m_max(max), m_step(step), m_decimal(decimal), m_tracking(tracking)
{
  m_sliderMaxValue = static_cast<int>(std::max(50000.0, (m_max-m_min)/m_step));
  createWidget();
}

void ZDoubleSpinBoxWithSlider::setValue(double v)
{
  m_spinBox->setValue(v);
}

void ZDoubleSpinBoxWithSlider::valueChangedFromSlider(int v)
{
  m_value = static_cast<double>(v)/m_sliderMaxValue * (m_max - m_min) + m_min;
  m_spinBox->blockSignals(true);
  m_spinBox->setValue(m_value);
  m_spinBox->blockSignals(false);
  emit valueChanged(m_value);
}

void ZDoubleSpinBoxWithSlider::valueChangedFromSpinBox(double v)
{
  m_value = v;
  int sliderPos = static_cast<int>((m_value - m_min) / (m_max - m_min) * m_sliderMaxValue);
  m_slider->blockSignals(true);
  m_slider->setValue(sliderPos);
  m_slider->blockSignals(false);
  emit valueChanged(m_value);
}

void ZDoubleSpinBoxWithSlider::setDataRange(double min, double max)
{
  m_min = min;
  m_max = max;
  m_sliderMaxValue = static_cast<int>(std::max(50000.0, (m_max-m_min)/m_step));
  m_slider->setRange(0, m_sliderMaxValue);
  m_spinBox->setRange(m_min, m_max);
}

void ZDoubleSpinBoxWithSlider::createWidget()
{
  m_slider = new ZSlider2();
  m_slider->setRange(0, m_sliderMaxValue);
  m_slider->setValue(static_cast<int>((m_value - m_min) / (m_max - m_min) * m_sliderMaxValue));
  m_slider->setSingleStep(std::max(1, static_cast<int>(m_step*m_sliderMaxValue/(m_max-m_min))));
  m_slider->setTracking(m_tracking);
  m_spinBox = new ZDoubleSpinBox();
  m_spinBox->setRange(m_min, m_max);
  m_spinBox->setValue(m_value);
  m_spinBox->setSingleStep(m_step);
  m_spinBox->setDecimals(m_decimal);
  QHBoxLayout* lo = new QHBoxLayout(this);
  lo->setContentsMargins(0,0,0,0);
  lo->addWidget(m_spinBox);
  lo->addWidget(m_slider);
  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(valueChangedFromSlider(int)));
  connect(m_spinBox, SIGNAL(valueChanged(double)), this, SLOT(valueChangedFromSpinBox(double)));
}
