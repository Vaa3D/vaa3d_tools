#include "z3dgl.h"
#include "zclickablelabel.h"
#include "zcolormap.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include "z3dtransferfunction.h"
#include "znumericparameter.h"

ZClickableLabel::ZClickableLabel(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
}

void ZClickableLabel::mousePressEvent(QMouseEvent *ev)
{
  if (ev->button() == Qt::LeftButton)
    labelClicked();
}

bool ZClickableLabel::event(QEvent *event)
{
  if(event->type() == QEvent::ToolTip)
  {
    QHelpEvent *helpEvent = dynamic_cast<QHelpEvent *>(event);
    QRect tipRect;
    QString tipText;
    if(getTip(helpEvent->pos(), &tipRect, &tipText))
      QToolTip::showText(
            helpEvent->globalPos(), tipText, this, tipRect);
    else
      QToolTip::hideText();
  }
  return QWidget::event(event);
}

void ZClickableLabel::labelClicked()
{
  emit clicked();
}

ZClickableColorLabel::ZClickableColorLabel(ZVec4Parameter* color, QWidget *parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_vec4Color(color)
  , m_vec3Color(0)
  , m_dvec4Color(0)
  , m_dvec3Color(0)
{
  connect(m_vec4Color, SIGNAL(valueChanged()), this, SLOT(update()));
}

ZClickableColorLabel::ZClickableColorLabel(ZVec3Parameter *color, QWidget *parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_vec4Color(0)
  , m_vec3Color(color)
  , m_dvec4Color(0)
  , m_dvec3Color(0)
{
  connect(m_vec3Color, SIGNAL(valueChanged()), this, SLOT(update()));
}

ZClickableColorLabel::ZClickableColorLabel(ZDVec4Parameter *color, QWidget *parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_vec4Color(0)
  , m_vec3Color(0)
  , m_dvec4Color(color)
  , m_dvec3Color(0)
{
  connect(m_dvec4Color, SIGNAL(valueChanged()), this, SLOT(update()));
}

ZClickableColorLabel::ZClickableColorLabel(ZDVec3Parameter *color, QWidget *parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_vec4Color(0)
  , m_vec3Color(0)
  , m_dvec4Color(0)
  , m_dvec3Color(color)
{
  connect(m_dvec3Color, SIGNAL(valueChanged()), this, SLOT(update()));
}

void ZClickableColorLabel::paintEvent(QPaintEvent *e)
{
  if (!m_vec4Color && !m_vec3Color && !m_dvec4Color && !m_dvec3Color) {
    QWidget::paintEvent(e); // clear the widget
    return;
  }

  QPainter painter(this);
  painter.setBrush(toQColor());
  painter.drawRect(1, 1, rect().width() - 2, rect().height() - 2);
}

QSize ZClickableColorLabel::minimumSizeHint() const
{
  return QSize(50, 33);
}

bool ZClickableColorLabel::getTip(const QPoint &p, QRect *r, QString *s)
{
  if (!m_vec4Color && !m_vec3Color && !m_dvec4Color && !m_dvec3Color)
    return false;

  if(contentsRect().contains(p))
  {
    *r = contentsRect();
    *s = toQColor().name();
    return true;
  }

  return false;
}

void ZClickableColorLabel::labelClicked()
{
  QColor newColor = QColorDialog::getColor(toQColor());
  if(newColor.isValid())
  {
    fromQColor(newColor);
  }
}

QColor ZClickableColorLabel::toQColor()
{
  if (m_vec4Color) {
    return QColor(static_cast<int>(m_vec4Color->get().r * 255.f),
                  static_cast<int>(m_vec4Color->get().g * 255.f),
                  static_cast<int>(m_vec4Color->get().b * 255.f));
  } else if (m_vec3Color) {
    return QColor(static_cast<int>(m_vec3Color->get().r * 255.f),
                  static_cast<int>(m_vec3Color->get().g * 255.f),
                  static_cast<int>(m_vec3Color->get().b * 255.f));
  } else if (m_dvec4Color) {
    return QColor(static_cast<int>(m_dvec4Color->get().r * 255.f),
                  static_cast<int>(m_dvec4Color->get().g * 255.f),
                  static_cast<int>(m_dvec4Color->get().b * 255.f));
  } else if (m_dvec3Color) {
    return QColor(static_cast<int>(m_dvec3Color->get().r * 255.f),
                  static_cast<int>(m_dvec3Color->get().g * 255.f),
                  static_cast<int>(m_dvec3Color->get().b * 255.f));
  } else {
    return QColor(0,0,0);
  }
}

void ZClickableColorLabel::fromQColor(const QColor &col)
{
  if (m_vec4Color)
    m_vec4Color->set(glm::vec4(col.redF(), col.greenF(), col.blueF(), m_vec4Color->get().a));
  if (m_vec3Color)
    m_vec3Color->set(glm::vec3(col.redF(), col.greenF(), col.blueF()));
  if (m_dvec4Color)
    m_dvec4Color->set(glm::dvec4(col.redF(), col.greenF(), col.blueF(), m_dvec4Color->get().a));
  if (m_dvec3Color)
    m_dvec3Color->set(glm::dvec3(col.redF(), col.greenF(), col.blueF()));
}

ZClickableColorMapLabel::ZClickableColorMapLabel(ZColorMapParameter *colorMap, QWidget *parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_colorMap(colorMap)
{
  connect(m_colorMap, SIGNAL(valueChanged()), this, SLOT(update()));
}

void ZClickableColorMapLabel::paintEvent(QPaintEvent *e)
{
  if (!m_colorMap) {
    QWidget::paintEvent(e); // clear the widget
    return;
  }

  QPainter painter(this);

  for(int x = contentsRect().left(); x <= contentsRect().right(); ++x) {
    painter.setPen(m_colorMap->get().getFractionMappedQColor((x*1.-contentsRect().left()) / contentsRect().width()));
    painter.drawLine(x, contentsRect().top(), x, contentsRect().bottom());
  }
}

QSize ZClickableColorMapLabel::minimumSizeHint() const
{
  return QSize(255, 33);
}

bool ZClickableColorMapLabel::getTip(const QPoint &p, QRect *r, QString *s)
{
  if (!m_colorMap)
    return false;

  if(contentsRect().contains(p))
  {
    r->setCoords(p.x(), contentsRect().top(),
                 p.x(), contentsRect().bottom());
    QColor color = m_colorMap->get().getFractionMappedQColor((p.x()*1.-contentsRect().left())/contentsRect().width());
    *s = color.name();
    return true;
  }

  return false;
}

ZClickableTransferFunctionLabel::ZClickableTransferFunctionLabel(Z3DTransferFunctionParameter *transferFunc,
                                                                 QWidget *parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_transferFunction(transferFunc)
{
  connect(m_transferFunction, SIGNAL(valueChanged()), this, SLOT(update()));
}

void ZClickableTransferFunctionLabel::paintEvent(QPaintEvent */*e*/)
{
  QPainter painter(this);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  QColor color1(0, 0, 0);
  QColor color2(255, 255, 255);
  qreal width = contentsRect().width() / 20.;
  qreal height = contentsRect().height() / 4.;
  for (int i=0; i<20; i++) {
    if (i%2==0) {
      painter.fillRect(QRectF(contentsRect().left()+i*width, contentsRect().top(), width, height), color2);
      painter.fillRect(QRectF(contentsRect().left()+i*width, 0.25*(contentsRect().top()+contentsRect().bottom()), width, height), color1);
    } else {
      painter.fillRect(QRectF(contentsRect().left()+i*width, contentsRect().top(), width, height), color1);
      painter.fillRect(QRectF(contentsRect().left()+i*width, 0.25*(contentsRect().top()+contentsRect().bottom()), width, height), color2);
    }
  }

  if (m_transferFunction) {
    for(int x = contentsRect().left(); x <= contentsRect().right(); ++x) {
      double fraction = (x*1.-contentsRect().left()) / contentsRect().width();
      QColor color = m_transferFunction->get().getMappedQColor(fraction);
      painter.setPen(color);
      painter.drawLine(x, contentsRect().top(), x, contentsRect().bottom() * 0.5);
      color.setAlpha(255);
      painter.setPen(color);
      painter.drawLine(x, contentsRect().bottom() * 0.5, x, contentsRect().bottom());
    }
  }
}

QSize ZClickableTransferFunctionLabel::minimumSizeHint() const
{
  return QSize(255, 33);
}

bool ZClickableTransferFunctionLabel::getTip(const QPoint &p, QRect *r, QString *s)
{
  if (!m_transferFunction)
    return false;

  if (contentsRect().contains(p)) {
    r->setCoords(p.x(), contentsRect().top(),
                 p.x(), contentsRect().bottom());
    QColor color = m_transferFunction->get().getFractionMappedQColor((p.x()*1.-contentsRect().left())/contentsRect().width());
    *s = color.name();
    return true;
  }

  return false;
}
