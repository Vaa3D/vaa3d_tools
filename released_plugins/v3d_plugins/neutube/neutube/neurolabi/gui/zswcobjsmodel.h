#ifndef ZSWCOBJSMODEL_H
#define ZSWCOBJSMODEL_H

#include <map>
#include "zobjsmodel.h"

class ZStackDoc;
class ZSwcTree;
struct _Swc_Tree_Node;
typedef _Swc_Tree_Node Swc_Tree_Node;

class ZSwcObjsModel : public ZObjsModel
{
  Q_OBJECT
public:
  explicit ZSwcObjsModel(ZStackDoc *doc, QObject *parent = 0);
  virtual ~ZSwcObjsModel();

  QModelIndex getIndex(ZSwcTree* tree, int col = 0) const;
  QModelIndex getIndex(Swc_Tree_Node* tn, int col = 0) const;
  ZSwcTree* getSwcTree(const QModelIndex &index) const;
  Swc_Tree_Node* getSwcTreeNode(const QModelIndex &index) const;

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;
  
  std::map<ZSwcTree*, int> m_swcToRow;
  std::map<Swc_Tree_Node*, int> m_swcTreeNodeToRow;
  std::map<Swc_Tree_Node*, ZSwcTree*> m_swcTreeNodeToSwc;
  
};

#endif // ZSWCOBJSMODEL_H
