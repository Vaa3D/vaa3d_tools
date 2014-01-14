#include "zspinbox.h"
#include <QEvent>
#include "QsLog.h"

ZSpinBox::ZSpinBox(QWidget *parent)
  : QSpinBox(parent)
{
  installEventFilter(new ZSpinBoxEventFilter(this));
  setFocusPolicy(Qt::StrongFocus);
  setKeyboardTracking(false);
}

QSize ZSpinBox::sizeHint() const
{
  QSize size = QSpinBox::sizeHint();
  size.setWidth(std::min(size.width(), 160));
  return size;
}

QSize ZSpinBox::minimumSizeHint() const
{
  QSize size = QSpinBox::minimumSizeHint();
  size.setWidth(std::min(size.width(), 57));
  return size;
}

void ZSpinBox::setDataRange(int min, int max)
{
  setRange(min, max);
}

void ZSpinBox::focusInEvent(QFocusEvent *e)
{
  QSpinBox::focusInEvent(e);
  setFocusPolicy(Qt::WheelFocus);
}

void ZSpinBox::focusOutEvent(QFocusEvent *e)
{
  QSpinBox::focusOutEvent(e);
  setFocusPolicy(Qt::StrongFocus);
}

ZDoubleSpinBox::ZDoubleSpinBox(QWidget *parent)
  : QDoubleSpinBox(parent)
{
  installEventFilter(new ZSpinBoxEventFilter(this));
  setFocusPolicy(Qt::StrongFocus);
  setKeyboardTracking(false);
}

QSize ZDoubleSpinBox::sizeHint() const
{
  QSize size = QDoubleSpinBox::sizeHint();
  size.setWidth(std::min(size.width(), 160));
  return size;
}

QSize ZDoubleSpinBox::minimumSizeHint() const
{
  QSize size = QDoubleSpinBox::minimumSizeHint();
  size.setWidth(std::min(size.width(), 57));
  return size;
}

void ZDoubleSpinBox::setDataRange(double min, double max)
{
  setRange(min, max);
}

void ZDoubleSpinBox::focusInEvent(QFocusEvent *e)
{
  QDoubleSpinBox::focusInEvent(e);
  setFocusPolicy(Qt::WheelFocus);
}

void ZDoubleSpinBox::focusOutEvent(QFocusEvent *e)
{
  QDoubleSpinBox::focusOutEvent(e);
  setFocusPolicy(Qt::StrongFocus);
}

ZSpinBoxEventFilter::ZSpinBoxEventFilter(QObject *parent)
  : QObject(parent)
{
}

bool ZSpinBoxEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  if(event->type() == QEvent::Wheel &&
     qobject_cast<QAbstractSpinBox*>(obj))
  {
    if(qobject_cast<QAbstractSpinBox*>(obj)->focusPolicy() == Qt::WheelFocus)
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
