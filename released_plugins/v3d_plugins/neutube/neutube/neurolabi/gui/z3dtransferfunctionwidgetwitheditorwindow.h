#ifndef Z3DTRANSFERFUNCTIONWIDGETWITHEDITORWINDOW_H
#define Z3DTRANSFERFUNCTIONWIDGETWITHEDITORWINDOW_H

#include "zclickablelabel.h"
class Z3DTransferFunctionEditor;
class QMainWindow;
class QDockWidget;

class Z3DTransferFunctionWidgetWithEditorWindow : public ZClickableTransferFunctionLabel
{
  Q_OBJECT
public:
  explicit Z3DTransferFunctionWidgetWithEditorWindow(Z3DTransferFunctionParameter* tf, QMainWindow *minWin, QWidget *parent = 0);

signals:

public slots:
  void createEditorWindow();
  //void mainWindowClosed();
  void mainWindowDestroyed();

protected:
  virtual void labelClicked();
  virtual void hideEvent(QHideEvent *);

private:
  Z3DTransferFunctionParameter *m_transferFunction;
  Z3DTransferFunctionEditor *m_transferFunctionEditor;

  QMainWindow *m_mainWin;
  QDockWidget* m_editorWindow;
  
};

#endif // Z3DTRANSFERFUNCTIONWIDGETWITHEDITORWINDOW_H
