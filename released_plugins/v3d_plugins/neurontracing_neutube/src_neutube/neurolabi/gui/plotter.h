/**@file plotter.h
 * @brief Plotting widget
 * @author Ting Zhao
 */
#ifndef PLOTTER_H
#define PLOTTER_H

#include <QMap>
#include <QPixmap>
#include <QVector>
#include <QWidget>

#include "plotsettings.h"

class QToolButton;

class Plotter : public QWidget
{
  Q_OBJECT

public:
  Plotter(QWidget *parent = 0);

  void setPlotSettings(const PlotSettings &settings);
  void setCurveData(int id, const QVector<QPointF> &data);
  void clearCurve(int id);
  void clearCurveAll();
  QSize minimumSizeHint() const;
  QSize sizeHint() const;

public slots:
  void zoomIn();
  void zoomOut();

protected:
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void wheelEvent(QWheelEvent *event);

private:
  void updateRubberBandRegion();
  void refreshPixmap();
  void drawGrid(QPainter *painter);
  void drawCurves(QPainter *painter);

  enum { Margin = 30 };

  QToolButton *zoomInButton;
  QToolButton *zoomOutButton;
  QMap<int, QVector<QPointF> > curveMap;
  QVector<PlotSettings> zoomStack;
  int curZoom;
  bool rubberBandIsShown;
  QRect rubberBandRect;
  QPixmap pixmap;
};


#endif
