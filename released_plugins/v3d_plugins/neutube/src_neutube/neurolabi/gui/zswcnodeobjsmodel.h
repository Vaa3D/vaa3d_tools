#ifndef ZSWCNODEOBJSMODEL_H
#define ZSWCNODEOBJSMODEL_H

//Categorized swc node model

#include <map>
#include <set>
#include "zobjsmodel.h"
#include "swctreenode.h"

class ZStackDoc;
class ZSwcTree;

class ZSwcNodeObjsModel : public ZObjsModel
{
  Q_OBJECT
public:
  explicit ZSwcNodeObjsModel(ZStackDoc *doc, QObject *parent = 0);
  virtual ~ZSwcNodeObjsModel();

  QModelIndex getIndex(SwcTreeNode::ETopologicalType type, int col = 0) const;
  QModelIndex getIndex(Swc_Tree_Node* tn, int col = 0) const;
  Swc_Tree_Node* getSwcTreeNode(const QModelIndex &index) const;
  std::set<Swc_Tree_Node*> getSwcTreeNodeSet(const QModelIndex &index) const;

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;

  std::map<SwcTreeNode::ETopologicalType, int> m_typeToRow;
  std::map<Swc_Tree_Node*, int> m_swcTreeNodeToRow;
  std::map<Swc_Tree_Node*, SwcTreeNode::ETopologicalType> m_swcTreeNodeToType;
};

#endif // ZSWCNODEOBJSMODEL_H
