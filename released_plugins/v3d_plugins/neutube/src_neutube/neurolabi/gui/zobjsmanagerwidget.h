#ifndef ZOBJSMANAGERWIDGET_H
#define ZOBJSMANAGERWIDGET_H

#include <QWidget>
#include <QTreeView>

class ZStackDoc;
class ZPunctum;
class ZSwcTree;
struct _Swc_Tree_Node;
typedef _Swc_Tree_Node Swc_Tree_Node;
class QSortFilterProxyModel;

class ZObjsManagerWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZObjsManagerWidget(ZStackDoc *doc, QWidget *parent = 0);
  virtual ~ZObjsManagerWidget();
  
signals:
  void swcDoubleClicked(ZSwcTree *tree);
  void swcNodeDoubleClicked(Swc_Tree_Node* node);
  void punctaDoubleClicked(ZPunctum* p);
  
public slots:
  void swcItemDoubleClicked(QModelIndex index);
  void processDoubleClickOnCategorizedSwcNode(QModelIndex index);
  void swcSelectionChangedFromTreeView(
      QItemSelection selected, QItemSelection deselected);
  void updateSelectionFromCategorizedSwcNode(
      QItemSelection selected, QItemSelection deselected);
  void punctaItemDoubleClicked(QModelIndex index);
  void punctaSelectionChangedFromTreeView(QItemSelection selected, QItemSelection deselected);

  void punctaSelectionChanged(QList<ZPunctum*> selected, QList<ZPunctum*> deselected);
  void swcSelectionChanged(QList<ZSwcTree*> selected, QList<ZSwcTree*> deselected);
  void swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*> selected, QList<Swc_Tree_Node*> deselected);

protected:
  void createWidget();

  virtual void keyPressEvent(QKeyEvent *event);

  void buildItemSelectionFromList(const QList<ZPunctum*> &list, QItemSelection &is);
  void buildItemSelectionFromList(const QList<ZSwcTree*> &list, QItemSelection &is);
  void buildItemSelectionFromList(const QList<Swc_Tree_Node*> &list, QItemSelection &is);

  QList<Swc_Tree_Node*> getSwcNodeList(QItemSelection &is);

  ZStackDoc *m_doc;
  
  QTreeView *m_swcObjsTreeView;
  QTreeView *m_swcNodeObjsTreeView;
  QTreeView *m_punctaObjsTreeView;
  QSortFilterProxyModel *m_punctaProxyModel;
};

#endif // ZOBJSMANAGERWIDGET_H
