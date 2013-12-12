#include "z3drenderport.h"
#include "z3drenderprocessor.h"
#include "z3drendertarget.h"

#include "z3dgpuinfo.h"

Z3DRenderOutputPort::Z3DRenderOutputPort(const QString &name, bool allowMultipleConnections,
                                         Z3DProcessor::InvalidationState invalidationState, GLint internalColorFormat, GLint internalDepthFormat)
  : Z3DOutputPortBase(name, allowMultipleConnections, invalidationState)
  , m_renderTarget(NULL)
  , m_resultIsValid(false)
  , m_size(128,128)
  , m_internalColorFormat(internalColorFormat)
  , m_internalDepthFormat(internalDepthFormat)
  , m_multisample(false)
  , m_sample(4)
{
}

Z3DRenderOutputPort::~Z3DRenderOutputPort()
{
  if (m_renderTarget) {
    LERROR() << getName() << "has not been deinitialized before destruction";
  }
}

void Z3DRenderOutputPort::invalidate()
{
  m_resultIsValid = false;
  Z3DOutputPortBase::invalidate();
}

void Z3DRenderOutputPort::bindTarget()
{
  if (m_renderTarget) {
    m_renderTarget->bind();
    m_resultIsValid = true;
  } else
    LERROR() << getName() << "has no RenderTarget to bind";
}

void Z3DRenderOutputPort::releaseTarget()
{
  if (m_renderTarget)
    m_renderTarget->release();
  else
    LERROR() << getName() << "has no RenderTarget to release";
}

void Z3DRenderOutputPort::clearTarget()
{
  if (!isBound())
    LERROR() << "RenderTarget is not bound, can not clear.";
  else
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

glm::ivec2 Z3DRenderOutputPort::getSize() const
{
  if (hasRenderTarget())
    return getRenderTarget()->getSize();
  else
    return glm::ivec2(0);
}

glm::ivec2 Z3DRenderOutputPort::getExpectedSize() const
{
  glm::ivec2 result(-1, -1);
  const std::vector<Z3DInputPortBase*> inports = getConnected();
  for (size_t j=0; j<inports.size(); ++j) {
    Z3DRenderInputPort *renderInport = dynamic_cast< Z3DRenderInputPort* >(inports[j]);
    if (renderInport)
      result = glm::max(result, renderInport->getExpectedSize());
    else
      LERROR() << "This should not happen..";
  }
  return result;
}

bool Z3DRenderOutputPort::isBound() const
{
  return m_renderTarget && m_renderTarget->isBound();
}

Z3DTexture *Z3DRenderOutputPort::getColorTexture()
{
  if (hasRenderTarget())
    return getRenderTarget()->getAttachment(GL_COLOR_ATTACHMENT0);
  else
    return NULL;
}

Z3DTexture *Z3DRenderOutputPort::getDepthTexture()
{
  if (hasRenderTarget())
    return getRenderTarget()->getAttachment(GL_DEPTH_ATTACHMENT);
  else
    return NULL;
}

void Z3DRenderOutputPort::resize(const glm::ivec2 &newsize)
{
  if (m_size == newsize)
    return;
  if (newsize == glm::ivec2(0)) {
    LWARN() << "invalid size:" << newsize;
    return;
  }
  if (newsize.x > Z3DGpuInfoInstance.getMaxTextureSize() ||
      newsize.y > Z3DGpuInfoInstance.getMaxTextureSize()) {
    LWARN() << "size" << newsize << "exceeds texture size limit:"
            << Z3DGpuInfoInstance.getMaxTextureSize();
    return;
  }
  if (m_renderTarget) {
    m_renderTarget->resize(newsize);
  }
  m_resultIsValid = false;
  m_size = newsize;
}

void Z3DRenderOutputPort::changeColorFormat(GLint internalColorFormat)
{
  m_internalColorFormat = internalColorFormat;
  m_renderTarget->changeColorAttachmentFormat(m_internalColorFormat);
  invalidate();
}

void Z3DRenderOutputPort::chagneDepthFormat(GLint internalDepthFormat)
{
  m_internalDepthFormat = internalDepthFormat;
  m_renderTarget->changeDepthAttachmentFormat(m_internalDepthFormat);
  invalidate();
}

bool Z3DRenderOutputPort::canConnectTo(const Z3DInputPortBase *inport) const
{
  if (dynamic_cast<const Z3DRenderInputPort*>(inport))
    return Z3DOutputPortBase::canConnectTo(inport);
  else
    return false;
}

//void Z3DRenderOutputPort::setMultisample(bool multisample, int nsample)
//{
//  if (!isInitialized() || (multisample == m_multisample && nsample == m_sample))
//    return;
//  m_multisample = multisample;
//  m_sample = nsample;
//  changeFormat(m_internalColorFormat, m_internalDepthFormat);    // use same format, just replace rendertarget
//}

void Z3DRenderOutputPort::setRenderTarget(Z3DRenderTarget *renderTarget)
{
  m_renderTarget = renderTarget;
  invalidate();
}

void Z3DRenderOutputPort::initialize()
{
  Z3DOutputPortBase::initialize();
  m_renderTarget = new Z3DRenderTarget(m_internalColorFormat, m_internalDepthFormat,
                                       m_size, m_multisample, m_sample);

  assert(m_processor);
  m_resultIsValid = false;
  CHECK_GL_ERROR;
}

void Z3DRenderOutputPort::deinitialize()
{
  if (m_renderTarget) {
    delete m_renderTarget;
    m_renderTarget = NULL;
  }
  CHECK_GL_ERROR;

  Z3DOutputPortBase::deinitialize();
}

void Z3DRenderOutputPort::setProcessor(Z3DProcessor *p)
{
  Z3DOutputPortBase::setProcessor(p);

  Z3DRenderProcessor* rp = dynamic_cast<Z3DRenderProcessor*>(p);
  assert(rp);
  if (!rp) {
    LERROR() << "RenderPort" << getName() << "attached to processor of wrong type:"
             << p->getClassName();
  }
}

//-----------------------------------------------------------------------------------

Z3DRenderInputPort::Z3DRenderInputPort(const QString &name, bool allowMultipleConnections,
                                       Z3DProcessor::InvalidationState invalidationState)
  : Z3DInputPortBase(name, allowMultipleConnections, invalidationState)
  , m_expectedSize(0)
{
}

Z3DRenderInputPort::~Z3DRenderInputPort()
{
}

int Z3DRenderInputPort::getNumOfValidInputs() const
{
  int res = 0;
  for (size_t i=0; i<m_connectedOutputPorts.size(); ++i) {
    const Z3DRenderOutputPort* p = dynamic_cast<const Z3DRenderOutputPort*>(m_connectedOutputPorts[0]);
    assert(p);
    if (p->hasValidData())
      ++res;
  }
  return res;
}

glm::ivec2 Z3DRenderInputPort::getSize(int idx) const
{
  if (getRenderTarget(idx))
    return getRenderTarget(idx)->getSize();
  else
    return glm::ivec2(0);
}

const Z3DTexture *Z3DRenderInputPort::getColorTexture(int idx) const
{
  if (getRenderTarget(idx))
    return getRenderTarget(idx)->getAttachment(GL_COLOR_ATTACHMENT0);
  else
    return NULL;
}

const Z3DTexture *Z3DRenderInputPort::getDepthTexture(int idx) const
{
  if (getRenderTarget(idx))
    return getRenderTarget(idx)->getAttachment(GL_DEPTH_ATTACHMENT);
  else
    return NULL;
}

void Z3DRenderInputPort::setProcessor(Z3DProcessor *p)
{
  Z3DInputPortBase::setProcessor(p);

  Z3DRenderProcessor* rp = dynamic_cast<Z3DRenderProcessor*>(p);
  assert(rp);
  if (!rp) {
    LERROR() << "RenderPort" << getName() << "attached to processor of wrong type:"
             << p->getClassName();
  }
}

const Z3DRenderTarget *Z3DRenderInputPort::getRenderTarget(int idx) const
{
  if (idx < 0 || idx > getNumOfValidInputs() - 1)
    return NULL;
  int res = 0;
  for (size_t i=0; i<m_connectedOutputPorts.size(); ++i) {
    const Z3DRenderOutputPort* p = dynamic_cast<const Z3DRenderOutputPort*>(m_connectedOutputPorts[0]);
    assert(p);
    if (p->hasValidData())
      ++res;
    if (idx == res - 1)
      return p->getRenderTarget();
  }
  return NULL;
}


