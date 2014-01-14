#ifndef ZCOLORMAPWIDGETWITHEDITORWINDOW_H
#define ZCOLORMAPWIDGETWITHEDITORWINDOW_H

#include "zclickablelabel.h"
class QMainWindow;
class ZColorMapParameter;
class ZColorMapEditor;
class QDockWidget;

class ZColorMapWidgetWithEditorWindow : public ZClickableColorMapLabel
{
  Q_OBJECT
public:
  explicit ZColorMapWidgetWithEditorWindow(ZColorMapParameter* cm, QMainWindow *minWin, QWidget *parent = 0);
  
signals:
  
public slots:
  void createEditorWindow();
  //void mainWindowClosed();
  void mainWindowDestroyed();

protected:
  virtual void labelClicked();
  virtual void hideEvent(QHideEvent *);

private:
  ZColorMapParameter *m_colorMap;
  ZColorMapEditor *m_colorMapEditor;

  QMainWindow *m_mainWin;
  QDockWidget* m_editorWindow;
};

#endif // ZCOLORMAPWIDGETWITHEDITORWINDOW_H
