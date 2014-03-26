#ifndef ZOBJSMODEL_H
#define ZOBJSMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class ZObjsItem;

// provide a data model for all objects, used by views such as treeview, tableview
class ZObjsModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  ZObjsModel(QObject *parent = 0);
  ~ZObjsModel();

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

protected:
  void updateChildCheckState(const QModelIndex & parent, Qt::CheckState cs);
  void updateParentCheckState(const QModelIndex & child);
  // These two functions assume the input modelindex is valid
  virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  Qt::CheckState getModelIndexCheckState(const QModelIndex &index) const;

  virtual bool needCheckbox(const QModelIndex &index) const = 0;

  ZObjsItem *m_rootItem;

signals:
  
public slots:
  
};

#endif // ZOBJSMODEL_H
