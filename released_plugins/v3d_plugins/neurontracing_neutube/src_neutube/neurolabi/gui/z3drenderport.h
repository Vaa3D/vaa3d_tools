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

#ifndef Z3DRENDERPORT_H
#define Z3DRENDERPORT_H

#include "z3dport.h"
#include "z3dtexture.h"
#include <typeinfo>

class Z3DRenderTarget;

class Z3DRenderInputPort;

class Z3DRenderOutputPort : public Z3DOutputPortBase
{
  friend class Z3DRenderProcessor;

public:
  Z3DRenderOutputPort(const QString& name, bool allowMultipleConnections = true,
                      Z3DProcessor::InvalidationState invalidationState = Z3DProcessor::InvalidAllResult,
                      GLint internalColorFormat=GL_RGBA16, GLint internalDepthFormat=GL_DEPTH_COMPONENT24);
  virtual ~Z3DRenderOutputPort();

  virtual void invalidate();

  void bindTarget();
  void releaseTarget();

  GLint getInternalDepthFormat() const { return m_internalDepthFormat; }
  GLint getInternalColorFormat() const { return m_internalColorFormat; }

  // Clears the contents of an activated outport's RenderTarget,
  void clearTarget();

  virtual bool isReady() const { return isConnected() && m_renderTarget; }

  virtual bool hasValidData() const { return m_renderTarget && m_resultIsValid; }

  // returns the dimensions of the associated RenderTarget or zero if no rendertarget
  glm::ivec2 getSize() const;

  // return the maximum of expectesize of all connected inports.
  // If no inport connected, return (-1, -1)
  glm::ivec2 getExpectedSize() const;

  // Returns true, if the associated RenderTarget is currently bound.
  bool isBound() const;

  // Returns the port's RenderTarget, may be null.
  Z3DRenderTarget* getRenderTarget() const { return m_renderTarget; }

  Z3DTexture* getColorTexture();
  Z3DTexture* getDepthTexture();

  bool hasRenderTarget() const { return m_renderTarget; }

  // Resizes the associated RenderTarget to the passed dimensions.
  void resize(const glm::ivec2& newsize);
  void resize(int x, int y) { resize(glm::ivec2(x,y)); }

  // change RenderTarget with the given format.
  void changeColorFormat(GLint internalColorFormat);
  void chagneDepthFormat(GLint internalDepthFormat);

  virtual bool canConnectTo(const Z3DInputPortBase* inport) const;

  //void setMultisample(bool multisample, int nsample = 4);

protected:

  virtual void setRenderTarget(Z3DRenderTarget* renderTarget);

  virtual void initialize();

  virtual void deinitialize();

  virtual void setProcessor(Z3DProcessor *p);

private:
  Z3DRenderTarget* m_renderTarget;

  bool m_resultIsValid;
  glm::ivec2 m_size;

  GLint m_internalColorFormat;
  GLint m_internalDepthFormat;

  bool m_multisample;
  int m_sample;
};

class Z3DRenderInputPort : public Z3DInputPortBase
{
  friend class Z3DRenderProcessor;

public:
  Z3DRenderInputPort(const QString &name, bool allowMultipleConnections = false,
                     Z3DProcessor::InvalidationState invalidationState = Z3DProcessor::InvalidAllResult);
  virtual ~Z3DRenderInputPort();

  virtual bool isReady() const { return getNumOfValidInputs() > 0; }

  void setExpectedSize(glm::ivec2 size) { m_expectedSize = size; }
  glm::ivec2 getExpectedSize() const { return m_expectedSize; }

  // go through all connected output render ports and count how many have valid rendering
  int getNumOfValidInputs() const;

  // once we have the number of valid inputs, we can use a index as parameter to query data from input
  // idx range from 0 to getNumOfValidInputs() - 1
  glm::ivec2 getSize(int idx = 0) const;
  const Z3DTexture* getColorTexture(int idx = 0) const;
  const Z3DTexture* getDepthTexture(int idx = 0) const;

protected:
  virtual void setProcessor(Z3DProcessor *p);

private:
  const Z3DRenderTarget* getRenderTarget(int idx) const;

  glm::ivec2 m_expectedSize;
};

#endif // Z3DRENDERPORT_H
