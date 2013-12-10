#ifndef ZSTACKVIEWLOCATOR_H
#define ZSTACKVIEWLOCATOR_H

#include <QRect>
#include <QSize>

class ZStackViewLocator
{
public:
  ZStackViewLocator();

  void setSceneRatio(double ratio);

  void setCanvasSize(int w, int h);

  QRect getViewPort(double cx, double cy, double radius) const;
  int getZoomRatio(int viewPortWidth, int viewPortHeight) const;
  inline double getSceneRatio() const { return m_sceneRatio; }

private:
  double m_sceneRatio; //0 to 1
  QSize m_canvasSize;
};

#endif // ZSTACKVIEWLOCATOR_H
