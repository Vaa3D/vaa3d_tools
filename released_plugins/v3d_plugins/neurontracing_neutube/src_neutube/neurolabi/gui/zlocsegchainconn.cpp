/*
 * zlocsegchainconn.cpp
 *
 *  Created on: Aug 22, 2009
 *      Author: zhaot
 */

#include <iostream>
#include <QtGui>
#include <QXmlStreamWriter>
#include "zlocsegchainconn.h"
#include "tz_math.h"
#include "tz_voxel_graphics.h"
#include "zlocsegchain.h"

ZLocsegChainConn::ZLocsegChainConn()
{
  m_hook = -1;
  m_loop = -1;
  m_hookChain = NULL;
  m_loopChain = NULL;
  m_mode = NEUROCOMP_CONN_HL;
}

ZLocsegChainConn::ZLocsegChainConn(int hook, int link,
                                   int hookSpot, int loopSpot, int mode)
{
  m_hook = hook;
  m_loop = link;
  m_hookSpot = hookSpot;
  m_loopSpot = loopSpot;
  m_mode = mode;
}

ZLocsegChainConn::ZLocsegChainConn(ZLocsegChain *hook, ZLocsegChain *loop,
                                   int hookSpot, int loopSpot, int mode)
{
  m_hook = hook->id();
  m_loop = loop->id();
  m_hookChain = hook;
  m_loopChain = loop;
  m_hookSpot = hookSpot;
  m_loopSpot = loopSpot;
  m_mode = mode;
}

ZLocsegChainConn::~ZLocsegChainConn()
{
}

void ZLocsegChainConn::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZLocsegChainConn::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZLocsegChainConn::display(QPainter &painter, int z, Display_Style option) const
{
  UNUSED_PARAMETER(option);

  if ((m_hookChain != NULL) && (m_loopChain != NULL)) {
    Local_Neuroseg *locseg1;
    Local_Neuroseg *locseg2;
    if (m_hookSpot == 0) {
      locseg1 = m_hookChain->headNeuroseg();
    } else {
      locseg1 = m_hookChain->tailNeuroseg();
    }

    if (m_mode == NEUROCOMP_CONN_LINK) {
      if (m_loopSpot == 0) {
        locseg2 = m_loopChain->headNeuroseg();
      } else {
        locseg2 = m_loopChain->tailNeuroseg();
      }
    } else {
#ifdef _DEBUG_2
      print();
#endif
      locseg2 = m_loopChain->neurosegAt(m_loopSpot);
    }

    double center1[3], center2[3];
    Local_Neuroseg_Center(locseg1, center1);
    Local_Neuroseg_Center(locseg2, center2);

    for (int i = 0; i < 3; i++) {
      center1[i] = round(center1[i]);
      center2[i] = round(center2[i]);
    }

    QPointF center;

    painter.setPen(QPen(QColor(255, 255, 255, 255), .7));

    if ((center1[2] == z) || (z < 0)) {
      center.setX(center1[0]);
      center.setY(center1[1]);
      painter.drawEllipse(center, 3, 3);
    }

    if ((center2[2] == z) || (z < 0)){
      center.setX(center2[0]);
      center.setY(center2[1]);
      painter.drawEllipse(center, 3, 3);
    }

    voxel_t start, end;
    for (int i = 0; i < 3; i++) {
      start[i] = iround(center1[i]);
      end[i] = iround(center2[i]);
    }

    if (start[2] == z || z < 0 || end[2] == z || z < 0 ||
        (start[2] < z && end[2] > z) ||
        (start[2] > z && end[2] < z)) {
      QColor lineColor(255, 255, 255);
      painter.setPen(QPen(lineColor, .7));


      painter.drawLine(QPointF(center1[0], center1[1]),
          QPointF(center2[0], center2[1]));
    }

    /*
    Object_3d *obj = Line_To_Object_3d(start, end);

    for (size_t i = 0; i < obj->size; i++) {
      if (((obj->voxels[i][2] == z) || (z < 0))&&
          IS_IN_CLOSE_RANGE(obj->voxels[i][0], 0, widget->width() - 1) &&
          IS_IN_CLOSE_RANGE(obj->voxels[i][1], 0, widget->height() - 1)){
        uchar *pixel = widget->scanLine(obj->voxels[i][1])
                       + 4 * obj->voxels[i][0];
        if (i % 2 == 0) {
          pixel[0] = 255;
          pixel[1] = 255;
          pixel[2] = 255;
          pixel[3] = 255;
        } else {
          pixel[0] = 0;
          pixel[1] = 0;
          pixel[2] = 0;
          pixel[3] = 255;
        }
      }
    }

    Kill_Object_3d(obj);
    */
  }
}

bool ZLocsegChainConn::has(const ZLocsegChain *hook,
                           const ZLocsegChain *loop) const
{
  return (m_hookChain == hook) && (m_loopChain == loop);
}

bool ZLocsegChainConn::isHook(const ZLocsegChain *chain) const
{
  return (chain == m_hookChain);
}

bool ZLocsegChainConn::isLoop(const ZLocsegChain *chain) const
{
  return (chain == m_loopChain);
}

void ZLocsegChainConn::writeXml(QXmlStreamWriter &xml)
{
  xml.writeStartElement("connection");

  xml.writeStartElement("hook");
  xml.writeTextElement("filePath", m_hookChain->source());
  xml.writeTextElement("spot", QString("%1").arg(m_hookSpot));
  xml.writeEndElement();

  xml.writeStartElement("loop");
  xml.writeTextElement("filePath", m_loopChain->source());
  xml.writeTextElement("spot", QString("%1").arg(m_loopSpot));
  xml.writeEndElement();

  if (m_mode == NEUROCOMP_CONN_LINK) {
    xml.writeTextElement("mode", QString("%1").arg(NEUROCOMP_CONN_LINK));
  }

  xml.writeEndElement();
}

void ZLocsegChainConn::print()
{
  std::cout << "conn" << std::endl;
  std::cout << m_mode << std::endl;
  std::cout << m_hookChain->source().toLocal8Bit().constData() << std::endl;
  std::cout << m_hookSpot << std::endl;
  std::cout << m_loopChain->source().toLocal8Bit().constData() << std::endl;
  std::cout << m_loopSpot << std::endl;
}

void ZLocsegChainConn::translateMode()
{
  if (m_mode != NEUROCOMP_CONN_LINK) {
    if (m_loopChain != NULL) {
      if (m_loopSpot == 0) {
        m_mode = NEUROCOMP_CONN_LINK;
      } else if (m_loopSpot == m_loopChain->length() - 1) {
        m_loopSpot = 1;
        m_mode = NEUROCOMP_CONN_LINK;
      }
    }
  }
}

ZINTERFACE_DEFINE_CLASS_NAME(ZLocsegChainConn)
