#ifndef Z3DCANVAS_H
#define Z3DCANVAS_H

#include "z3dgl.h"
#include <QGraphicsView>
#include <QGLWidget>
#include <QInputEvent>
class Z3DScene;
class Z3DNetworkEvaluator;
class Z3DCanvasEventListener;
#include <deque>

class Z3DCanvas : public QGraphicsView
{
  Q_OBJECT
public:
  Z3DCanvas(const QString &title, int width = 512, int height = 512,
            const QGLFormat &format = QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba | QGL::SampleBuffers | QGL::AlphaChannel),
            QWidget* parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0);


  virtual ~Z3DCanvas();

  inline QGLFormat format() const { return m_glWidget->format(); }

  void setNetworkEvaluator(Z3DNetworkEvaluator *n);
  void setFakeStereoOnce();

  void addEventListenerToBack(Z3DCanvasEventListener* e);
  void addEventListenerToFront(Z3DCanvasEventListener* e);
  void removeEventListener(Z3DCanvasEventListener *e);
  void clearEventListeners();
  void broadcastEvent(QEvent* e, int w, int h);

  // Set the opengl context of this canvas as the current one.
  inline void getGLFocus() { m_glWidget->makeCurrent(); }
  void toggleFullScreen();
  void forceUpdate() { QPaintEvent *pe = new QPaintEvent(rect()); paintEvent(pe); delete pe; }
  void updateAll() { m_glWidget->update(); }

  // for high dpi support like retina
  glm::ivec2 getPhysicalSize() { return glm::ivec2(width() * getDevicePixelRatio(),
                                                   height() * getDevicePixelRatio()); }
  glm::ivec2 getLogicalSize() { return glm::ivec2(width(), height()); }

  virtual void enterEvent(QEvent* e);
  virtual void leaveEvent(QEvent* e);
  virtual void mousePressEvent(QMouseEvent* e);
  virtual void mouseReleaseEvent (QMouseEvent* e);
  virtual void mouseMoveEvent(QMouseEvent*  e);
  virtual void mouseDoubleClickEvent(QMouseEvent* e);
  virtual void wheelEvent(QWheelEvent* e);
  virtual void timerEvent(QTimerEvent* e);

  virtual void keyPressEvent(QKeyEvent* event);
  virtual void keyReleaseEvent(QKeyEvent* event);

  virtual void resizeEvent(QResizeEvent *event);
  virtual void paintEvent(QPaintEvent *event);
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);

signals:
  // w and h is physical size not logical size, opengl works in physical pixel
  void canvasSizeChanged(int w, int h);

protected:
  double getDevicePixelRatio();

  bool m_fullscreen;

  QGLWidget* m_glWidget;
  Z3DScene* m_3dScene;
  std::deque<Z3DCanvasEventListener*> m_listeners;
};

#endif // Z3DCANVAS_H
