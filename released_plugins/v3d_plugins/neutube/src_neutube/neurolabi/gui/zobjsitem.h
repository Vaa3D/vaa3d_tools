#ifndef ZOBJSITEM_H
#define ZOBJSITEM_H

#include <QList>
#include <QVariant>
#include <QString>


// this class should only be used by ZObjsModel
class ZObjsItem
{
public:
  ZObjsItem(const QList<QVariant> &data, void *pData, ZObjsItem *parent = 0);
  ~ZObjsItem();

  void appendChild(ZObjsItem *child);

  ZObjsItem *child(int row);
  int childCount() const;
  int columnCount() const;
  QVariant data(int column) const;
  int row() const;
  ZObjsItem *parent();

  inline void* getObj() const { return m_actualObj; }
  inline void setItemData(QList<QVariant> &itemData) { m_itemData = itemData; }
  inline QList<QVariant>& getItemData() {return m_itemData;}

  inline Qt::CheckState checkState() const { return m_checkState; }
  inline void setCheckState(Qt::CheckState set) { m_checkState = set; }

  inline QString toolTip() const { return m_toolTip; }
  inline void setToolTip(const QString &tt) { m_toolTip = tt; }

private:
  QList<ZObjsItem*> m_childItems;
  QList<QVariant> m_itemData;
  ZObjsItem *m_parentItem;
  void *m_actualObj;
  Qt::CheckState m_checkState;
  QString m_toolTip;
};

#endif // ZOBJSITEM_H
