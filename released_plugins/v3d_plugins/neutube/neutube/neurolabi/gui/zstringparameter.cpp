#include "zstringparameter.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif

ZStringParameter::ZStringParameter(const QString &name, const QString &str, QWidget *parent)
  : ZSingleValueParameter<QString>(name, str, parent)
{
}

void ZStringParameter::setContent(QString str)
{
  set(str);
}

QWidget *ZStringParameter::actualCreateWidget(QWidget *parent)
{
  QLineEdit* le = new QLineEdit(parent);
  le->setText(m_value);
  connect(le, SIGNAL(textChanged(QString)), this, SLOT(setContent(QString)));
  connect(this, SIGNAL(strChanged(QString)), le, SLOT(setText(QString)));
  return le;
}

void ZStringParameter::beforeChange(QString &value)
{
  emit strChanged(value);
}
