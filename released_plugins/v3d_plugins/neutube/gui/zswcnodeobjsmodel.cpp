#include "zswcnodeobjsmodel.h"

#include "zstackdoc.h"
#include "zobjsitem.h"
#include "zswctree.h"
#include "tz_swc_tree.h"
#include "QsLog.h"

ZSwcNodeObjsModel::ZSwcNodeObjsModel(ZStackDoc *doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{
  updateModelData();
}

ZSwcNodeObjsModel::~ZSwcNodeObjsModel()
{
}

QModelIndex ZSwcNodeObjsModel::getIndex(SwcTreeNode::ETopologicalType type,
                                        int col) const
{
  std::map<SwcTreeNode::ETopologicalType, int>::const_iterator iter =
      m_typeToRow.find(type);
  if (iter != m_typeToRow.end()) {
    return index(iter->second, col);
  }
  return QModelIndex();
}

QModelIndex ZSwcNodeObjsModel::getIndex(Swc_Tree_Node *tn, int col) const
{
  std::map<Swc_Tree_Node*, int>::const_iterator iter =
      m_swcTreeNodeToRow.find(tn);
  if (iter != m_swcTreeNodeToRow.end()) {
    std::map<Swc_Tree_Node*, SwcTreeNode::ETopologicalType>::const_iterator
        s2pIt = m_swcTreeNodeToType.find(tn);
    std::map<SwcTreeNode::ETopologicalType, int>::const_iterator p2rIt =
        m_typeToRow.find(s2pIt->second);
    return index(iter->second, col, index(p2rIt->second, 0));
  }
  return QModelIndex();
}

Swc_Tree_Node *ZSwcNodeObjsModel::getSwcTreeNode(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() && item->parent()->parent() == m_rootItem)
    return static_cast<Swc_Tree_Node*>(item->getObj());
  else
    return NULL;
}

std::set<Swc_Tree_Node*> ZSwcNodeObjsModel::getSwcTreeNodeSet(
    const QModelIndex &index) const
{
  std::set<Swc_Tree_Node*> nodeSet;
  if (index.isValid()) {
    ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());
    if (item->parent() == m_rootItem) {
      int childNumber = item->childCount();
      for (int i = 0; i < childNumber; ++i) {
        ZObjsItem *childItem = item->child(i);
        Swc_Tree_Node *tn = static_cast<Swc_Tree_Node*>(childItem->getObj());
        if (tn != NULL) {
          nodeSet.insert(tn);
        }
      }
    } else {
      Swc_Tree_Node *tn = getSwcTreeNode(index);
      if (tn != NULL) {
        nodeSet.insert(tn);
      }
    }
  }

  return nodeSet;
}


void ZSwcNodeObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;
  rootData << "Categories" << "id" << "type" << "radius" << "x" << "y" << "z"
           << "label";
  m_rootItem = new ZObjsItem(rootData, NULL);
  setupModelData(m_rootItem);
  endResetModel();
}

void ZSwcNodeObjsModel::setupModelData(ZObjsItem *parent)
{
  QList<QVariant> data;

  m_typeToRow.clear();
  m_swcTreeNodeToRow.clear();
  m_swcTreeNodeToType.clear();

  m_typeToRow[SwcTreeNode::TERMINAL] = 0;
  m_typeToRow[SwcTreeNode::BRANCH_POINT] = 1;

  data.clear();
  data << "Termini" << "id" << "type" << "radius" << "x" << "y" << "z"
       << "label";
  ZObjsItem *terminalItem = new ZObjsItem(data, NULL, parent);
  terminalItem->setCheckState(Qt::Checked);
  parent->appendChild(terminalItem);

  data.clear();
  data << "Branch Points" << "id" << "type" << "radius" << "x" << "y" << "z"
       << "label";
  ZObjsItem *branchPointItem = new ZObjsItem(data, NULL, parent);
  branchPointItem->setCheckState(Qt::Checked);
  parent->appendChild(branchPointItem);

  int terminalRow = 0;
  int branchPointRow = 0;

  for (int i=0; i<m_doc->swcList()->size(); i++) {
    data.clear();
    ZSwcTree *swcTree = m_doc->swcList()->at(i);

    //ZObjsItem *nodeParent = new ZObjsItem(data, swcTree, parent);
    //nodeParent->setCheckState(swcTree->isVisible() ? Qt::Checked : Qt::Unchecked);
    //nodeParent->setToolTip(QString("source: %1").arg(QString::fromStdString(swcTree->source())));
    //parent->appendChild(nodeParent);

    swcTree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);   //depth first
    for (Swc_Tree_Node *tn = swcTree->begin(); tn != swcTree->end(); tn = swcTree->next()) {
      if (!SwcTreeNode::isVirtual(tn)) {
        data.clear();
        data << "" << tn->node.id << tn->node.type << tn->node.d << tn->node.x
             << tn->node.y << tn->node.z  << tn->node.label << "";
        if (SwcTreeNode::isBranchPoint(tn)) {
          m_swcTreeNodeToType[tn] = SwcTreeNode::BRANCH_POINT;
          m_swcTreeNodeToRow[tn] = branchPointRow++;
          ZObjsItem *node = new ZObjsItem(data, tn, branchPointItem);
          branchPointItem->appendChild(node);
        } else if (SwcTreeNode::isRoot(tn) || SwcTreeNode::isLeaf(tn)) {
          m_swcTreeNodeToType[tn] = SwcTreeNode::TERMINAL;
          m_swcTreeNodeToRow[tn] = terminalRow++;
          ZObjsItem *node = new ZObjsItem(data, tn, terminalItem);
          terminalItem->appendChild(node);
        }
      }
    }
  }
}

void ZSwcNodeObjsModel::setModelIndexCheckState(
    const QModelIndex &/*index*/, Qt::CheckState /*cs*/)
{
}

bool ZSwcNodeObjsModel::needCheckbox(const QModelIndex &/*index*/) const
{
  return false;
}
