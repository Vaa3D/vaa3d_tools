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

#ifndef Z3DPICKINGMANAGER_H
#define Z3DPICKINGMANAGER_H

#include "zglmutils.h"
#include <stdint.h>
#include <map>

class Z3DRenderTarget;

struct colComp
{
  // only work in little-endian system
  bool operator()(const glm::col4 &c1, const glm::col4 &c2) const
  {
    const uint32_t i1 = reinterpret_cast<const uint32_t&>(c1);
    const uint32_t i2 = reinterpret_cast<const uint32_t&>(c2);
    return i1 < i2;
  }
};

class Z3DPickingManager
{
public:
  Z3DPickingManager();

  glm::col4 registerObject(const void* obj);
  void deregisterObject(const void* obj);
  void deregisterObject(const glm::col4& col);
  void clearRegisteredObjects();

  glm::col4 getColorFromObject(const void* obj);
  const void* getObjectFromColor(glm::col4 col);

  const void* getObjectAtWidgetPos(glm::ivec2 pos);
  const void* getObjectAtPos(glm::ivec2 pos);

  // find all objects within a radius of pos, sort by distance
  // if radius is -1, search the whole image
  std::vector<const void*> sortObjectsByDistanceToPos(glm::ivec2 pos, int radius = -1, bool ascend = true);

  bool isHit(glm::ivec2 pos, const void* obj) { return (getObjectAtPos(pos) == obj); }

  void bindTarget();
  void releaseTarget();
  void clearTarget();

  // input render target should has color internal format as GL_RGBA8
  void setRenderTarget(Z3DRenderTarget* rt);
  Z3DRenderTarget* getRenderTarget() const { return m_renderTarget; }
  // check this before use pickingManager since setRenderTarget might fail
  bool hasRenderTarget() const { return m_renderTarget != NULL; }

  bool isRegistered(const void* obj) { return m_objectToColor.find(obj) != m_objectToColor.end(); }
  bool isRegistered(glm::col4 col) { return m_colorToObject.find(col) != m_colorToObject.end(); }
private:
  void increaseColor();

  std::map<glm::col4, const void*, colComp> m_colorToObject;
  std::map<const void*, glm::col4> m_objectToColor;
  Z3DRenderTarget* m_renderTarget;
  glm::col4 m_currentColor;
};

#endif // Z3DPICKINGMANAGER_H
