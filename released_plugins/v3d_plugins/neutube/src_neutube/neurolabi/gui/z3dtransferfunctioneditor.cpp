/*
 * Copyright (C) 2005-2012 University of Muenster, Germany.
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>
 * For a list of authors please refer to the file "CREDITS.txt".
 * Copyright (C) 2012-2013 Korea Institiute of Science and Technologhy, Seoul.
 * Linqing Feng, Jinny Kim's lab <http://jinny.kist.re.kr>
 *
 * This file is derived from code of the free Voreen software package.
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License in the file
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>.
 */

#include "z3dtransferfunctioneditor.h"

#include "z3dtransferfunction.h"
#include "zclickablelabel.h"
#include "z3dvolume.h"

#include "z3dshaderprogram.h"

#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif

#include <math.h>

Z3DTransferFunctionWidget::Z3DTransferFunctionWidget(Z3DTransferFunctionParameter *tf, bool showHistogram, const QString &histogramNormalizeMethod,
                                                     QString xAxisText, QString yAxisText, QWidget* parent)
  : QWidget(parent)
  , m_transferFunction(tf)
  , m_histogramCache(NULL)
  , m_xAxisText(xAxisText)
  , m_yAxisText(yAxisText)
  , m_showHistogram(showHistogram)
  , m_histogramNormalizeMethod(histogramNormalizeMethod)
  , m_volume(tf->getVolume())
{
  m_xRange = glm::dvec2(0., 1.);
  m_yRange = glm::dvec2(0., 1.);
  m_padding = 36;
  m_keyCircleRadius = 5;
  m_selectedLeftPart = true;
  m_splitFactor = 1.2;
  m_dragging = false;

  setObjectName("TransFuncMappingCanvas");
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);

  setFocus();

  QAction* cc = new QAction(tr("Change Color"), this);
  m_keyContextMenu.addAction(cc);
  connect(cc, SIGNAL(triggered()), this, SLOT(changeCurrentColor()));

  m_changeOpacityAction = new QAction(tr("Change Opacity"), this);
  m_keyContextMenu.addAction(m_changeOpacityAction);
  connect(m_changeOpacityAction, SIGNAL(triggered()), this, SLOT(changeCurrentOpacity()));

  m_changeIntensityAction = new QAction(tr("Change Intensity"), this);
  m_keyContextMenu.addAction(m_changeIntensityAction);
  connect(m_changeIntensityAction, SIGNAL(triggered()), this, SLOT(changeCurrentIntensity()));

  m_deleteKeyAction = new QAction(tr("Delete This Key"), this);
  m_keyContextMenu.addAction(m_deleteKeyAction);
  connect(m_deleteKeyAction, SIGNAL(triggered()), this, SLOT(deleteKey()));

  if (m_volume)
    connect(m_volume, SIGNAL(histogramFinished()), this, SLOT(update()));
  connect(m_transferFunction, SIGNAL(valueChanged()), this, SLOT(update()));
}

Z3DTransferFunctionWidget::~Z3DTransferFunctionWidget()
{
  delete m_histogramCache;
}

void Z3DTransferFunctionWidget::showNoKeyContextMenu(QMouseEvent *event)
{
  m_noKeyContextMenu.popup(event->globalPos());
}

void Z3DTransferFunctionWidget::showKeyContextMenu(QMouseEvent* event, size_t selectedKeyIndex)
{
  if (!m_transferFunction)
    return;

  if (selectedKeyIndex == 0 || selectedKeyIndex == m_transferFunction->get().getNumKeys()-1) {
    // first or last key, can not delete, can not change intensity
    m_deleteKeyAction->setEnabled(false);
    m_changeIntensityAction->setEnabled(false);
  } else {
    m_deleteKeyAction->setEnabled(true);
    m_changeIntensityAction->setEnabled(true);
  }

  m_keyContextMenu.popup(event->globalPos());
}

void Z3DTransferFunctionWidget::paintEvent(QPaintEvent* event)
{
  if (!m_transferFunction)
    return;

  event->accept();

  QPainter paint(this);

  paint.setRenderHint(QPainter::Antialiasing, false);
  paint.setPen(Qt::NoPen);
  paint.setBrush(Qt::white);
  paint.drawRect(0, 0, width() - 1, height() - 1);

  if (m_showHistogram) {
    if (m_histogramCache == NULL || m_histogramCache->rect() != rect()) {
      delete m_histogramCache;
      m_histogramCache = NULL;
      if (m_volume && m_volume->hasHistogram()) {
        m_histogramCache = new QPixmap(rect().size());
        m_histogramCache->fill(Qt::transparent);

        QPainter cachePaint(m_histogramCache);
        // draw histogram
        cachePaint.setPen(Qt::NoPen);
        cachePaint.setBrush(QColor(0, 40, 160, 120));
        cachePaint.setRenderHint(QPainter::Antialiasing, true);

        size_t histogramWidth = m_volume->getHistogramBinCount();
        double barWidth = 1.0/(histogramWidth-1.0);

        for (size_t x=0; x<histogramWidth; ++x) {
          double sxpos = x / (histogramWidth - 1.0);
          double expos = sxpos + 0.5 * barWidth;
          sxpos = sxpos - 0.5 * barWidth;
          if (x == 0)
            sxpos += 0.5 * barWidth;
          if (x == histogramWidth-1)
            expos -= 0.5 * barWidth;
          double value;
          if (m_histogramNormalizeMethod == "Log")
            value = m_volume->getLogNormalizedHistogramValue(x);
          else
            value = m_volume->getNormalizedHistogramValue(x);
          glm::dvec2 p1 = relativeToPixelCoordinates(glm::dvec2(sxpos, value * (m_yRange[1] - m_yRange[0]) + m_yRange[0]));
          glm::dvec2 p2 = relativeToPixelCoordinates(glm::dvec2(expos, m_yRange[0]));
          QPointF topLeft(p1.x, p1.y);
          QPointF bottomRight(p2.x, p2.y);
          cachePaint.drawRect(QRectF(topLeft, bottomRight));
        }
      } else if (m_volume) {
        m_volume->asyncGenerateHistogram();
      }
    }

    if (m_histogramCache)
      paint.drawPixmap(0, 0, *m_histogramCache);
  }

  // ----------------------------------------------

  // draw grid
  paint.setPen(QColor(220, 220, 220));
  paint.setRenderHint(QPainter::Antialiasing, false);

  glm::dvec2 pmin = glm::dvec2(0., 0.);
  glm::dvec2 pmax = glm::dvec2(1., 1.);

  glm::dvec2 gridSpacing(.1, .1);
  for (double f=pmin.x; f<pmax.x+gridSpacing.x*0.5; f+=gridSpacing.x) {
    glm::dvec2 p = relativeToPixelCoordinates(glm::dvec2(f, 0.));
    glm::dvec2 a = relativeToPixelCoordinates(glm::dvec2(0., 0.));
    glm::dvec2 b = relativeToPixelCoordinates(glm::dvec2(0., 1.));
    paint.drawLine(QPointF(p.x, a.y),
                   QPointF(p.x, b.y));
  }

  for (double f=pmin.y; f<pmax.y+gridSpacing.y*0.5; f+=gridSpacing.y) {
    glm::dvec2 p = relativeToPixelCoordinates(glm::dvec2(0., f));
    glm::dvec2 a = relativeToPixelCoordinates(glm::dvec2(0., 0.));
    glm::dvec2 b = relativeToPixelCoordinates(glm::dvec2(1., 0.));
    paint.drawLine(QPointF(a.x, p.y),
                   QPointF(b.x, p.y));
  }

  // draw x and y axes
  paint.setRenderHint(QPainter::Antialiasing, true);
  paint.setPen(Qt::gray);
  paint.setBrush(Qt::gray);

  // draw axes independently from visible range
  double oldx0 = m_xRange[0];
  double oldx1 = m_xRange[1];
  m_xRange[0] = 0.;
  m_xRange[1] = 1.;

  glm::dvec2 origin = relativeToPixelCoordinates(glm::dvec2(0., 0.));
  origin.x = floor(origin.x) + 0.5;
  origin.y = floor(origin.y) + 0.5;

  paint.setRenderHint(QPainter::Antialiasing, true);

  paint.drawLine(QPointF(m_padding, origin.y),
                 QPointF(width() - m_padding, origin.y));

  paint.drawLine(QPointF(origin.x, m_padding),
                 QPointF(origin.x, height() - m_padding));

  paint.drawText(static_cast<int>(width()/2.0 - 6.2 * m_padding), static_cast<int>(origin.y + 0.2 * m_padding),
                 static_cast<int>(6.2 * 2 * m_padding), static_cast<int>(0.6 * m_padding),
                 Qt::AlignHCenter | Qt::AlignVCenter, m_xAxisText);
  paint.drawText(static_cast<int>(0.15 * m_padding), static_cast<int>(m_padding - 2.0),
                 static_cast<int>(0.7 * m_padding), static_cast<int>(7.0 * 2),
                 Qt::AlignRight | Qt::AlignTop, "1.0");
  paint.drawText(static_cast<int>(0.15 * m_padding), static_cast<int>(height() - m_padding - 9.0),
                 static_cast<int>(0.7 * m_padding), static_cast<int>(14.0),
                 Qt::AlignRight | Qt::AlignBottom, "0.0");
  paint.drawText(static_cast<int>(origin.x - 3), static_cast<int>(height() - m_padding * 0.85),
                 static_cast<int>(m_padding * 2), static_cast<int>(0.7 * m_padding),
                 Qt::AlignLeft | Qt::AlignTop, "0.0");
  paint.drawText(static_cast<int>(width() - m_padding * 3.0), static_cast<int>(height() - m_padding * 0.85),
                 static_cast<int>(m_padding * 2.0 + 3), static_cast<int>(0.7 * m_padding),
                 Qt::AlignRight | Qt::AlignTop, "1.0");
  paint.save();
  paint.translate(0.2 * m_padding, height()/2.0 + 6.2 * m_padding);
  paint.rotate(270.);
  paint.drawText(0, 0,
                 static_cast<int>(6.2 * 2 * m_padding), static_cast<int>(0.6 * m_padding),
                 Qt::AlignHCenter | Qt::AlignVCenter, m_yAxisText);
  paint.restore();

  m_xRange[0] = oldx0;
  m_xRange[1] = oldx1;

  // ----------------------------------------------

  // draw mapping function
  QPen pen = QPen(Qt::darkRed);
  pen.setWidthF(1.5);
  paint.setPen(pen);

  origin = relativeToPixelCoordinates(glm::dvec2(0.));

  glm::dvec2 old;
  for (size_t i=0; i<m_transferFunction->get().getNumKeys(); ++i) {
    ZColorMapKey &key = m_transferFunction->get().getKey(i);
    glm::dvec2 p = relativeToPixelCoordinates(glm::dvec2(key.getIntensity(), key.getFloatAlphaL()));
    if (i == 0)  {
      if (m_transferFunction->get().getKeyIntensity(0) > 0.)
        paint.drawLine(QPointF(relativeToPixelCoordinates(glm::dvec2(0., 0.)).x, p.y),
                       QPointF(p.x - 1., p.y));
    }
    else {
      paint.drawLine(QPointF(old.x + 1., old.y),
                     QPointF(p.x - 1., p.y));
    }
    old = p;
    if (key.isSplit())
      old = relativeToPixelCoordinates(glm::dvec2(key.getIntensity(), key.getFloatAlphaR()));
  }
  if (m_transferFunction->get().getKeyIntensity(m_transferFunction->get().getNumKeys()-1) < 1.) {
    paint.drawLine(QPointF(old.x + 1., old.y),
                   QPointF(relativeToPixelCoordinates(glm::dvec2(1., 0.)).x, old.y));
  }

  if (m_xRange[1] != m_xRange[0])
    paintKeys(paint);

  // ----------------------------------------------

  paint.setRenderHint(QPainter::Antialiasing, false);

  paint.setPen(Qt::lightGray);
  paint.setBrush(Qt::NoBrush);
  paint.drawRect(0, 0, width() - 1, height() - 1);
}

void Z3DTransferFunctionWidget::mousePressEvent(QMouseEvent* event)
{
  event->accept();
  m_transferFunction->get().deselectAllKeys();

  size_t selectedKeyIndex;
  bool selectedKey = findkey(event->pos(), selectedKeyIndex, m_selectedLeftPart);
  if (selectedKey)
    m_transferFunction->get().setKeySelected(selectedKeyIndex, true);

  glm::dvec2 sHit = glm::dvec2(event->x(), event->y());
  glm::dvec2 hit = pixelToRelativeCoordinates(sHit);

  if (event->button() == Qt::RightButton) {
    if (!selectedKey)
      showNoKeyContextMenu(event);
    else
      showKeyContextMenu(event, selectedKeyIndex);
    return;
  }

  if (selectedKey && event->button() == Qt::LeftButton) {
    m_dragging = true;
    //keep values within valid range
    hit = glm::clamp(hit, 0., 1.);
    showKeyInfo(event->pos(), hit);
    return;
  }

  // no key was selected -> insert new key
  if (hit.x >= 0. && hit.x <= 1. &&
      hit.y >= 0. && hit.y <= 1. &&
      event->button() == Qt::LeftButton)
  {
    insertNewKey(hit);
    m_dragging = true;
    showKeyInfo(event->pos(), hit);
  }
}

void Z3DTransferFunctionWidget::mouseMoveEvent(QMouseEvent* event)
{
  event->accept();

  glm::dvec2 sHit = glm::dvec2(event->x(), event->y());
  glm::dvec2 hit = pixelToRelativeCoordinates(sHit);

  // return when no key was inserted or selected
  if (!m_dragging)
    return;

  // keep location within valid texture coord range
  hit = glm::clamp(hit, 0., 1.);

  std::vector<size_t> allSelected = m_transferFunction->get().getSelectedKeyIndexes();

  if (allSelected.size() == 1) {
    size_t selectedIdx = allSelected[0];
    if (selectedIdx != 0 && selectedIdx != m_transferFunction->get().getNumKeys()-1) {
      if (hit.x <= m_transferFunction->get().getKeyIntensity(selectedIdx-1))
        hit.x = m_transferFunction->get().getKeyIntensity(selectedIdx-1) + 1e-7;
      if (hit.x >= m_transferFunction->get().getKeyIntensity(selectedIdx+1))
        hit.x = m_transferFunction->get().getKeyIntensity(selectedIdx+1) - 1e-7;
      m_transferFunction->get().setKeyIntensity(selectedIdx, hit.x);
    }
    if (m_transferFunction->get().isKeySplit(selectedIdx)) {
      if (m_selectedLeftPart)
        m_transferFunction->get().setKeyFloatAlphaL(selectedIdx, hit.y);
      else
        m_transferFunction->get().setKeyFloatAlphaR(selectedIdx, hit.y);
    } else {
      m_transferFunction->get().setKeyFloatAlphaL(selectedIdx, hit.y);
    }
    showKeyInfo(event->pos(), hit);
  }
}

void Z3DTransferFunctionWidget::mouseReleaseEvent(QMouseEvent* event)
{
  event->accept();
  if (event->button() == Qt::LeftButton) {
    m_dragging = false;
    hideKeyInfo();
  }
}

void Z3DTransferFunctionWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  event->accept();
  if (event->button() == Qt::LeftButton)
    changeCurrentColor();
}

void Z3DTransferFunctionWidget::keyReleaseEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
    event->accept();
    deleteKey();
  }
}

bool Z3DTransferFunctionWidget::event(QEvent *e)
{
  if(e->type() == QEvent::ToolTip)
  {
    size_t index;
    bool isLeftPart;
    QHelpEvent *helpEvent = dynamic_cast<QHelpEvent *>(e);
    QRect tipRect;
    QString tipText;
    if (findkey(helpEvent->pos(), index, isLeftPart)) {
      glm::dvec2 p = relativeToPixelCoordinates(glm::dvec2(m_transferFunction->get().getKeyIntensity(index), m_transferFunction->get().getKeyFloatAlphaL(index)));
      if (m_transferFunction->get().isKeySplit(index)) {
        tipRect = QRect(p.x - m_splitFactor * m_keyCircleRadius, p.y - m_keyCircleRadius, m_splitFactor * m_keyCircleRadius * 2, m_keyCircleRadius * 2);
        if (isLeftPart)
          tipText = QString("Key %1 Left\nIntensity: %2\nColor: %3\nOpacity: %4").arg(index+1).arg(m_transferFunction->get().getKeyIntensity(index))
              .arg(m_transferFunction->get().getKeyQColorL(index).name()).arg(m_transferFunction->get().getKeyFloatAlphaL(index));
        else
          tipText = QString("Key %1 Right\nIntensity: %2\nColor: %3\nOpacity: %4").arg(index+1).arg(m_transferFunction->get().getKeyIntensity(index))
              .arg(m_transferFunction->get().getKeyQColorR(index).name()).arg(m_transferFunction->get().getKeyFloatAlphaR(index));
      } else {
        tipRect = QRect(p.x - m_keyCircleRadius, p.y - m_keyCircleRadius, m_keyCircleRadius * 2, m_keyCircleRadius * 2);
        tipText = QString("Key %1\nIntensity: %2\nColor: %3\nOpacity: %4").arg(index+1).arg(m_transferFunction->get().getKeyIntensity(index))
            .arg(m_transferFunction->get().getKeyQColorR(index).name()).arg(m_transferFunction->get().getKeyFloatAlphaR(index));
      }
      QToolTip::showText(helpEvent->globalPos(), tipText, this, tipRect);
    } else
      QToolTip::hideText();
  }
  return QWidget::event(e);
}

bool Z3DTransferFunctionWidget::findkey(const QPoint &pos, size_t &index, bool &isLeftPart)
{
  glm::dvec2 sHit = glm::dvec2(pos.x(), pos.y());

  double tol = 1.5;

  // see if a key was selected
  int selectedKeyIndex = -1;
  for (size_t i=1; i<m_transferFunction->get().getNumKeys(); ++i) {
    glm::dvec2 sp = relativeToPixelCoordinates(glm::dvec2(m_transferFunction->get().getKeyIntensity(i), m_transferFunction->get().getKeyFloatAlphaL(i)));
    glm::dvec2 spr = relativeToPixelCoordinates(glm::dvec2(m_transferFunction->get().getKeyIntensity(i), m_transferFunction->get().getKeyFloatAlphaR(i)));
    if (m_transferFunction->get().isKeySplit(i)) {
      if (sHit.x > sp.x - m_splitFactor * m_keyCircleRadius * tol && sHit.x <= sp.x &&
          sHit.y > sp.y - m_keyCircleRadius * tol && sHit.y < sp.y + m_keyCircleRadius * tol)
      {
        index = i;
        selectedKeyIndex = static_cast<int>(i);
        isLeftPart = true;
        break;
      }
      if (sHit.x >= spr.x && sHit.x < spr.x + m_splitFactor * m_keyCircleRadius * tol &&
          sHit.y > spr.y - m_keyCircleRadius * tol && sHit.y < spr.y + m_keyCircleRadius * tol)
      {
        index = i;
        selectedKeyIndex = static_cast<int>(i);
        isLeftPart = false;
        break;
      }
    }
    else {
      if (sHit.x > sp.x - m_keyCircleRadius * tol && sHit.x < sp.x + m_keyCircleRadius * tol &&
          sHit.y > sp.y - m_keyCircleRadius * tol && sHit.y < sp.y + m_keyCircleRadius * tol)
      {
        index = i;
        selectedKeyIndex = static_cast<int>(i);
        isLeftPart = false;
        break;
      }
    }
  }
  // check first key
  if (selectedKeyIndex == -1) {
    glm::dvec2 sp = relativeToPixelCoordinates(glm::dvec2(m_transferFunction->get().getKeyIntensity(0), m_transferFunction->get().getKeyFloatAlphaL(0)));
    glm::dvec2 spr = relativeToPixelCoordinates(glm::dvec2(m_transferFunction->get().getKeyIntensity(0), m_transferFunction->get().getKeyFloatAlphaR(0)));
    if (m_transferFunction->get().isKeySplit(0)) {
      if (sHit.x > sp.x - m_splitFactor * m_keyCircleRadius * tol && sHit.x <= sp.x &&
          sHit.y > sp.y - m_keyCircleRadius * tol && sHit.y < sp.y + m_keyCircleRadius * tol)
      {
        index = 0;
        selectedKeyIndex = 0;
        isLeftPart = true;
      }
      if (sHit.x >= spr.x && sHit.x < spr.x + m_splitFactor * m_keyCircleRadius * tol &&
          sHit.y > spr.y - m_keyCircleRadius * tol && sHit.y < spr.y + m_keyCircleRadius * tol)
      {
        index = 0;
        selectedKeyIndex = 0;
        isLeftPart = false;
      }
    }
    else {
      if (sHit.x > sp.x - m_keyCircleRadius * tol && sHit.x < sp.x + m_keyCircleRadius * tol &&
          sHit.y > sp.y - m_keyCircleRadius * tol && sHit.y < sp.y + m_keyCircleRadius * tol)
      {
        index = 0;
        selectedKeyIndex = 0;
        isLeftPart = false;
      }
    }
  }
  if (selectedKeyIndex != -1)
    return true;
  else
    return false;
}

void Z3DTransferFunctionWidget::setHistogramNormalizeMethod(const QString &method)
{
  if (m_histogramNormalizeMethod != method) {
    m_histogramNormalizeMethod = method;
    delete m_histogramCache;
    m_histogramCache = 0;
    update();
  }
}

void Z3DTransferFunctionWidget::setHistogramVisible(bool v)
{
  if (v != m_showHistogram) {
    m_showHistogram = v;
    update();
  }
}

void Z3DTransferFunctionWidget::deleteKey()
{
  if (!m_transferFunction || m_transferFunction->get().getNumKeys() < 3)
    return;
  std::vector<size_t> allSelected = m_transferFunction->get().getSelectedKeyIndexes();
  if (allSelected.size() != 1)
    return;
  if (allSelected[0] == 0 || allSelected[0] == m_transferFunction->get().getNumKeys()-1) {
    return;
  }

  m_transferFunction->get().removeSelectedKeys();
}

void Z3DTransferFunctionWidget::changeCurrentColor()
{
  std::vector<size_t> allSelected = m_transferFunction->get().getSelectedKeyIndexes();
  if (allSelected.size() != 1)
    return;

  size_t selectedIdx = allSelected[0];

  QColor oldColor;
  if (m_transferFunction->get().isKeySplit(selectedIdx) && !m_selectedLeftPart)
    oldColor = m_transferFunction->get().getKeyQColorR(selectedIdx);
  else
    oldColor = m_transferFunction->get().getKeyQColorL(selectedIdx);

  QColor newColor = QColorDialog::getColor(oldColor, 0);
  if (newColor.isValid() && oldColor != newColor) {
    if (m_transferFunction->get().isKeySplit(selectedIdx) && !m_selectedLeftPart) {
      newColor.setAlpha(m_transferFunction->get().getKeyAlphaR(selectedIdx));
      m_transferFunction->get().setKeyColorR(selectedIdx, newColor);
    }
    else {
      newColor.setAlpha(m_transferFunction->get().getKeyAlphaL(selectedIdx));
      m_transferFunction->get().setKeyColorL(selectedIdx, newColor);
    }
  }
}

void Z3DTransferFunctionWidget::changeCurrentIntensity()
{
  std::vector<size_t> allSelected = m_transferFunction->get().getSelectedKeyIndexes();
  if (allSelected.size() != 1)
    return;

  size_t index = allSelected[0];

  bool ok;
  double newI = QInputDialog::getDouble(this, QString("Key %1").arg(index+1), "Intensity:",
                                        m_transferFunction->get().getKeyIntensity(index),
                                        m_transferFunction->get().getDomainMin() + 0.001,
                                        m_transferFunction->get().getDomainMax() - 0.001,
                                        3, &ok);
  if (ok) {
    m_transferFunction->get().setKeyIntensity(index, newI);
  }
}

void Z3DTransferFunctionWidget::changeCurrentOpacity()
{
  std::vector<size_t> allSelected = m_transferFunction->get().getSelectedKeyIndexes();
  if (allSelected.size() != 1)
    return;

  size_t index = allSelected[0];

  double oldOpacity;
  if (m_transferFunction->get().isKeySplit(index) && !m_selectedLeftPart)
    oldOpacity = m_transferFunction->get().getKeyFloatAlphaR(index);
  else
    oldOpacity = m_transferFunction->get().getKeyFloatAlphaL(index);

  bool ok;
  double newO = QInputDialog::getDouble(this, QString("Key %1").arg(index+1), "Opacity:",
                                        oldOpacity,
                                        0.0,
                                        1.0,
                                        3, &ok);
  if (ok) {
    if (m_transferFunction->get().isKeySplit(index) && !m_selectedLeftPart)
      m_transferFunction->get().setKeyFloatAlphaR(index, newO);
    else
      m_transferFunction->get().setKeyFloatAlphaL(index, newO);
  }
}

void Z3DTransferFunctionWidget::insertNewKey(glm::dvec2& hit)
{
  if (!m_transferFunction)
    return;

  hit = glm::clamp(hit, 0., 1.);

  m_transferFunction->get().deselectAllKeys();
  m_transferFunction->get().addKeyAtIntensity(hit.x, hit.y, true);
}

void Z3DTransferFunctionWidget::paintKeys(QPainter& paint)
{
  if (!m_transferFunction)
    return;

  for (size_t i=0; i<m_transferFunction->get().getNumKeys(); ++i) {
    glm::dvec2 p = relativeToPixelCoordinates(glm::dvec2(m_transferFunction->get().getKeyIntensity(i), m_transferFunction->get().getKeyFloatAlphaL(i)));
    QPen pen(QBrush(Qt::darkGray), Qt::SolidLine);
    double radiusScale = 1.0;

    if (m_transferFunction->get().isKeySplit(i)) {
      // left
      if (m_transferFunction->get().isKeySelected(i) && m_selectedLeftPart) {
        pen.setWidth(3);
        pen.setColor(Qt::black);
        radiusScale = 1.3;
      }
      paint.setPen(pen);
      QColor color = m_transferFunction->get().getKeyQColorL(i);
      color.setAlpha(255);
      paint.setBrush(color);

      paint.drawPie(QRectF(p.x - m_splitFactor * m_keyCircleRadius * radiusScale, p.y - m_keyCircleRadius * radiusScale,
                           m_splitFactor * m_keyCircleRadius * radiusScale * 2, m_keyCircleRadius * radiusScale * 2),
                    90 * 16, 180 * 16);

      // right
      p = relativeToPixelCoordinates(glm::dvec2(m_transferFunction->get().getKeyIntensity(i), m_transferFunction->get().getKeyFloatAlphaR(i)));
      if (m_transferFunction->get().isKeySelected(i) && !m_selectedLeftPart) {
        pen.setWidth(3);
        pen.setColor(Qt::black);
        radiusScale = 1.3;
      } else {
        pen.setWidth(1);
        pen.setColor(Qt::darkGray);
        radiusScale = 1.0;
      }
      paint.setPen(pen);
      color = m_transferFunction->get().getKeyQColorR(i);
      color.setAlpha(255);
      paint.setBrush(color);

      paint.drawPie(QRectF(p.x - m_splitFactor * m_keyCircleRadius * radiusScale, p.y - m_keyCircleRadius * radiusScale,
                           m_splitFactor * m_keyCircleRadius * radiusScale * 2, m_keyCircleRadius * radiusScale * 2),
                    270 * 16, 180 * 16);
    } else {
      if (m_transferFunction->get().isKeySelected(i)) {
        pen.setWidth(3);
        pen.setColor(Qt::black);
        radiusScale = 1.3;
      }
      paint.setPen(pen);
      QColor color = m_transferFunction->get().getKeyQColorL(i);
      color.setAlpha(255);
      paint.setBrush(color);

      paint.drawEllipse(QRectF(p.x - m_keyCircleRadius * radiusScale, p.y - m_keyCircleRadius * radiusScale,
                               m_keyCircleRadius * radiusScale * 2, m_keyCircleRadius * radiusScale * 2));
    }
  }
}

glm::dvec2 Z3DTransferFunctionWidget::relativeToPixelCoordinates(glm::dvec2 r)
{
  double px = (r.x - m_xRange[0]) / (m_xRange[1] - m_xRange[0]) * (static_cast<double>(width())  - 2 * m_padding) + m_padding;
  double py = height() - ((r.y - m_yRange[0]) / (m_yRange[1] - m_yRange[0]) * (static_cast<double>(height()) - 2 * m_padding) + m_padding);
  return glm::dvec2(px, py);
}

glm::dvec2 Z3DTransferFunctionWidget::pixelToRelativeCoordinates(glm::dvec2 p)
{
  double rx = (p.x - m_padding) / (static_cast<double>(width())  - 2 * m_padding) * (m_xRange[1] - m_xRange[0]) + m_xRange[0];
  double ry = (height() - p.y - m_padding) / (static_cast<double>(height()) - 2 * m_padding) * (m_yRange[1] - m_yRange[0]) + m_yRange[0];
  return glm::dvec2(rx, ry);
}

QSize Z3DTransferFunctionWidget::minimumSizeHint () const
{
  return QSize(300, 185);
}

QSize Z3DTransferFunctionWidget::sizeHint () const
{
  return QSize(300, 185);
}

QSizePolicy Z3DTransferFunctionWidget::sizePolicy () const
{
  return QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void Z3DTransferFunctionWidget::hideKeyInfo()
{
  QToolTip::hideText();
}

void Z3DTransferFunctionWidget::showKeyInfo(QPoint pos, glm::dvec2 values)
{
  QToolTip::showText(mapToGlobal(pos), QString("Intensity: %1 \nOpacity: %2").arg(values.x).arg(values.y));
}

void Z3DTransferFunctionWidget::volumeChanged(Z3DVolume* volume)
{
  delete m_histogramCache;
  m_histogramCache = NULL;

  m_volume = volume;
  connect(m_volume, SIGNAL(histogramFinished()), this, SLOT(update()));
}

void Z3DTransferFunctionWidget::setTransFunc(Z3DTransferFunctionParameter *tf)
{
  m_transferFunction = tf;
  update();
}

//----------------------------------------------------------------------------------------------------------------

Z3DTransferFunctionEditor::Z3DTransferFunctionEditor(Z3DTransferFunctionParameter* para, QWidget* parent)
  : QWidget(parent)
  , m_transferFunction(para)
  , m_volume(NULL)
  , m_transferFunctionWidget(NULL)
  , m_transferFunctionTexture(NULL)
  , m_showHistogram("Show Histogram: ", true)
  , m_histogramNormalizeMethod("Histogram Normalize Method: ")
{
  m_histogramNormalizeMethod.addOptions("Linear", "Log");
  m_histogramNormalizeMethod.select("Log");
  createWidgets();
  updateFromTransferFunction();
  createConnections();
}

Z3DTransferFunctionEditor::~Z3DTransferFunctionEditor()
{
}

QLayout* Z3DTransferFunctionEditor::createMappingLayout()
{
  m_transferFunctionWidget = new Z3DTransferFunctionWidget(m_transferFunction,
                                                           m_showHistogram.get(), m_histogramNormalizeMethod.get());
  m_transferFunctionWidget->setMinimumWidth(140);

  QWidget* additionalSpace = new QWidget();
  additionalSpace->setMinimumHeight(2);

  //histogram
  QHBoxLayout *hboxHist = new QHBoxLayout();
  hboxHist->addWidget(m_showHistogram.createNameLabel());
  hboxHist->addWidget(m_showHistogram.createWidget());
  hboxHist->addStretch();
  hboxHist->addWidget(m_histogramNormalizeMethod.createNameLabel());
  hboxHist->addWidget(m_histogramNormalizeMethod.createWidget());

  //data bounds
  QHBoxLayout* hboxData = new QHBoxLayout();
  m_dataMinNameLabel = new QLabel("Data Min: ", this);
  m_dataMinNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_dataMaxNameLabel = new QLabel("Data Max: ", this);
  m_dataMaxNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_dataMinValueLabel = new QLabel();
  m_dataMaxValueLabel = new QLabel();
  m_dataMinValueLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_dataMaxValueLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  hboxData->addSpacing(6);
  hboxData->addWidget(m_dataMinNameLabel);
  hboxData->addWidget(m_dataMinValueLabel);
  hboxData->addStretch();
  hboxData->addWidget(m_dataMaxNameLabel);
  hboxData->addWidget(m_dataMaxValueLabel);
  hboxData->addStretch();
  hboxData->addSpacing(21);

  //domain settings:
  QHBoxLayout* hboxDomain = new QHBoxLayout();
  m_domainMinSpinBox = new QDoubleSpinBox();
  m_domainMaxSpinBox = new QDoubleSpinBox();
  m_domainMaxSpinBox->setRange(0.001, 1.0);
  m_domainMaxSpinBox->setSingleStep(.001);
  m_domainMaxSpinBox->setDecimals(3);
  m_domainMaxSpinBox->setKeyboardTracking(false);
  m_domainMinSpinBox->setRange(0., 0.999);
  m_domainMinSpinBox->setSingleStep(.001);
  m_domainMinSpinBox->setDecimals(3);
  m_domainMinSpinBox->setKeyboardTracking(false);

  m_fitDomainToDataButton = new QPushButton();
  m_fitDomainToDataButton->setText("Fit to Data");

  m_domainMinNameLabel = new QLabel("TF Start: ", this);
  m_domainMaxNameLabel = new QLabel("TF End: ", this);
  m_domainMinNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_domainMaxNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  m_rescaleKeys = new QCheckBox("Rescale Keys", this);
  m_rescaleKeys->setToolTip("If set, reserve all keys (by rescaling) while changing the domain. Otherwise out of domain keys will be deleted.");
  m_rescaleKeys->setChecked(false);

  hboxDomain->addWidget(m_domainMinNameLabel);
  hboxDomain->addWidget(m_domainMinSpinBox);
  hboxDomain->addWidget(m_domainMaxNameLabel);
  hboxDomain->addWidget(m_domainMaxSpinBox);
  hboxDomain->addWidget(m_fitDomainToDataButton);
  hboxDomain->addWidget(m_rescaleKeys);

  QPushButton *resetButton = new QPushButton("Reset", this);
  connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

  m_transferFunctionTexture = new ZClickableTransferFunctionLabel(m_transferFunction);

  // put widgets in layout
  QVBoxLayout* vBox = new QVBoxLayout();
  vBox->setMargin(0);
  vBox->setSpacing(1);
  vBox->addStretch();
  vBox->addWidget(m_transferFunctionWidget, 1);
  vBox->addWidget(additionalSpace);
  vBox->addLayout(hboxHist);
  vBox->addLayout(hboxData);
  vBox->addLayout(hboxDomain);
  vBox->addSpacing(1);
  vBox->addWidget(resetButton);
  vBox->addSpacing(2);
  vBox->addWidget(m_transferFunctionTexture);

  return vBox;
}

void Z3DTransferFunctionEditor::createWidgets()
{
  QLayout* mappingLayout = createMappingLayout();

  mappingLayout->setMargin(5);
  setLayout(mappingLayout);
}

void Z3DTransferFunctionEditor::createConnections()
{
  connect(m_transferFunction, SIGNAL(valueChanged()), this, SLOT(updateFromTransferFunction()));

  connect(m_domainMinSpinBox, SIGNAL(valueChanged(double)), this, SLOT(domainMinSpinBoxChanged(double)));
  connect(m_domainMaxSpinBox, SIGNAL(valueChanged(double)), this, SLOT(domainMaxSpinBoxChanged(double)));
  connect(m_fitDomainToDataButton, SIGNAL(clicked()), this, SLOT(fitDomainToData()));
  connect(&m_showHistogram, SIGNAL(valueChanged(bool)), m_transferFunctionWidget, SLOT(setHistogramVisible(bool)));
  connect(&m_histogramNormalizeMethod, SIGNAL(valueChanged()), this, SLOT(changeHistogramNormalizeMethod()));
}

void Z3DTransferFunctionEditor::changeHistogramNormalizeMethod()
{
  m_transferFunctionWidget->setHistogramNormalizeMethod(m_histogramNormalizeMethod.get());
}

void Z3DTransferFunctionEditor::updateFromTransferFunction()
{
  assert(m_transferFunction);

  // check whether the volume associated with the TransFuncProperty has changed
  Z3DVolume* newVolume = m_transferFunction->getVolume();
  if (newVolume != m_volume) {
    m_volume = newVolume;
    volumeChanged();
  }

  // update treshold widgets from tf
  m_domainMinSpinBox->setValue(m_transferFunction->get().getDomainMin());
  m_domainMaxSpinBox->setValue(m_transferFunction->get().getDomainMax());
}

void Z3DTransferFunctionEditor::fitDomainToData()
{
  if(m_volume) {
    m_transferFunction->get().setDomain(glm::dvec2(m_volume->getFloatMinValue(),
                                                   std::max(m_volume->getFloatMaxValue(), m_volume->getFloatMinValue()+0.001)),
                                        m_rescaleKeys->isChecked());
  }
}

void Z3DTransferFunctionEditor::reset()
{
  if (m_transferFunction)
    m_transferFunction->get().resetToDefault();
}

void Z3DTransferFunctionEditor::domainMinSpinBoxChanged(double min)
{
  if (m_transferFunction->get().isValidDomainMin(min)) {
    m_transferFunction->get().setDomainMin(min, m_rescaleKeys->isChecked());
  } else {
    QMessageBox::critical(this, "invalid transfer function range start", "invalid transfer function range start");
    m_domainMinSpinBox->setValue(m_transferFunction->get().getDomainMin());
  }
}

void Z3DTransferFunctionEditor::domainMaxSpinBoxChanged(double max)
{
  if (m_transferFunction->get().isValidDomainMax(max)) {
    m_transferFunction->get().setDomainMax(max, m_rescaleKeys->isChecked());
  } else {
    QMessageBox::critical(this, "invalid transfer function range end", "invalid transfer function range end");
    m_domainMaxSpinBox->setValue(m_transferFunction->get().getDomainMax());
  }
}

void Z3DTransferFunctionEditor::volumeChanged()
{   
  if (m_volume) {
    m_dataMinValueLabel->setText(QString::number(m_volume->getFloatMinValue()));
    m_dataMaxValueLabel->setText(QString::number(m_volume->getFloatMaxValue()));

    // propagate new volume to transfuncMappingCanvas
    m_transferFunctionWidget->volumeChanged(m_volume);
  }
}
