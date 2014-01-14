#include "zparameter.h"
#include <QWidget>
#include <QLabel>
#include <QCheckBox>

ZParameter::ZParameter(const QString& name, QObject *parent) :
  QObject(parent), m_name(name), m_style("DEFAULT"), m_isWidgetsEnabled(true), m_isWidgetsVisible(true)
, m_mainWin(NULL)
{
  addStyle("DEFAULT");
}

ZParameter::~ZParameter()
{
}

void ZParameter::setName(const QString &name)
{
  if (name != m_name) {
    m_name = name;
    emit nameChanged(m_name);
  }
}

void ZParameter::setStyle(const QString &style)
{
  if (m_allStyles.contains(style))
    m_style = style;
  else
    m_style = "DEFAULT";
}

QLabel *ZParameter::createNameLabel(QWidget *parent)
{
  QLabel *label = new QLabel(m_name, parent);
  if (!m_isWidgetsVisible)
    label->setVisible(m_isWidgetsVisible);
  if (!m_isWidgetsEnabled)
    label->setEnabled(m_isWidgetsEnabled);
  connect(this, SIGNAL(setWidgetsEnabled(bool)), label, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(setWidgetsVisible(bool)), label, SLOT(setVisible(bool)));
  connect(this, SIGNAL(nameChanged(QString)), label, SLOT(setText(QString)));
  return label;
}

QWidget *ZParameter::createWidget(QWidget *parent)
{
  QWidget* widget = actualCreateWidget(parent);
  if (!m_isWidgetsVisible)
    widget->setVisible(m_isWidgetsVisible);
  if (!m_isWidgetsEnabled)
    widget->setEnabled(m_isWidgetsEnabled);
  connect(this, SIGNAL(setWidgetsEnabled(bool)), widget, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(setWidgetsVisible(bool)), widget, SLOT(setVisible(bool)));
#ifdef __APPLE__
  widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
#endif
  return widget;
}

void ZParameter::setVisible(bool s)
{
  if (s != m_isWidgetsVisible) {
    m_isWidgetsVisible = s;
    emit setWidgetsVisible(m_isWidgetsVisible);
  }
}

void ZParameter::setEnabled(bool s)
{
  if (s != m_isWidgetsEnabled) {
    m_isWidgetsEnabled = s;
    emit setWidgetsEnabled(m_isWidgetsEnabled);
  }
}

ZBoolParameter::ZBoolParameter(const QString &name, bool value, QObject *parent)
  : ZSingleValueParameter<bool>(name, value, parent)
{
}

void ZBoolParameter::setValue(bool v)
{
  set(v);
}

void ZBoolParameter::beforeChange(bool &value)
{
  emit valueChanged(value);
}

QWidget *ZBoolParameter::actualCreateWidget(QWidget *parent)
{
  QCheckBox* cb = new QCheckBox(parent);
  cb->setChecked(m_value);
  connect(cb, SIGNAL(toggled(bool)), this, SLOT(setValue(bool)));
  connect(this, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)));
  return cb;
}
