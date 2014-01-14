#include "zobjsmanagerwidget.h"

#include "z3dgl.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include <map>
#include <set>

#include "zstackdoc.h"
#include "zswcobjsmodel.h"
#include "zswcnodeobjsmodel.h"
#include "zpunctaobjsmodel.h"
#include "QsLog/QsLog.h"
#include "neutubeconfig.h"


ZObjsManagerWidget::ZObjsManagerWidget(ZStackDoc *doc, QWidget *parent) :
  QWidget(parent), m_doc(doc)
{
  setFocusPolicy(Qt::StrongFocus);
  createWidget();
}

ZObjsManagerWidget::~ZObjsManagerWidget()
{
}

void ZObjsManagerWidget::swcItemDoubleClicked(QModelIndex index)
{
  ZSwcTree *p2 = m_doc->swcObjsModel()->getSwcTree(index);
  if (p2 != NULL) {
    emit swcDoubleClicked(p2);
  } else {
    Swc_Tree_Node *p3 = m_doc->swcObjsModel()->getSwcTreeNode(index);
    if (p3 != NULL) {
      emit swcNodeDoubleClicked(p3);
    }
  }
}

void ZObjsManagerWidget::processDoubleClickOnCategorizedSwcNode(
    QModelIndex index)
{
  Swc_Tree_Node *p3 = m_doc->swcNodeObjsModel()->getSwcTreeNode(index);
  if (p3 != NULL) {
    emit swcNodeDoubleClicked(p3);
  }
}

void ZObjsManagerWidget::swcSelectionChangedFromTreeView(QItemSelection selected, QItemSelection deselected)
{
  QModelIndexList indexes = deselected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    ZSwcTree *p2 = m_doc->swcObjsModel()->getSwcTree(indexes[i]);
    if (p2 != NULL) {
      m_doc->setSwcSelected(p2, false);
    } else {
      Swc_Tree_Node *p3 = m_doc->swcObjsModel()->getSwcTreeNode(indexes[i]);
      if (p3 != NULL) {
        m_doc->setSwcTreeNodeSelected(p3, false);
      }
    }
  }
  indexes = selected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    ZSwcTree *p2 = m_doc->swcObjsModel()->getSwcTree(indexes[i]);
    if (p2 != NULL) {
      m_doc->setSwcSelected(p2, true);
    } else {
      Swc_Tree_Node *p3 = m_doc->swcObjsModel()->getSwcTreeNode(indexes[i]);
      if (p3 != NULL) {
        m_doc->setSwcTreeNodeSelected(p3, true);
      }
    }
  }
}

void ZObjsManagerWidget::updateSelectionFromCategorizedSwcNode(
    QItemSelection selected, QItemSelection deselected)
{
  QModelIndexList indexes = deselected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    Swc_Tree_Node *p3 = m_doc->swcNodeObjsModel()->getSwcTreeNode(indexes[i]);
    if (p3 != NULL) {
      m_doc->setSwcTreeNodeSelected(p3, false);
    } else {
      std::set<Swc_Tree_Node*> nodeSet =
          m_doc->swcNodeObjsModel()->getSwcTreeNodeSet(indexes[i]);
      for (std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        m_doc->setSwcTreeNodeSelected(*iter, false);
      }
    }
  }
  indexes = selected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    Swc_Tree_Node *p3 = m_doc->swcNodeObjsModel()->getSwcTreeNode(indexes[i]);
    if (p3 != NULL) {
      m_doc->setSwcTreeNodeSelected(p3, true);
    } else {
      std::set<Swc_Tree_Node*> nodeSet =
          m_doc->swcNodeObjsModel()->getSwcTreeNodeSet(indexes[i]);
      for (std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        m_doc->setSwcTreeNodeSelected(*iter, true);
      }
    }
  }
}

void ZObjsManagerWidget::punctaItemDoubleClicked(QModelIndex index)
{
  ZPunctum *p = m_doc->punctaObjsModel()->getPunctum(m_punctaProxyModel->mapToSource(index));
  if (p != NULL) {
    emit punctaDoubleClicked(p);
  }
}

void ZObjsManagerWidget::punctaSelectionChangedFromTreeView(QItemSelection selected, QItemSelection deselected)
{
  QModelIndexList indexes = deselected.indexes();
  std::vector<ZPunctum*> coll;
  for (int i=0; i<indexes.size(); i++) {
    if (indexes[i].column() > 0)
        continue;
    ZPunctum *p = m_doc->punctaObjsModel()->getPunctum(m_punctaProxyModel->mapToSource(indexes[i]));
    if (p != NULL) {
      coll.push_back(p);
    } else {
      const std::vector<ZPunctum*>* ps = m_doc->punctaObjsModel()->getPuncta(m_punctaProxyModel->mapToSource(indexes[i]));
      if (ps != NULL) {
        std::copy(ps->begin(), ps->end(), std::back_inserter(coll));
      }
    }
  }
  if (!coll.empty()) {
    m_doc->setPunctumSelected(coll.begin(), coll.end(), false);
    coll.clear();
  }

  indexes = selected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    if (indexes[i].column() > 0)
        continue;
    ZPunctum *p = m_doc->punctaObjsModel()->getPunctum(m_punctaProxyModel->mapToSource(indexes[i]));
    if (p != NULL) {
      coll.push_back(p);
    } else {
      const std::vector<ZPunctum*>* ps = m_doc->punctaObjsModel()->getPuncta(m_punctaProxyModel->mapToSource(indexes[i]));
      if (ps != NULL) {
        std::copy(ps->begin(), ps->end(), std::back_inserter(coll));
      }
    }
  }
  if (!coll.empty()) {
    m_doc->setPunctumSelected(coll.begin(), coll.end(), true);
  }
}

void ZObjsManagerWidget::punctaSelectionChanged(QList<ZPunctum *> selected, QList<ZPunctum *> deselected)
{
  if (!selected.empty() && m_punctaObjsTreeView != NULL) {
    QItemSelection is;
    buildItemSelectionFromList(selected, is);
    m_punctaObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // scroll to first one if necessary
    QModelIndex index = m_doc->punctaObjsModel()->getIndex(selected[0]);
    if (m_punctaObjsTreeView->isExpanded(m_punctaProxyModel->mapFromSource(m_doc->punctaObjsModel()->parent(index)))) {
      m_punctaObjsTreeView->scrollTo(m_punctaProxyModel->mapFromSource(index));
    }
  }

  if (!deselected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(deselected, is);
    m_punctaObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }
}

void ZObjsManagerWidget::swcSelectionChanged(QList<ZSwcTree *> selected, QList<ZSwcTree *> deselected)
{
  if (!selected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(selected, is);
    m_swcObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // scroll to first one if necessary
    QModelIndex index = m_doc->swcObjsModel()->getIndex(selected[0]);
    //if (m_swcObjsTreeView->isExpanded(m_doc->swcObjsModel()->parent(index))) {
    m_swcObjsTreeView->scrollTo(index);
    //}
  }

  if (!deselected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(deselected, is);
    m_swcObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }
}

void ZObjsManagerWidget::swcTreeNodeSelectionChanged(
    QList<Swc_Tree_Node *> selected, QList<Swc_Tree_Node *> deselected)
{
  if (!selected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(selected, is);
    m_swcObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // scroll to first one if necessary
    QModelIndex index = m_doc->swcObjsModel()->getIndex(selected[0]);
    if (m_swcObjsTreeView->isExpanded(m_doc->swcObjsModel()->parent(index))) {
      m_swcObjsTreeView->scrollTo(index);
    }
  }

  if (!deselected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(deselected, is);
    m_swcObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }
}

void ZObjsManagerWidget::createWidget()
{
  QTabWidget *tabs = new QTabWidget(this);
  tabs->setElideMode(Qt::ElideNone);
  tabs->setUsesScrollButtons(true);

  m_swcObjsTreeView = new QTreeView(this);
  m_swcObjsTreeView->setExpandsOnDoubleClick(false);
  m_swcObjsTreeView->setModel(m_doc->swcObjsModel());
  m_swcObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_swcObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_swcObjsTreeView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(swcItemDoubleClicked(QModelIndex)));
  connect(m_swcObjsTreeView->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(swcSelectionChangedFromTreeView(QItemSelection,QItemSelection)));
  tabs->addTab(m_swcObjsTreeView, "Swcs");

  if (NeutubeConfig::getInstance().getObjManagerConfig().isCategorizedSwcNodeOn()) {
    m_swcNodeObjsTreeView = new QTreeView(this);
    m_swcNodeObjsTreeView->setExpandsOnDoubleClick(false);
    m_swcNodeObjsTreeView->setModel(m_doc->swcNodeObjsModel());
    m_swcNodeObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_swcNodeObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_swcNodeObjsTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(processDoubleClickOnCategorizedSwcNode(QModelIndex)));
    connect(m_swcNodeObjsTreeView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateSelectionFromCategorizedSwcNode(QItemSelection,QItemSelection)));
    tabs->addTab(m_swcNodeObjsTreeView, "SWC Nodes");
  }

  if (NeutubeConfig::getInstance().getMainWindowConfig().isMarkPunctaOn()) {
    m_punctaObjsTreeView = new QTreeView(this);
    m_punctaObjsTreeView->setSortingEnabled(true);
    m_punctaObjsTreeView->setExpandsOnDoubleClick(false);
    m_punctaProxyModel = new QSortFilterProxyModel(this);
    m_punctaProxyModel->setSourceModel(m_doc->punctaObjsModel());
    //m_punctaObjsTreeView->setModel(m_doc->punctaObjsModel());
    m_punctaObjsTreeView->setModel(m_punctaProxyModel);
    m_punctaObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_punctaObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_punctaObjsTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(punctaItemDoubleClicked(QModelIndex)));
    connect(m_punctaObjsTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(punctaSelectionChangedFromTreeView(QItemSelection,QItemSelection)));
    m_punctaObjsTreeView->sortByColumn(0, Qt::AscendingOrder);
    tabs->addTab(m_punctaObjsTreeView, "Puncta");
  }

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(tabs);
  setLayout(layout);

  if (!m_doc->selectedPuncta()->empty()) {
    std::set<ZPunctum*> *selectedPuncta = m_doc->selectedPuncta();
    QList<ZPunctum*> selected;
    QList<ZPunctum*> deselected;
    std::copy(selectedPuncta->begin(), selectedPuncta->end(), std::back_inserter(selected));
    punctaSelectionChanged(selected, deselected);
  }
  if (!m_doc->selectedSwcs()->empty()) {
    std::set<ZSwcTree*> *selectedSwcs = m_doc->selectedSwcs();
    QList<ZSwcTree*> selected;
    QList<ZSwcTree*> deselected;
    std::copy(selectedSwcs->begin(), selectedSwcs->end(), std::back_inserter(selected));
    swcSelectionChanged(selected, deselected);
  }
  if (!m_doc->selectedSwcTreeNodes()->empty()) {
    std::set<Swc_Tree_Node*> *selectedTreeNodes = m_doc->selectedSwcTreeNodes();
    QList<Swc_Tree_Node*> selected;
    QList<Swc_Tree_Node*> deselected;
    std::copy(selectedTreeNodes->begin(), selectedTreeNodes->end(), std::back_inserter(selected));
    swcTreeNodeSelectionChanged(selected, deselected);
  }

  connect(m_doc,
          SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),
          this, SLOT(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)));
  connect(m_doc,
          SIGNAL(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)),
          this, SLOT(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)));
  connect(m_doc,
          SIGNAL(swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),
          this, SLOT(swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)));
}

void ZObjsManagerWidget::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
  {
  case Qt::Key_Backspace:
  case Qt::Key_Delete:
  {
    m_doc->executeRemoveObjectCommand();
#if 0
    if (m_doc->selectedChains()->empty() && m_doc->selectedPuncta()->empty() && m_doc->selectedSwcs()->empty())
      return;
    QUndoCommand *removeSelectedObjectsCommand =
        new ZStackDocRemoveSelectedObjectCommand(m_doc);
    m_doc->undoStack()->push(removeSelectedObjectsCommand);
#endif
  }
    break;
  default:
    break;
  }
}

namespace {

struct ModelIndexCompareRow {
  bool operator() (const QModelIndex& lhs, const QModelIndex& rhs) const
  {return lhs.row() < rhs.row();}
};

void buildItemSelection(
    const std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> > &allIndex,
    QItemSelection &is)
{
  int numRange = 0;
  for (std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> >::const_iterator ait = allIndex.begin();
       ait != allIndex.end(); ++ait) {
    const std::set<QModelIndex, ModelIndexCompareRow> &indexes = ait->second;
    std::set<QModelIndex, ModelIndexCompareRow>::iterator it = indexes.begin();
    QModelIndex start = *it;
    QModelIndex end = start;
    int prevRow = start.row();
    ++it;
    for (; it != indexes.end(); ++it) {
      if (it->row() - prevRow == 1) {
        end = *it;
        prevRow = end.row();
      } else if (it->row() - prevRow > 1) {
        is.select(start, end);
        numRange++;
        start = *it;
        end = start;
        prevRow = start.row();
      } else {
        LDEBUG() << "impossible";
      }
    }
    is.select(start, end);
    numRange++;
  }
  //LDEBUG() << "number of range: " << numRange;
}

}

// build continues range to speed up selection speed in qt tree view
void ZObjsManagerWidget::buildItemSelectionFromList(const QList<ZPunctum *> &list, QItemSelection &is)
{
  if (list.empty())
    return;

  std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> > allIndex;

  for (int i=0; i<list.size(); i++) {
    QModelIndex index = m_punctaProxyModel->mapFromSource(m_doc->punctaObjsModel()->getIndex(list[i]));
    if (index.isValid()) {
      allIndex[index.parent()].insert(index);
    } else {
      LERROR() << "puncta don't exist in widget, something is wrong";
    }
  }

  buildItemSelection(allIndex, is);
}

void ZObjsManagerWidget::buildItemSelectionFromList(const QList<ZSwcTree *> &list, QItemSelection &is)
{
  if (list.empty())
    return;

  std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> > allIndex;

  for (int i=0; i<list.size(); i++) {
    QModelIndex index = m_doc->swcObjsModel()->getIndex(list[i]);
    if (index.isValid()) {
      allIndex[index.parent()].insert(index);
    } else {
      LERROR() << "swc tree don't exist in widget, something is wrong";
    }
  }

  buildItemSelection(allIndex, is);
}

void ZObjsManagerWidget::buildItemSelectionFromList(
    const QList<Swc_Tree_Node *> &list, QItemSelection &is)
{
  if (list.empty())
    return;

  std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> > allIndex;

  for (int i=0; i<list.size(); i++) {
    QModelIndex index = m_doc->swcNodeObjsModel()->getIndex(list[i]);
    if (index.isValid()) {
      allIndex[index.parent()].insert(index);
    }
  }

  buildItemSelection(allIndex, is);
}
