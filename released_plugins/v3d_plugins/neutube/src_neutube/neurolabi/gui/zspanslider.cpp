#include "zspanslider.h"
#include <QBoxLayout>

ZSpanSliderWithSpinBox::ZSpanSliderWithSpinBox(int lowerValue, int upperValue, int min, int max,
                                               int singleStep, bool tracking, QWidget *parent)
  : QWidget(parent)
{
  createWidget(lowerValue, upperValue, min, max, singleStep, tracking);
}


void ZSpanSliderWithSpinBox::setLowerValue(int lower)
{
  m_slider->setLowerValue(lower);
  m_lowerSpinBox->setValue(lower);
}

void ZSpanSliderWithSpinBox::setUpperValue(int upper)
{
  m_slider->setUpperValue(upper);
  m_upperSpinBox->setValue(upper);
}

void ZSpanSliderWithSpinBox::setDataRange(int min, int max)
{
  m_slider->setRange(min, max);
  m_lowerSpinBox->setRange(m_slider->minimum(), m_slider->upperValue());
  m_lowerSpinBox->setValue(m_slider->lowerValue());
  m_upperSpinBox->setRange(m_slider->lowerValue(), m_slider->maximum());
  m_upperSpinBox->setValue(m_slider->upperValue());
}

void ZSpanSliderWithSpinBox::lowerValueChangedFromSlider(int l)
{
  m_lowerSpinBox->blockSignals(true);
  m_lowerSpinBox->setValue(l);
  m_lowerSpinBox->blockSignals(false);
  m_upperSpinBox->setMinimum(l);
  emit lowerValueChanged(l);
}

void ZSpanSliderWithSpinBox::upperValueChangedFromSlider(int u)
{
  m_upperSpinBox->blockSignals(true);
  m_upperSpinBox->setValue(u);
  m_upperSpinBox->blockSignals(false);
  m_lowerSpinBox->setMaximum(u);
  emit upperValueChanged(u);
}

void ZSpanSliderWithSpinBox::valueChangedFromLowerSpinBox(int l)
{
  m_slider->blockSignals(true);
  m_slider->setLowerValue(l);
  m_slider->blockSignals(false);
  emit lowerValueChanged(l);
}

void ZSpanSliderWithSpinBox::valueChangedFromUpperSpinBox(int u)
{
  m_slider->blockSignals(true);
  m_slider->setUpperValue(u);
  m_slider->blockSignals(false);
  emit upperValueChanged(u);
}

void ZSpanSliderWithSpinBox::createWidget(int lowerValue, int upperValue, int min, int max, int singleStep, bool tracking)
{
  m_slider = new QxtSpanSlider(Qt::Horizontal);
  m_slider->setRange(min, max);
  m_slider->setLowerValue(lowerValue);
  m_slider->setUpperValue(upperValue);
  m_slider->setSingleStep(singleStep);
  m_slider->setTracking(tracking);
  m_slider->setHandleMovementMode(QxtSpanSlider::NoCrossing);
  m_slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  m_lowerSpinBox = new ZSpinBox();
  m_lowerSpinBox->setRange(min, upperValue);
  m_lowerSpinBox->setValue(lowerValue);
  m_lowerSpinBox->setSingleStep(singleStep);
  m_upperSpinBox = new ZSpinBox();
  m_upperSpinBox->setRange(lowerValue, max);
  m_upperSpinBox->setValue(upperValue);
  m_upperSpinBox->setSingleStep(singleStep);
  QHBoxLayout* lo = new QHBoxLayout(this);
  lo->setContentsMargins(0,0,0,0);
  lo->addWidget(m_lowerSpinBox);
  lo->addWidget(m_slider);
  lo->addWidget(m_upperSpinBox);
  connect(m_slider, SIGNAL(lowerValueChanged(int)), this, SLOT(lowerValueChangedFromSlider(int)));
  connect(m_slider, SIGNAL(upperValueChanged(int)), this, SLOT(upperValueChangedFromSlider(int)));
  connect(m_lowerSpinBox, SIGNAL(valueChanged(int)), this, SLOT(valueChangedFromLowerSpinBox(int)));
  connect(m_upperSpinBox, SIGNAL(valueChanged(int)), this, SLOT(valueChangedFromUpperSpinBox(int)));
}


ZDoubleSpanSliderWithSpinBox::ZDoubleSpanSliderWithSpinBox(double lowerValue, double upperValue, double min, double max,
                                                           double singleStep, int decimal, bool tracking, QWidget *parent)
  : QWidget(parent), m_lowerValue(lowerValue), m_upperValue(upperValue), m_min(min),
    m_max(max), m_step(singleStep), m_decimal(decimal), m_tracking(tracking)
{
  m_sliderMaxValue = static_cast<int>(std::max(500000.0, (m_max-m_min)/m_step));
  createWidget();
}


void ZDoubleSpanSliderWithSpinBox::setLowerValue(double lower)
{
  double l = (lower - m_min) / (m_max - m_min) * m_sliderMaxValue;
  m_slider->setLowerValue(l);
  m_lowerSpinBox->setValue(lower);
}

void ZDoubleSpanSliderWithSpinBox::setUpperValue(double upper)
{
  double u = (upper - m_min) / (m_max - m_min) * m_sliderMaxValue;
  m_slider->setUpperValue(u);
  m_upperSpinBox->setValue(upper);
}

void ZDoubleSpanSliderWithSpinBox::setDataRange(double min, double max)
{
  m_min = min;
  m_max = max;
  m_sliderMaxValue = static_cast<int>(std::max(50000.0, (m_max-m_min)/m_step));
  m_slider->setRange(0, m_sliderMaxValue);
  double l = m_slider->lowerValue() / (double)m_sliderMaxValue * (m_max-m_min) + m_min;
  double u = m_slider->upperValue() / (double)m_sliderMaxValue * (m_max-m_min) + m_min;
  m_lowerSpinBox->setRange(m_min, u);
  m_lowerSpinBox->setValue(l);
  m_upperSpinBox->setRange(l, m_max);
  m_upperSpinBox->setValue(u);
}

void ZDoubleSpanSliderWithSpinBox::lowerValueChangedFromSlider(int l)
{
  m_lowerValue = l / (double)m_sliderMaxValue * (m_max - m_min) + m_min;
  m_lowerSpinBox->blockSignals(true);
  m_lowerSpinBox->setValue(m_lowerValue);
  m_lowerSpinBox->blockSignals(false);
  m_upperSpinBox->setMinimum(m_lowerValue);
  emit lowerValueChanged(m_lowerValue);
}

void ZDoubleSpanSliderWithSpinBox::upperValueChangedFromSlider(int u)
{
  m_upperValue = u / (double)m_sliderMaxValue * (m_max - m_min) + m_min;
  m_upperSpinBox->blockSignals(true);
  m_upperSpinBox->setValue(m_upperValue);
  m_upperSpinBox->blockSignals(false);
  m_lowerSpinBox->setMaximum(m_upperValue);
  emit upperValueChanged(m_upperValue);
}

void ZDoubleSpanSliderWithSpinBox::valueChangedFromLowerSpinBox(double l)
{
  m_lowerValue = l;
  int sliderPos = static_cast<int>((m_lowerValue - m_min) / (m_max - m_min) * m_sliderMaxValue);
  m_slider->blockSignals(true);
  m_slider->setLowerValue(sliderPos);
  m_slider->blockSignals(false);
  emit lowerValueChanged(m_lowerValue);
}

void ZDoubleSpanSliderWithSpinBox::valueChangedFromUpperSpinBox(double u)
{
  m_upperValue = u;
  int sliderPos = static_cast<int>((m_upperValue - m_min) / (m_max - m_min) * m_sliderMaxValue);
  m_slider->blockSignals(true);
  m_slider->setLowerValue(sliderPos);
  m_slider->blockSignals(false);
  emit upperValueChanged(m_upperValue);
}

void ZDoubleSpanSliderWithSpinBox::createWidget()
{
  m_slider = new QxtSpanSlider(Qt::Horizontal);
  m_slider->setRange(0, m_sliderMaxValue);
  m_slider->setLowerValue(static_cast<int>((m_lowerValue - m_min) / (m_max - m_min) * m_sliderMaxValue));
  m_slider->setUpperValue(static_cast<int>((m_upperValue - m_min) / (m_max - m_min) * m_sliderMaxValue));
  m_slider->setSingleStep(std::max(1, static_cast<int>(m_step*m_sliderMaxValue/(m_max-m_min))));
  m_slider->setTracking(m_tracking);
  m_slider->setHandleMovementMode(QxtSpanSlider::NoCrossing);
  m_slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  m_lowerSpinBox = new ZDoubleSpinBox();
  m_lowerSpinBox->setRange(m_min, m_upperValue);
  m_lowerSpinBox->setValue(m_lowerValue);
  m_lowerSpinBox->setSingleStep(m_step);
  m_lowerSpinBox->setDecimals(m_decimal);
  m_upperSpinBox = new ZDoubleSpinBox();
  m_upperSpinBox->setRange(m_lowerValue, m_max);
  m_upperSpinBox->setValue(m_upperValue);
  m_upperSpinBox->setSingleStep(m_step);
  m_upperSpinBox->setDecimals(m_decimal);
  QHBoxLayout* lo = new QHBoxLayout(this);
  lo->setContentsMargins(0,0,0,0);
  lo->addWidget(m_lowerSpinBox);
  lo->addWidget(m_slider);
  lo->addWidget(m_upperSpinBox);
  connect(m_slider, SIGNAL(lowerValueChanged(int)), this, SLOT(lowerValueChangedFromSlider(int)));
  connect(m_slider, SIGNAL(upperValueChanged(int)), this, SLOT(upperValueChangedFromSlider(int)));
  connect(m_lowerSpinBox, SIGNAL(valueChanged(double)), this, SLOT(valueChangedFromLowerSpinBox(double)));
  connect(m_upperSpinBox, SIGNAL(valueChanged(double)), this, SLOT(valueChangedFromUpperSpinBox(double)));
}
