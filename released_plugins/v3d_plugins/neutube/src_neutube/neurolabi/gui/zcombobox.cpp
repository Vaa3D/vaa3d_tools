#include "zcombobox.h"
#include <QEvent>
#include "QsLog.h"

ZComboBox::ZComboBox(QWidget *parent) :
  QComboBox(parent)
{
  installEventFilter(new ZComboBoxEventFilter(this));
  setFocusPolicy(Qt::StrongFocus);
}

QSize ZComboBox::sizeHint() const
{
  QSize size = QComboBox::sizeHint();
  size.setWidth(std::min(size.width(), 160));
  return size;
}

QSize ZComboBox::minimumSizeHint() const
{
  QSize size = QComboBox::minimumSizeHint();
  size.setWidth(std::min(size.width(), 80));
  return size;
}

void ZComboBox::addItemSlot(const QString &text)
{
  //LINFO() << text;
  addItem(text);
}

void ZComboBox::removeItemSlot(const QString &text)
{
  removeItem(findText(text));
}

void ZComboBox::focusInEvent(QFocusEvent *)
{
  setFocusPolicy(Qt::WheelFocus);
}

void ZComboBox::focusOutEvent(QFocusEvent *)
{
  setFocusPolicy(Qt::StrongFocus);
}

ZComboBoxEventFilter::ZComboBoxEventFilter(QObject *parent)
  : QObject(parent)
{
}

bool ZComboBoxEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  if(event->type() == QEvent::Wheel &&
     qobject_cast<QComboBox*>(obj))
  {
    if(qobject_cast<QComboBox*>(obj)->focusPolicy() == Qt::WheelFocus)
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
