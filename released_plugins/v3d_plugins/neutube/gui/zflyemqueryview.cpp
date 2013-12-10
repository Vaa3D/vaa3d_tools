#include "zflyemqueryview.h"

#include <iostream>
#include <QMenu>
#include <QContextMenuEvent>

ZFlyEmQueryView::ZFlyEmQueryView(QWidget *parent) :
  QTableView(parent), m_contextMenu(NULL)
{
}

void ZFlyEmQueryView::contextMenuEvent(QContextMenuEvent *event)
{
#ifdef _DEBUG_
  std::cout << "Context menu triggered." << std::endl;
#endif

  if (m_contextMenu != NULL) {
    m_contextMenu->popup(event->globalPos());
  }
}

void ZFlyEmQueryView::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton) {
    QTableView::mousePressEvent(event);
  }
}
