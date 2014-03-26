#include "zobjsmodel.h"

#include <QtGui>

#include "zobjsitem.h"
#include "QsLog.h"

ZObjsModel::ZObjsModel(QObject *parent)
  : QAbstractItemModel(parent), m_rootItem(NULL)
{
}

ZObjsModel::~ZObjsModel()
{
  delete m_rootItem;
}

int ZObjsModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return static_cast<ZObjsItem*>(parent.internalPointer())->columnCount();
  else
    return m_rootItem->columnCount();
}

QVariant ZObjsModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (role == Qt::CheckStateRole && index.column() == 0 && needCheckbox(index))
    return item->checkState();

  if (role == Qt::ToolTipRole && !item->toolTip().isEmpty())
    return item->toolTip();

  if (role == Qt::DisplayRole)
    return item->data(index.column());

  return QVariant();
}

Qt::ItemFlags ZObjsModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if ( index.column() == 0 )
    flags |= Qt::ItemIsUserCheckable;

  return flags;
}

bool ZObjsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid())
    return false;

  if (role == Qt::CheckStateRole && index.column() == 0 && needCheckbox(index)) {
    Qt::CheckState cs = static_cast<Qt::CheckState>(value.toInt());

    setModelIndexCheckState(index, cs);
    // update child items check state
    updateChildCheckState(index, cs);
    // update parent items
    updateParentCheckState(index);

    return true;
  }

  return false;
}

QVariant ZObjsModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return m_rootItem->data(section);

  return QVariant();
}

QModelIndex ZObjsModel::index(int row, int column, const QModelIndex &parent)
const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  ZObjsItem *parentItem;

  if (!parent.isValid())
    parentItem = m_rootItem;
  else
    parentItem = static_cast<ZObjsItem*>(parent.internalPointer());

  ZObjsItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex ZObjsModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  ZObjsItem *childItem = static_cast<ZObjsItem*>(index.internalPointer());
  ZObjsItem *parentItem = childItem->parent();

  if (parentItem == m_rootItem)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

int ZObjsModel::rowCount(const QModelIndex &parent) const
{
  ZObjsItem *parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid()) {
    parentItem = m_rootItem;
    return parentItem->childCount();
  } else {
    parentItem = static_cast<ZObjsItem*>(parent.internalPointer());
    return parentItem->childCount();
  }
}

void ZObjsModel::updateChildCheckState(const QModelIndex &parent, Qt::CheckState cs)
{
  if (cs == Qt::PartiallyChecked) {
    LERROR() << "Wrong CheckState type, should not happen";
    return;
  }
  for (int i=0; i<rowCount(parent); i++) {
    QModelIndex child = index(i, 0, parent);
    if (!needCheckbox(child))
      return;
    if (cs != getModelIndexCheckState(child)) {
      setModelIndexCheckState(child, cs);
      updateChildCheckState(child, cs);
    }
  }
}

void ZObjsModel::updateParentCheckState(const QModelIndex &child)
{
  QModelIndex idx = parent(child);
  if (!idx.isValid())
    return;

  int numChecked = 0;
  int numUnChecked = 0;
  int numPartialChecked = 0;
  Qt::CheckState oldCheckState = getModelIndexCheckState(idx);
  Qt::CheckState newCheckState;

  for (int i=0; i<rowCount(idx); i++) {
    QModelIndex cld = index(i, 0, idx);
    Qt::CheckState cs = getModelIndexCheckState(cld);
    if (cs == Qt::Checked) {
      ++numChecked;
    } else if (cs == Qt::Unchecked) {
      ++numUnChecked;
    } else {  // should be PartialChecked
      ++numPartialChecked;
      break;
    }
  }

  if (numPartialChecked > 0) {
    newCheckState = Qt::PartiallyChecked;
  } else if (numChecked > 0 && numUnChecked > 0) {
    newCheckState = Qt::PartiallyChecked;
  } else if (numChecked == 0) {
    if (numUnChecked == 0) {
      LERROR() << "This Index should have at least one child.";
      return;
    }
    newCheckState = Qt::Unchecked;
  } else {   // numUnChecked == 0
    if (numChecked == 0) {
      LERROR() << "This Index should have at least one child.";
      return;
    }
    newCheckState = Qt::Checked;
  }

  if (newCheckState != oldCheckState) {
    setModelIndexCheckState(idx, newCheckState);
    updateParentCheckState(idx);
  }
}

void ZObjsModel::setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs)
{
  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());
  item->setCheckState(cs);
  emit dataChanged(index, index);
}

Qt::CheckState ZObjsModel::getModelIndexCheckState(const QModelIndex &index) const
{
  return static_cast<ZObjsItem*>(index.internalPointer())->checkState();
}
