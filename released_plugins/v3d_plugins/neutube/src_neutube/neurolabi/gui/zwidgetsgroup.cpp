#include "zwidgetsgroup.h"
#include "z3dcameraparameter.h"
#include "zparameter.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif

ZWidgetsGroup::ZWidgetsGroup(QWidget *widget, ZWidgetsGroup *parentGroup, int visibleLevel, QObject *parent)
  : QObject(parent)
  , m_type(WIDGET)
  , m_groupName("This is not a group")
  , m_widget(widget)
  , m_parameter(NULL)
  , m_parent(parentGroup)
  , m_visibleLevel(visibleLevel)
  , m_isSorted(true)
  , m_isVisible(true)
{
  if (parentGroup)
    parentGroup->addChildGroup(this);
}

ZWidgetsGroup::ZWidgetsGroup(const QString &groupName, ZWidgetsGroup *parentGroup, int visibleLevel, QObject *parent)
  : QObject(parent)
  , m_type(GROUP)
  , m_groupName(groupName)
  , m_widget(NULL)
  , m_parameter(NULL)
  , m_parent(parentGroup)
  , m_visibleLevel(visibleLevel)
  , m_isSorted(false)
  , m_cutOffbetweenBasicAndAdvancedLevel(1)
  , m_isVisible(true)
{
  if (parentGroup)
    parentGroup->addChildGroup(this);
}

ZWidgetsGroup::ZWidgetsGroup(ZParameter *parameter, ZWidgetsGroup *parentGroup, int visibleLevel, QObject *parent)
  : QObject(parent)
  , m_type(PARAMETER)
  , m_groupName("This is not a group")
  , m_widget(NULL)
  , m_parameter(parameter)
  , m_parent(parentGroup)
  , m_visibleLevel(visibleLevel)
  , m_isSorted(true)
  , m_isVisible(true)
{
  if (parentGroup)
    parentGroup->addChildGroup(this);
}

ZWidgetsGroup::~ZWidgetsGroup()
{
  if (m_parent)
    m_parent->removeChildGroup(this);
  if (m_type == GROUP)
    for (int i=0; i<m_childGroups.size(); i++) {
      delete m_childGroups[i];
    }
}

void ZWidgetsGroup::setVisible(bool visible)
{
  m_isVisible = visible;
}

const QList<ZWidgetsGroup *> &ZWidgetsGroup::getChildGroups()
{
  if (!m_isSorted)
    sortChildGroups();
  return m_childGroups;
}

void ZWidgetsGroup::addChildGroup(ZWidgetsGroup *child, bool atEnd)
{
  if (child->m_parent && child->m_parent != this)
    child->m_parent->removeChildGroup(child);
  child->m_parent = this;
  if (atEnd)
    m_childGroups.push_back(child);
  else
    m_childGroups.push_front(child);
  connect(child, SIGNAL(widgetsGroupChanged()), this, SLOT(emitWidgetsGroupChangedSignal()));
  m_isSorted = false;
}

void ZWidgetsGroup::removeChildGroup(ZWidgetsGroup *child)
{
  int num = m_childGroups.removeAll(child);
  child->m_parent = NULL;
  if (num > 0)
    child->disconnect(this);
}

void ZWidgetsGroup::mergeGroup(ZWidgetsGroup *other, bool atEnd)
{
  if (other->m_type != GROUP) {
      addChildGroup(other, atEnd);
  } else {
    if (atEnd) {
      while (other->m_childGroups.size() > 0) {
        addChildGroup(other->m_childGroups[0], atEnd);
      }
    } else {
      for (int i=other->m_childGroups.size()-1; i>=0; i--) {
        addChildGroup(other->m_childGroups[i], atEnd);
      }
    }
  }
}

QWidget *ZWidgetsGroup::createWidget(QMainWindow *mainWin, bool createBasic)
{
  QLayout *lw = createLayout(mainWin, createBasic);
  // if is boxLayout, add strech to fill the space
  QBoxLayout *blo = dynamic_cast<QBoxLayout*>(lw);
  if (blo)
    blo->addStretch();
  QWidget *widget = new QWidget();
  widget->setLayout(lw);
  QScrollArea * sa = new QScrollArea();
  sa->setWidgetResizable(true);
  sa->setWidget(widget);

  //sa->setVisible(isVisible());

  return sa;
}

QLayout *ZWidgetsGroup::createLayout(QMainWindow *mainWin, bool createBasic)
{
  switch (m_type) {
  case WIDGET: {
    QHBoxLayout *hbl = new QHBoxLayout;
    hbl->addWidget(m_widget);
    return hbl;
  }
  case PARAMETER: {
    QHBoxLayout *hbl = new QHBoxLayout;
    if (dynamic_cast<Z3DCameraParameter*>(m_parameter)) {
      QWidget* wg = m_parameter->createWidget();
      hbl->addWidget(wg);
      return hbl;
    }
    m_parameter->setMainWindow(mainWin);
    QLabel *label = m_parameter->createNameLabel();
    label->setMinimumWidth(125);
    label->setWordWrap(true);
    hbl->addWidget(label);
    QWidget* wg = m_parameter->createWidget();
    wg->setMinimumWidth(125);
    wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hbl->addWidget(wg);

    return hbl;
  }
  default: /*case GROUP:*/ {
    QVBoxLayout *vbl = new QVBoxLayout;
    if (!m_isSorted)
      sortChildGroups();
    if (createBasic) {
      int i;
      for (i=0; i<m_childGroups.size() && m_childGroups[i]->m_visibleLevel
           <= m_cutOffbetweenBasicAndAdvancedLevel; i++) {
        QLayout *lw = m_childGroups[i]->createLayout(mainWin, true);
        if (m_childGroups[i]->isGroup()) {
          QGroupBox *groupBox = new QGroupBox(m_childGroups[i]->getGroupName());
          groupBox->setLayout(lw);
          vbl->addWidget(groupBox);
        } else
          vbl->addLayout(lw);
      }
      if (i<m_childGroups.size()) {
        QPushButton *pb = new QPushButton("Advanced...");
        pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        vbl->addWidget(pb, 0, Qt::AlignHCenter | Qt::AlignVCenter);
        connect(pb, SIGNAL(clicked()), this, SLOT(emitRequestAdvancedWidgetSignal()));
      }
    } else {
      for (int i=0; i<m_childGroups.size(); i++) {
        QLayout *lw = m_childGroups[i]->createLayout(mainWin, false);
        if (m_childGroups[i]->isGroup()) {
          QGroupBox *groupBox = new QGroupBox(m_childGroups[i]->getGroupName());
          groupBox->setLayout(lw);
          vbl->addWidget(groupBox);
        } else
          vbl->addLayout(lw);
      }
    }
    return vbl;
  }
  }
}

bool ZWidgetsGroup::operator <(const ZWidgetsGroup &other) const
{
  return m_visibleLevel < other.m_visibleLevel;
}

bool __widgetGroupPtVisibleLevelLessThan(const ZWidgetsGroup *s1, const ZWidgetsGroup *s2)
 {
     return s1->getVisibleLevel() < s2->getVisibleLevel();
 }

void ZWidgetsGroup::sortChildGroups()
{
  qStableSort(m_childGroups.begin(), m_childGroups.end(), __widgetGroupPtVisibleLevelLessThan);
  m_isSorted = true;
}

void ZWidgetsGroup::emitRequestAdvancedWidgetSignal()
{
  emit requestAdvancedWidget(m_groupName);
}

void ZWidgetsGroup::emitWidgetsGroupChangedSignal()
{
  emit widgetsGroupChanged();
}
