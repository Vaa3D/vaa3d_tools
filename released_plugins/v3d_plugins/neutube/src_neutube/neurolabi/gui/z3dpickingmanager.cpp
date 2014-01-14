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

#include "z3dpickingmanager.h"

#include "z3dtexture.h"
#include "QsLog.h"
#include "z3drendertarget.h"
#include <QApplication>

Z3DPickingManager::Z3DPickingManager()
  : m_renderTarget(NULL)
  , m_currentColor(0,0,0,128)
{
}

glm::col4 Z3DPickingManager::registerObject(const void* obj)
{
  increaseColor();
  m_colorToObject[m_currentColor] = obj;
  m_objectToColor[obj] = m_currentColor;
  return m_currentColor;
}

void Z3DPickingManager::deregisterObject(const void* obj)
{
  glm::col4 col = getColorFromObject(obj);
  m_colorToObject.erase(col);
  m_objectToColor.erase(obj);
}

void Z3DPickingManager::deregisterObject(const glm::col4& col)
{
  const void* obj = getObjectFromColor(col);
  m_colorToObject.erase(col);
  m_objectToColor.erase(obj);
}

void Z3DPickingManager::clearRegisteredObjects()
{
  m_colorToObject.clear();
  m_objectToColor.clear();
  m_currentColor = glm::col4(0,0,0,128);
}

glm::col4 Z3DPickingManager::getColorFromObject(const void* obj)
{
  if (!obj)
    return glm::col4(0,0,0,0);

  if (isRegistered(obj)) {
    return m_objectToColor[obj];
  } else
    return glm::col4(0,0,0,0);
}

const void* Z3DPickingManager::getObjectFromColor(glm::col4 col)
{
  if (col.a == 0)
    return NULL;

  if (isRegistered(col))
    return m_colorToObject[col];
  else
    return NULL;
}

const void* Z3DPickingManager::getObjectAtWidgetPos(glm::ivec2 pos)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  pos[0] = pos[0] * qApp->devicePixelRatio();
  pos[1] = pos[1] * qApp->devicePixelRatio();
#endif
  glm::ivec3 texSize =
      getRenderTarget()->getAttachment(GL_COLOR_ATTACHMENT0)->getDimensions();
  pos[1] = texSize[1]- pos[1];
  return getObjectAtPos(pos);
}

const void* Z3DPickingManager::getObjectAtPos(glm::ivec2 pos)
{
  return getObjectFromColor(m_renderTarget->getColorAtPos(pos));
}

std::vector<const void *> Z3DPickingManager::sortObjectsByDistanceToPos(glm::ivec2 pos, int radius, bool ascend)
{
  std::map<glm::col4, int, colComp> col2dist;
  glm::col4* buf = m_renderTarget->downloadColorBuffer();
  glm::ivec2 texSize = m_renderTarget->getSize();
  if (radius < 0)
    radius = std::max(texSize.x, texSize.y);
  for(int y = std::max(0, pos.y-radius); y <= std::min(texSize.y-1, pos.y+radius); ++y)
    for(int x = std::max(0, pos.x-radius); x <= std::min(texSize.x-1, pos.x+radius); ++x) {
      glm::col4 col = buf[(y*texSize.x) + x];
      if (col2dist[col] == 0)
        col2dist[col] = (x-pos.x)*(x-pos.x) + (y-pos.y)*(y-pos.y);
      else
        col2dist[col] = std::min(col2dist[col], (x-pos.x)*(x-pos.x) + (y-pos.y)*(y-pos.y));
    }
  delete[] buf;
  std::vector<const void *> res;
  if (ascend) {
    std::multimap<int, const void *> dist2obj;
    for (std::map<glm::col4, int>::const_iterator it = col2dist.begin();
         it != col2dist.end(); ++it) {
      const void *obj = getObjectFromColor(it->first);
      if (obj)
        dist2obj.insert(std::pair<int,const void*>(it->second,obj));
    }
    for (std::multimap<int, const void *>::const_iterator it = dist2obj.begin();
         it != dist2obj.end(); ++it) {
      res.push_back(it->second);
    }
  } else {
    std::multimap<int, const void *, std::greater<int> > dist2obj;
    for (std::map<glm::col4, int>::const_iterator it = col2dist.begin();
         it != col2dist.end(); ++it) {
      const void *obj = getObjectFromColor(it->first);
      if (obj)
        dist2obj.insert(std::pair<int,const void*>(it->second,obj));
    }
    for (std::multimap<int, const void *>::const_iterator it = dist2obj.begin();
         it != dist2obj.end(); ++it) {
      res.push_back(it->second);
    }
  }
  return res;
}

void Z3DPickingManager::bindTarget()
{
  if (m_renderTarget) {
    m_renderTarget->bind();
  }
  else
    LERROR() << "No RenderTarget!";
}

void Z3DPickingManager::releaseTarget()
{
  if (m_renderTarget)
    m_renderTarget->release();
  else
    LERROR() << "No RenderTarget!";
}

void Z3DPickingManager::clearTarget()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Z3DPickingManager::setRenderTarget(Z3DRenderTarget *rt)
{
  if (rt->getAttachment(GL_COLOR_ATTACHMENT0)->getInternalFormat() != GL_RGBA8) {
    LERROR() << "Render target format should be GL_RGBA8! Picking will fail.";
    return;
  }
  m_renderTarget = rt;
}

void Z3DPickingManager::increaseColor()
{
  if (*reinterpret_cast<uint32_t*>(&m_currentColor[0]) != 0xffffffff)
    ++(*reinterpret_cast<uint32_t*>(&m_currentColor[0]));
  else {
    m_currentColor = glm::col4(0,0,0,128);
    //LERROR() << "Out of colors...";
  }
}
