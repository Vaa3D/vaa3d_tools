#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif

#include "zslider.h"

#define USE_TOOL_BUTTON 1

ZSlider::ZSlider(bool useArrow, QWidget *parent) : QWidget(parent)
{
  m_slider = new QScrollBar(Qt::Horizontal, this);
  //m_slider->setTickInterval(1);
  m_slider->setSingleStep(1);
  m_slider->setPageStep(5);
  m_slider->setRange(0, 0);
  m_slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  m_slider->setFocusPolicy(Qt::NoFocus);

  m_label = new QLabel(this);
  m_label->setText(QString("100 / 100"));
  m_label->setMinimumSize(m_label->sizeHint());

  m_label->setText(QString("%1 / %2").arg(m_slider->value()).
                   arg(m_slider->maximum()));
  m_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  connect(m_slider, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));
  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(setText(int)));

  if (useArrow) {
#if USE_TOOL_BUTTON
    m_goLeftButton = new QToolButton;
    m_goLeftButton->setIcon(QIcon(":/images/go-previous.png"));
    m_goLeftButton->setStyleSheet("border: none");
    //m_goLeftButton->setEnabled(false); //This messes subwindow stacks for unknown reason
    m_goRightButton = new QToolButton;
    m_goRightButton->setIcon(QIcon(":/images/go-next.png"));
    m_goRightButton->setStyleSheet("border: none");
    //m_goRightButton->setEnabled(false);
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(updateArrowState(int)));
    connect(m_goLeftButton, SIGNAL(clicked()), this, SLOT(goLeftButtonClicked()));
    connect(m_goRightButton, SIGNAL(clicked()), this, SLOT(goRightButtonClicked()));
#else
    m_goLeftButtonSimple = new QPushButton;
    m_goLeftButtonSimple->setText("<");
    //m_goLeftButtonSimple->setEnabled(false);
    m_goRightButtonSimple = new QPushButton;
    m_goRightButtonSimple->setText(">");
    //m_goRightButtonSimple->setEnabled(false);
    connect(m_goLeftButtonSimple, SIGNAL(clicked()), this, SLOT(goLeftButtonClicked()));
    connect(m_goRightButtonSimple, SIGNAL(clicked()), this, SLOT(goRightButtonClicked()));
#endif

    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(updateArrowState(int)));
  }

  m_layout = new QHBoxLayout;
  if (useArrow) {
#if USE_TOOL_BUTTON
    m_layout->addWidget(m_goLeftButton);
    m_layout->addWidget(m_goRightButton);
#else
    m_layout->addWidget(m_goLeftButtonSimple);
    m_layout->addWidget(m_goRightButtonSimple);
#endif
  }
  m_layout->addWidget(m_slider);
  m_layout->addWidget(m_label);

  setLayout(m_layout);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setFocusPolicy(Qt::NoFocus);
}

void ZSlider::setRange(int min, int max)
{
  m_slider->setRange(min, max);
  setText(m_slider->value());
  //m_spinBox->setRange(min, max);
  //m_spinBox->setSuffix(QString(" /    %1").arg(m_spinBox->maximum()));
}

void ZSlider::setRangeQuietly(int min, int max)
{
  disconnect(m_slider, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));
  m_slider->setRange(min, max);
  setText(m_slider->value());
  connect(m_slider, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));
}

void ZSlider::setText(int value)
{
  m_label->setText(QString("%1 / %2").arg(value, 3, 10, QChar(' ')).
                   arg(m_slider->maximum()));
  m_label->setTextFormat(Qt::PlainText);
}

void ZSlider::updateArrowState(int value)
{
#if USE_TOOL_BUTTON
  m_goLeftButton->setEnabled(value != 0);
  m_goRightButton->setEnabled(value != m_slider->maximum());
#else
  m_goLeftButtonSimple->setEnabled(value != 0);
  m_goRightButtonSimple->setEnabled(value != m_slider->maximum());
#endif
}

void ZSlider::goLeftButtonClicked()
{
  setValue(m_slider->value() - 1);
}

void ZSlider::goRightButtonClicked()
{
  setValue(m_slider->value() + 1);
}

void ZSlider::setValue(int value)
{
  if (value < minimum()) {
    value = minimum();
  } else if (value > maximum()) {
    value = maximum();
  }

  if (m_slider->value() != value) {
    m_slider->setValue(value);
  }
}

void ZSlider::setValueQuietly(int value)
{
  if (m_slider->value() != value) {
    disconnect(m_slider, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));
    m_slider->setValue(value);
    qDebug() << "Value: " << value << '\n';
    connect(m_slider, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));
  }
}

int ZSlider::value()
{
  return m_slider->value();
}

int ZSlider::minimum()
{
  return m_slider->minimum();
}

int ZSlider::maximum()
{
  return m_slider->maximum();
}
