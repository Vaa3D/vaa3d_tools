#ifndef ZWIDGETSGROUP_H
#define ZWIDGETSGROUP_H

#include <QObject>
#include <QLayout>

class ZParameter;
class QMainWindow;

class ZWidgetsGroup : public QObject
{
  Q_OBJECT
public:
  enum type {
    GROUP, WIDGET, PARAMETER
  };

  explicit ZWidgetsGroup(QWidget* widget, ZWidgetsGroup* parentGroup, int visibleLevel, QObject *parent = 0);
  explicit ZWidgetsGroup(const QString &groupName, ZWidgetsGroup* parentGroup, int visibleLevel, QObject *parent = 0);
  explicit ZWidgetsGroup(ZParameter *parameter, ZWidgetsGroup* parentGroup, int visibleLevel, QObject *parent = 0);
  virtual ~ZWidgetsGroup();

  inline bool isGroup() const {return m_type == GROUP;}
  inline QString getGroupName() const {return m_groupName;}
  const QList<ZWidgetsGroup*>& getChildGroups();

  void addChildGroup(ZWidgetsGroup *child, bool atEnd = true);
  void removeChildGroup(ZWidgetsGroup* child);

  void mergeGroup(ZWidgetsGroup* other, bool atEnd = true);

  inline int getVisibleLevel() const {return m_visibleLevel;}
  inline void setVisibleLevel(int v) {m_visibleLevel=v;}
  inline void setBasicAdvancedCutoff(int v) {m_cutOffbetweenBasicAndAdvancedLevel=v;}
  inline int getBasicAdvancedCutoff() const {return m_cutOffbetweenBasicAndAdvancedLevel;}
  void setVisible(bool visible);
  inline bool isVisible() { return m_isVisible; }

  QWidget *createWidget(QMainWindow *mainWin, bool createBasic = true);
  QLayout *createLayout(QMainWindow *mainWin, bool createBasic = true);

  bool operator<(const ZWidgetsGroup& other) const;
  
signals:
  void requestAdvancedWidget(const QString &name);
  void widgetsGroupChanged();
  
public slots:
  void emitRequestAdvancedWidgetSignal();
  void emitWidgetsGroupChangedSignal();

private:
  void sortChildGroups();
  
private:
  type m_type;
  QString m_groupName;
  QWidget *m_widget;
  ZParameter *m_parameter;
  ZWidgetsGroup* m_parent;
  int m_visibleLevel;
  bool m_isSorted;
  int m_cutOffbetweenBasicAndAdvancedLevel;
  QList<ZWidgetsGroup*> m_childGroups;
  bool m_isVisible;
};

#endif // ZWIDGETSGROUP_H
