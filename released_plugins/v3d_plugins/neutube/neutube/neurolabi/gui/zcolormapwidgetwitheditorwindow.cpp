#include "zcolormapwidgetwitheditorwindow.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include "zcolormap.h"
#include "zcolormapeditor.h"
#include "QsLog.h"

ZColorMapWidgetWithEditorWindow::ZColorMapWidgetWithEditorWindow(ZColorMapParameter *cm, QMainWindow *minWin, QWidget *parent) :
  ZClickableColorMapLabel(cm, parent), m_colorMap(cm), m_mainWin(minWin), m_editorWindow(NULL)
{
  if (!m_mainWin) {
    LWARN() << "If Main Window is empty, this widget will show nothing when clicked.";
  } else {
    connect(m_mainWin, SIGNAL(destroyed()), this, SLOT(mainWindowDestroyed()));
  }
  //  if (m_mainWin) {
  //    connect(m_mainWin, SIGNAL(closed()), this, SLOT(mainWindowClosed()));
  //  }
}

void ZColorMapWidgetWithEditorWindow::hideEvent(QHideEvent *)
{
  if (m_editorWindow && m_editorWindow->isVisible())
    m_editorWindow->hide();
}

void ZColorMapWidgetWithEditorWindow::createEditorWindow()
{
  if (m_editorWindow) {
    if (m_editorWindow->isVisible()) {
      //hide editor widget
      m_editorWindow->hide();
    } else {
      //show editor Widget
      m_editorWindow->show();
    }
  } else if (m_mainWin) {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    m_editorWindow = new QDockWidget(m_colorMap->getName(), m_mainWin);
    m_editorWindow->setAllowedAreas(Qt::RightDockWidgetArea);
    m_colorMapEditor = new ZColorMapEditor(m_colorMap, m_mainWin);
    m_editorWindow->setWidget(m_colorMapEditor);
    m_mainWin->addDockWidget(Qt::RightDockWidgetArea, m_editorWindow);
    m_editorWindow->setFloating(true);

    QApplication::restoreOverrideCursor();
  }
}

void ZColorMapWidgetWithEditorWindow::mainWindowDestroyed()
{
  m_mainWin = NULL;
  m_editorWindow = NULL;
}

//void ZColorMapWidgetWithEditorWindow::mainWindowClosed()
//{
//  if (m_editorWindow)
//    m_editorWindow->close();
//}

void ZColorMapWidgetWithEditorWindow::labelClicked()
{
  createEditorWindow();
}
