#include "zswcobjsmodel.h"

#include "zstackdoc.h"
#include "zobjsitem.h"
#include "zswctree.h"
#include "tz_swc_tree.h"
#include "QsLog.h"

ZSwcObjsModel::ZSwcObjsModel(ZStackDoc *doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{
  updateModelData();
}

ZSwcObjsModel::~ZSwcObjsModel()
{
}

QModelIndex ZSwcObjsModel::getIndex(ZSwcTree *tree, int col) const
{
  std::map<ZSwcTree*, int>::const_iterator swc2rIt = m_swcToRow.find(tree);
  if (swc2rIt != m_swcToRow.end()) {
    return index(swc2rIt->second, col);
  }
  return QModelIndex();
}

QModelIndex ZSwcObjsModel::getIndex(Swc_Tree_Node *tn, int col) const
{
  std::map<Swc_Tree_Node*, int>::const_iterator pun2rIt = m_swcTreeNodeToRow.find(tn);
  if (pun2rIt != m_swcTreeNodeToRow.end()) {
    std::map<Swc_Tree_Node*, ZSwcTree*>::const_iterator s2pIt = m_swcTreeNodeToSwc.find(tn);
    std::map<ZSwcTree*, int>::const_iterator p2rIt = m_swcToRow.find(s2pIt->second);
    return index(pun2rIt->second, col, index(p2rIt->second, 0));
  }
  return QModelIndex();
}

ZSwcTree *ZSwcObjsModel::getSwcTree(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() == m_rootItem)
    return static_cast<ZSwcTree*>(item->getObj());
  else
    return NULL;
}

Swc_Tree_Node *ZSwcObjsModel::getSwcTreeNode(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() && item->parent()->parent() == m_rootItem)
    return static_cast<Swc_Tree_Node*>(item->getObj());
  else
    return NULL;
}


void ZSwcObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;
  /*
  rootData << "swcs" << "id" << "type" << "radius" << "x" << "y" << "z" <<
              "parent_id" << "label" << "weight" << "feature" << "index" << "source";
              */
  rootData << "SWC" << "Source";
  m_rootItem = new ZObjsItem(rootData, m_doc->swcList());
  setupModelData(m_rootItem);
  endResetModel();
}

void ZSwcObjsModel::setupModelData(ZObjsItem *parent)
{
  QList<QVariant> data;

  m_swcToRow.clear();
  m_swcTreeNodeToRow.clear();
  m_swcTreeNodeToSwc.clear();
  for (int i=0; i<m_doc->swcList()->size(); i++) {
    data.clear();
    ZSwcTree *swcTree = m_doc->swcList()->at(i);

    data << QString("swc %1").arg(i+1)
         << QString::fromStdString(swcTree->source());

    ZObjsItem *nodeParent = new ZObjsItem(data, swcTree, parent);
    nodeParent->setCheckState(swcTree->isVisible() ? Qt::Checked : Qt::Unchecked);
    nodeParent->setToolTip(QString("source: %1").arg(
                             QString::fromStdString(swcTree->source())));
    parent->appendChild(nodeParent);
    m_swcToRow[swcTree] = i;

#if 0
    data << QString("swc %1").arg(i+1) << "id" << "type" << "radius" << "x" << "y" << "z" <<
            "parent_id" << "label" << "weight" << "feature" << "index" << QString::fromStdString(swcTree->source());
    ZObjsItem *nodeParent = new ZObjsItem(data, swcTree, parent);
    nodeParent->setCheckState(swcTree->isVisible() ? Qt::Checked : Qt::Unchecked);
    nodeParent->setToolTip(QString("source: %1").arg(QString::fromStdString(swcTree->source())));
    parent->appendChild(nodeParent);
    m_swcToRow[swcTree] = i;

    swcTree->updateIterator(1);   //depth first
    int row = 0;
    for (Swc_Tree_Node *tn = swcTree->begin(); tn != swcTree->end(); tn = swcTree->next()) {
      if (!Swc_Tree_Node_Is_Virtual(tn)) {
        data.clear();
        m_swcTreeNodeToSwc[tn] = swcTree;
        m_swcTreeNodeToRow[tn] = row++;
        data << "" << tn->node.id << tn->node.type << tn->node.d << tn->node.x << tn->node.y << tn->node.z <<
                tn->node.parent_id << tn->node.label << tn->weight << tn->feature << tn->index << "";
        ZObjsItem *node = new ZObjsItem(data, tn, nodeParent);
        node->setToolTip(QString("swc node from: %1").arg(QString::fromStdString(swcTree->source())));
        nodeParent->appendChild(node);
      }
    }
#endif
  }
}

void ZSwcObjsModel::setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs)
{
  ZObjsModel::setModelIndexCheckState(index, cs);
  if (getSwcTree(index) != NULL)
    m_doc->setSwcVisible(getSwcTree(index), cs == Qt::Checked);
}

bool ZSwcObjsModel::needCheckbox(const QModelIndex &index) const
{
  return getSwcTree(index) != NULL;
}
