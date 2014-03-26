/**@file zimagewidget.h
 * @brief Image widget
 * @author Ting Zhao
 */
#ifndef _ZIMAGEWIDGET_H_
#define _ZIMAGEWIDGET_H_

#include <QImage>
#include <QWidget>
#include <QMenu>
#include <QVector>

class QPaintEvent;
class ZPaintBundle;

/** A class of widget for image display.
 *  Sample usage:
 *    ...
 *    ZImageWidget widget = new ZImageWidget(parent_widget, NULL);
 *
 *   --               -----
 *  |view port  ==>  | project region
 *   --              |
 *                    -----
 */
class ZImageWidget : public QWidget {
  Q_OBJECT

public:
  ZImageWidget(QWidget *parent, QImage *image = NULL);
  virtual ~ZImageWidget();

  inline void setPaintBundle(ZPaintBundle *bd) { m_paintBundle = bd; }

  void setImage(QImage *image);
  void setMask(QImage *mask, int channel);
  void setViewPort(const QRect &rect);
  void setProjRegion(const QRect &rect);
  void setView(int zoomRatio, const QPoint &zoomOffset);
  void setView(const QRect &viewPort, const QRect &projRegion);
  void setViewPortOffset(int x, int y);
  void moveViewPort(int x, int y);
  void setZoomRatio(int zoomRatio);
  inline int zoomRatio() const { return m_zoomRatio; }
  void increaseZoomRatio();
  void decreaseZoomRatio();
  void zoom(int zoomRatio);
  void zoom(int zoomRatio, const QPoint &ref);

  //void setData(const uchar *data, int width, int height, QImage::Format format);
  QSize minimumSizeHint() const;
  QSize sizeHint() const;
  bool isColorTableRequired();
  void addColorTable();

  QSize canvasSize() const;
  QSize screenSize() const;
  inline QSize projectSize() const { return m_projRegion.size(); }
  inline const QRect& viewPort() const { return m_viewPort; }

  QPoint canvasCoordinate(QPoint widgetCoord) const;

  void paintEvent(QPaintEvent *event);

  void popLeftMenu(const QPoint &pos);
  void popRightMenu(const QPoint &pos);

  void showContextMenu(QMenu *menu, const QPoint &pos);

  QMenu* leftMenu();
  QMenu* rightMenu();

  ///{
  /// The actual ration and offset are calculated from the current view port
  /// and project region.
  ///
  //Formula: x0' = x0 * ratio + offset
  /*!
   * \brief Get the actual zoom ratio along X (horizontal)
   */
  double getAcutalZoomRatioX() const;
  /*!
   * \brief Get the actual offset along X (horizontal)
   */
  double getActualOffsetX() const;
  /*!
   * \brief Get the actual zoom ratio along Y (vertical)
   */
  double getAcutalZoomRatioY() const;
  /*!
   * \brief Get the actual offset along Y (vertical)
   */
  double getActualOffsetY() const;
  ///}

public:
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseDoubleClickEvent(QMouseEvent *event);
  virtual void wheelEvent(QWheelEvent *event);

signals:
  void mouseReleased(QMouseEvent*);
  void mouseMoved(QMouseEvent*);
  void mousePressed(QMouseEvent*);
  void mouseDoubleClicked(QMouseEvent*);
  void mouseWheelRolled(QWheelEvent *event);

protected:
  int getMaxZoomRatio() const;

private:
  QImage *m_image;
  QVector<QImage*> m_mask;
  QRect m_viewPort; /* viewport */
  QRect m_projRegion; /* projection region */
  int m_zoomRatio;
  bool m_isowner;
  QMenu *m_leftButtonMenu;
  QMenu *m_rightButtonMenu;
  ZPaintBundle *m_paintBundle;
};

#endif
