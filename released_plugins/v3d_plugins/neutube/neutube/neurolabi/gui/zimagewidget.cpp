#include <QtGui>
#include <cstring>
#include "tz_rastergeom.h"
#include "zimagewidget.h"
#include <cmath>
#include "zpaintbundle.h"

ZImageWidget::ZImageWidget(QWidget *parent, QImage *image) : QWidget(parent)
{
  if (image != NULL) {
    m_viewPort.setRect(0, 0, image->width(), image->height());
  }

  m_projRegion.setRect(0, 0, 0, 0);

  m_zoomRatio = 1;

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setImage(image);
  setCursor(Qt::CrossCursor);
  setMouseTracking(true);
  m_leftButtonMenu = new QMenu(this);
  m_rightButtonMenu = new QMenu(this);
  m_paintBundle = NULL;
}

ZImageWidget::~ZImageWidget()
{
  if (m_isowner == true) {
    if (m_image != NULL) {
      delete m_image;
    }
  }
}

void ZImageWidget::setImage(QImage *image)
{
  if (image != NULL) {
    if (m_viewPort.width() == 0) {
      m_viewPort.setRect(0, 0, image->width(), image->height());
    }
  }

  m_image = image;
  updateGeometry();
  m_isowner = false;
}

void ZImageWidget::setMask(QImage *mask, int channel)
{
  if (channel >= m_mask.size()) {
    m_mask.resize(channel + 1);
  }

  m_mask[channel] = mask;
}

void ZImageWidget::setViewPort(const QRect &rect)
{
  if (m_viewPort != rect) {
    m_viewPort = rect;
  }
}

void ZImageWidget::setProjRegion(const QRect &rect)
{
  if (m_projRegion != rect) {
    m_projRegion = rect;
    //update(QRect(QPoint(0, 0), screenSize()));
  }
}

void ZImageWidget::setView(const QRect &viewPort, const QRect &projRegion)
{
  if ((m_viewPort != viewPort) || (m_projRegion != projRegion)) {
    m_viewPort = viewPort;
    m_projRegion = projRegion;
  }
}

void ZImageWidget::setView(int zoomRatio, const QPoint &zoomOffset)
{
  int dc, ec, dv, ev, dp, ep, ds, es;
  int td, te;

  //QPoint vpOffset = m_viewPort.topLeft();
  QSize vpSize = m_viewPort.size();
  QSize projSize = projectSize();

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    dc = canvasSize().width();
    ec = canvasSize().height();
    dv = vpSize.width();
    ev = vpSize.height();
    dp = projSize.width();
    ep = projSize.height();
    ds = screenSize().width();
    es = screenSize().height();
    td = zoomOffset.x();
    te = zoomOffset.y();
  } else {
    dc = canvasSize().height();
    ec = canvasSize().width();
    dv = vpSize.height();
    ev = vpSize.width();
    dp = projSize.height();
    ep = projSize.width();
    ds = screenSize().height();
    es = screenSize().width();
    td = zoomOffset.y();
    te = zoomOffset.x();
  }

  dp = ds;
  dv = dc / zoomRatio;
  ev = dv * es / dp;
  if (ev > ec - te) {
    ev = ec - te;
  }
  ep = dp * ev / dv;

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    setView(QRect(td, te, dv, ev), QRect(0, 0, dp, ep));
  } else {
    setView(QRect(te, td, ev, dv), QRect(0, 0, ep, dp));
  }
}

void ZImageWidget::setViewPortOffset(int x, int y)
{
  if (x < 0) {
    x = 0;
  }

  if (y < 0) {
    y = 0;
  }

  if (x + m_viewPort.width() > canvasSize().width()) {
    x = canvasSize().width() - m_viewPort.width();
  }

  if (y + m_viewPort.height() > canvasSize().height()) {
    y = canvasSize().height() - m_viewPort.height();
  }

  setViewPort(QRect(x, y, m_viewPort.width(), m_viewPort.height()));
}

void ZImageWidget::setZoomRatio(int zoomRatio)
{
  zoomRatio = std::max(zoomRatio, 1);
  zoomRatio = std::min(getMaxZoomRatio(), zoomRatio);
  if (zoomRatio != m_zoomRatio) {
    m_zoomRatio = zoomRatio;
    zoom(m_zoomRatio);
    update();
  }
}

void ZImageWidget::increaseZoomRatio()
{
  if (m_zoomRatio < getMaxZoomRatio()) {
    m_zoomRatio += 1;
    zoom(m_zoomRatio);
    update();
  }
}

void ZImageWidget::decreaseZoomRatio()
{
  if (m_zoomRatio > 1) {
    m_zoomRatio -= 1;
    zoom(m_zoomRatio);
    update();
  }
}

void ZImageWidget::moveViewPort(int x, int y)
{
  setViewPortOffset(m_viewPort.left() + x, m_viewPort.top() + y);
}

void ZImageWidget::zoom(int zoomRatio, const QPoint &ref)
{
  int dc, ec, dv, ev, dp, ep, ds, es;
  int td, te, td0, te0, cd, ce;

  QPoint vpOffset = m_viewPort.topLeft();
  QSize vpSize = m_viewPort.size();
  QSize projSize = projectSize();

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    dc = canvasSize().width();
    ec = canvasSize().height();
    dv = vpSize.width();
    ev = vpSize.height();
    dp = projSize.width();
    ep = projSize.height();
    ds = screenSize().width();
    es = screenSize().height();
    td = vpOffset.x();
    te = vpOffset.y();
    td0 = vpOffset.x();
    te0 = vpOffset.y();
    cd = ref.x();
    ce = ref.y();
  } else {
    dc = canvasSize().height();
    ec = canvasSize().width();
    dv = vpSize.height();
    ev = vpSize.width();
    dp = projSize.height();
    ep = projSize.width();
    ds = screenSize().height();
    es = screenSize().width();
    td = vpOffset.y();
    te = vpOffset.x();
    td0 = vpOffset.y();
    te0 = vpOffset.x();
    cd = ref.y();
    ce = ref.x();
  }

  int dv0 = dv;
  int ev0 = ev;
  int ep0 = ep;

  dp = ds;
  dv = dc / zoomRatio;
  td = td0 + cd * (dv0 - dv) / (dp - 1);
  if (td < 0) {
    td = 0;
  }

  if (td > dc - dv) {
    td = dc - dv;
  }

  ep = es;
  ev = dv * es / dp;
  if (ev > ec) {
    ev = ec;
  }

  te = ce * (ev0 - 1) / (ep0 - 1) - ce * (ev - 1) / (ep - 1) + te0;
  if (te < 0) {
    te = 0;
  }

  if (ev > ec - te) {
    ev = ec - te;
  }
  ep = dp * ev / dv;

  te = ce * (ev0 - 1) / (ep0 - 1) - ce * (ev - 1) / (ep - 1) + te0;
  if (te > ec - ev) {
    te = ec - ev;
  }
  if (te < 0) {
    te = 0;
  }

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    setView(QRect(td, te, dv, ev), QRect(0, 0, dp, ep));
  } else {
    setView(QRect(te, td, ev, dv), QRect(0, 0, ep, dp));
  }

  /*
  m_srcrect.setRect(zoomOffset.x(), zoomOffset.y(),
                    canvasSize().width() / zoomRatio,
                    canvasSize().height() / zoomRatio);
                    */
}

void ZImageWidget::zoom(int zoomRatio)
{
  if (zoomRatio == 1) {
    setViewPort(QRect(0, 0, canvasSize().width(), canvasSize().height()));
    if (canvasSize().width() * screenSize().height() >=
        canvasSize().height() * screenSize().width()) {
      setProjRegion(QRect(0, 0, screenSize().width(),
                          screenSize().width() * canvasSize().height() /
                          canvasSize().width()));
    } else {
      setProjRegion(QRect(0, 0,
                          screenSize().height() * canvasSize().width() /
                          canvasSize().height(),
                          screenSize().height()));
    }
  } else {
    QPoint ref;
    ref = m_projRegion.center();

    zoom(zoomRatio, ref);
  }
}

void ZImageWidget::paintEvent(QPaintEvent * /*event*/)
{
  if (m_image != NULL) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    //zoom(m_zoomRatio);
    setView(m_zoomRatio, m_viewPort.topLeft());

    QSize size = projectSize();
    painter.drawImage(QRectF(0, 0, size.width(), size.height()), *m_image,
                      m_viewPort);

    for (int i = 0; i < m_mask.size(); ++i) {
      if (m_mask[i] != NULL) {
        painter.drawImage(QRectF(0, 0, size.width(), size.height()),
                            *(m_mask[i]), m_viewPort);
      }
    }
    painter.end();

    if (m_paintBundle) {
      double zoomRatio = size.width() * 1.0 / m_viewPort.width();
      QPainter painter1(this);
#ifdef _DEBUG_
      std::cout << x() - parentWidget()->x() << " "
                << y() - parentWidget()->y()
                << 0.5 - m_viewPort.x() << std::endl;
#endif
      painter1.setRenderHints(QPainter::Antialiasing/* | QPainter::HighQualityAntialiasing*/);

      QTransform transform;
      transform.translate((0.5 - m_viewPort.x())*zoomRatio,
                          (0.5 - m_viewPort.y())*zoomRatio);
      transform.scale(zoomRatio, zoomRatio);
      painter1.setTransform(transform);

      for (ZPaintBundle::const_iterator it = m_paintBundle->begin();
           it != m_paintBundle->end(); ++it) {
        if ((*it)->getTarget() == ZStackDrawable::WIDGET) {
          (*it)->display(painter1, m_paintBundle->sliceIndex(),
                         m_paintBundle->displayStyle());
        }
#ifdef _DEBUG_2
        std::cout << "object painted" << std::endl;
#endif
      }
      painter1.end();
    }

    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    if (m_zoomRatio > 1) {
      painter.setPen(QPen(QColor(0, 0, 255, 128)));
      double ratio = (double) projectSize().width() /
                     canvasSize().width() / 5.0;
      painter.drawRect(0, 0, ratio * canvasSize().width(),
                       ratio * canvasSize().height());
      painter.setPen(QPen(QColor(0, 255, 0, 128)));
      painter.drawRect(ratio * m_viewPort.left(), ratio * m_viewPort.top(),
                       ratio * m_viewPort.width(), ratio * m_viewPort.height());
    }
  }
}

QSize ZImageWidget::minimumSizeHint() const
{
  if (m_image != NULL) {
    return QSize(std::min(300, m_image->width()),
                 std::min(300, m_image->height()));
  } else {
    return QSize(300, 300);
  }
}


QSize ZImageWidget::sizeHint() const
{
  if (m_image == NULL ) {
    return minimumSizeHint();
  } else {
    return m_projRegion.size();
  }
}

bool ZImageWidget::isColorTableRequired()
{
  if (m_image != NULL) {
    if (m_image->format() == QImage::Format_Indexed8) {
      return true;
    }
  }

  return false;
}

void ZImageWidget::addColorTable()
{
  if (m_image != NULL) {
    QVector<QRgb> colorTable(256);
    for (int i = 0; i < colorTable.size(); i++) {
      colorTable[i] = qRgb(i, i, i);
    }
    m_image->setColorTable(colorTable);
  }
}

QSize ZImageWidget::screenSize() const
{
  if (m_image == NULL) {
    return QSize(0, 0);
  } else {
    /*
    int width;
    int height;
    Raster_Ratio_Scale(m_image->width(), m_image->height(),
                       this->width(), this->height(), &width, &height);

    return QSize(width, height);
    */
    return size();
  }
}

QSize ZImageWidget::canvasSize() const
{
  if (m_image == NULL) {
    return QSize(0, 0);
  } else {
    return m_image->size();
  }
}


QPoint ZImageWidget::canvasCoordinate(QPoint widgetCoord) const
{
  QSize csize = projectSize();
  //QSize isize = canvasSize();

  QPoint pt;

  if (csize.width() > 0 && csize.height() > 0) {
    pt.setX(widgetCoord.x() * (m_viewPort.width())/ (csize.width()) + m_viewPort.left());
    pt.setY(widgetCoord.y() * (m_viewPort.height())/ (csize.height()) + m_viewPort.top());
  }
  /*
  pt.setX(Raster_Linear_Map(widgetCoord.x(), 0, csize.width(),
                            m_viewPort.left(), m_viewPort.width()));
  pt.setY(Raster_Linear_Map(widgetCoord.y(), 0, csize.height(),
                            m_viewPort.top(), m_viewPort.height()));
*/

  return pt;
}

QMenu* ZImageWidget::leftMenu()
{
  return m_leftButtonMenu;
}

QMenu* ZImageWidget::rightMenu()
{
  return m_rightButtonMenu;
}

void ZImageWidget::popLeftMenu(const QPoint &pos)
{
  m_leftButtonMenu->popup(mapToGlobal(pos));
}

void ZImageWidget::popRightMenu(const QPoint &pos)
{
  m_rightButtonMenu->popup(mapToGlobal(pos));
}

void ZImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
  emit mouseReleased(event);
}

void ZImageWidget::mouseMoveEvent(QMouseEvent *event)
{
  emit mouseMoved(event);
}

void ZImageWidget::mousePressEvent(QMouseEvent *event)
{
  emit mousePressed(event);
}

void ZImageWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  emit mouseDoubleClicked(event);
}

void ZImageWidget::wheelEvent(QWheelEvent *event)
{
  emit mouseWheelRolled(event);
}

int ZImageWidget::getMaxZoomRatio() const
{
  int ratio = static_cast<int>(std::ceil(std::min(canvasSize().width()*16.0/screenSize().width(),
                                                  canvasSize().height()*16.0/screenSize().height())));
  return std::max(ratio, 16);
}

double ZImageWidget::getAcutalZoomRatioX() const
{
  return static_cast<double>(m_projRegion.width()) / m_viewPort.width();
}

double ZImageWidget::getAcutalZoomRatioY() const
{
  return static_cast<double>(m_projRegion.height()) / m_viewPort.height();
}

double ZImageWidget::getActualOffsetX() const
{
  return static_cast<double>(
        m_viewPort.left() * m_projRegion.right() -
        m_viewPort.right() * m_projRegion.left()) / m_viewPort.width();
}

double ZImageWidget::getActualOffsetY() const
{
  return static_cast<double>(
        m_viewPort.top() * m_projRegion.bottom() -
        m_viewPort.bottom() * m_projRegion.top()) / m_viewPort.height();
}
