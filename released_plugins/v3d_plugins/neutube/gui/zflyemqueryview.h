#ifndef ZFLYEMQUERYVIEW_H
#define ZFLYEMQUERYVIEW_H

#include <QTableView>

class QMenu;

class ZFlyEmQueryView : public QTableView
{
  Q_OBJECT
public:
  explicit ZFlyEmQueryView(QWidget *parent = 0);

  inline void setContextMenu(QMenu *menu) { m_contextMenu = menu; }

signals:

public slots:

protected:
  void contextMenuEvent(QContextMenuEvent *);
  void mousePressEvent(QMouseEvent *event);

private:
  QMenu *m_contextMenu;
};

#endif // ZFLYEMQUERYVIEW_H
