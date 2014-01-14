#include "z3dtransferfunctionwidgetwitheditorwindow.h"
#include "z3dgl.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include "z3dtransferfunction.h"
#include "z3dtransferfunctioneditor.h"

Z3DTransferFunctionWidgetWithEditorWindow::Z3DTransferFunctionWidgetWithEditorWindow(Z3DTransferFunctionParameter *tf,
                                                                                     QMainWindow *minWin, QWidget *parent)
  : ZClickableTransferFunctionLabel(tf, parent)
  , m_transferFunction(tf)
  , m_mainWin(minWin)
  , m_editorWindow(NULL)

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

void Z3DTransferFunctionWidgetWithEditorWindow::hideEvent(QHideEvent *)
{
  if (m_editorWindow && m_editorWindow->isVisible())
    m_editorWindow->hide();
}

void Z3DTransferFunctionWidgetWithEditorWindow::createEditorWindow()
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

    m_editorWindow = new QDockWidget(m_transferFunction->getName(), m_mainWin);
    m_editorWindow->setAllowedAreas(Qt::RightDockWidgetArea);
    m_transferFunctionEditor = new Z3DTransferFunctionEditor(m_transferFunction, m_mainWin);
    m_editorWindow->setWidget(m_transferFunctionEditor);
    m_mainWin->addDockWidget(Qt::RightDockWidgetArea, m_editorWindow);
    m_editorWindow->setFloating(true);

    QApplication::restoreOverrideCursor();
  }
}

void Z3DTransferFunctionWidgetWithEditorWindow::mainWindowDestroyed()
{
  m_mainWin = NULL;
  m_editorWindow = NULL;
}

//void Z3DTransferFunctionWidgetWithEditorWindow::mainWindowClosed()
//{
//  if (m_editorWindow)
//    m_editorWindow->close();
//}

void Z3DTransferFunctionWidgetWithEditorWindow::labelClicked()
{
  createEditorWindow();
}
