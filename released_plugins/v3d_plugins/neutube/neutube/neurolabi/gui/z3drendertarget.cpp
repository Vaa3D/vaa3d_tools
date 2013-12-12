#include "z3drendertarget.h"

#include "QsLog.h"
#include "zbenchtimer.h"

Z3DRenderTarget::Z3DRenderTarget(GLint internalColorFormat, GLint internalDepthFormat, glm::ivec2 size,
                                 bool multisample, int sample)
  : m_previousDrawFBOID(0)
  , m_previousReadFBOID(0)
  , m_multisample(multisample)
  , m_samples(sample)
  , m_maxSamples(0)
  , m_size(size)
{
  generateId();

  createColorAttachment(internalColorFormat);
  createDepthAttachment(internalDepthFormat);
  isFBOComplete();
}

Z3DRenderTarget::Z3DRenderTarget(glm::ivec2 size)
  : m_previousDrawFBOID(0)
  , m_previousReadFBOID(0)
  , m_multisample(false)
  , m_samples(4)
  , m_maxSamples(0)
  , m_size(size)
{
  generateId();
}

Z3DRenderTarget::~Z3DRenderTarget()
{
  //  if (m_multisample) {
  //    glDeleteRenderbuffers(1, &m_colorBufferID);
  //    glDeleteRenderbuffers(1, &m_depthBufferID);
  //    glDeleteFramebuffers(1, &m_multisampleFBOID);
  //  }
  glDeleteFramebuffers(1, &m_fboID);

  for (std::set<Z3DTexture*>::iterator it = m_ownTextures.begin();
       it != m_ownTextures.end(); ++it) {
    delete (*it);
  }
}

void Z3DRenderTarget::createColorAttachment(GLint internalColorFormat, GLenum attachment)
{
  Z3DTexture *colorTex;

  glm::ivec3 size3(m_size, 1);

  switch(internalColorFormat) {
  case GL_RGB:
    colorTex = new Z3DTexture(size3, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
    break;
  case GL_RGBA8:
    colorTex = new Z3DTexture(size3, GL_BGRA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV);
    break;
  case GL_RGBA16:
    colorTex = new Z3DTexture(size3, GL_BGRA, GL_RGBA16, GL_UNSIGNED_SHORT);
    break;
  case GL_RGB16F_ARB:
    colorTex = new Z3DTexture(size3, GL_RGB, GL_RGB16F_ARB, GL_FLOAT);
    break;
  case GL_RGBA16F_ARB:
    colorTex = new Z3DTexture(size3, GL_BGRA, GL_RGBA16F_ARB, GL_FLOAT);
    break;
  case GL_RGBA32F_ARB:
    colorTex = new Z3DTexture(size3, GL_BGRA, GL_RGBA32F_ARB, GL_FLOAT);
    break;
  default:
    LERROR() << "unknown internal format!";
    return;
  }
  colorTex->uploadTexture();

  attachTextureToFBO(colorTex, attachment);

  //  if (m_multisample) {
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_colorBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_colorTex->getInternalFormat(),
  //                                     m_colorTex->getWidth(), m_colorTex->getHeight());
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_depthTex->getInternalFormat(),
  //                                     m_depthTex->getWidth(), m_depthTex->getHeight());
  //    //It's time to attach the RBs to the FBO
  //    glBindFramebuffer(GL_FRAMEBUFFER, m_multisampleFBOID);
  //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorBufferID);
  //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferID);
  //  }
}

void Z3DRenderTarget::createDepthAttachment(GLint internalDepthFormat)
{
  Z3DTexture *depthTex;

  glm::ivec3 size3(m_size, 1);

  switch(internalDepthFormat) {
  case GL_DEPTH_COMPONENT:
    depthTex = new Z3DTexture(size3, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
    break;
  case GL_DEPTH_COMPONENT16:
    depthTex = new Z3DTexture(size3, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_FLOAT);
    break;
  case GL_DEPTH_COMPONENT24:
    depthTex = new Z3DTexture(size3, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_FLOAT);
    break;
  case GL_DEPTH_COMPONENT32:
    depthTex = new Z3DTexture(size3, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32, GL_FLOAT);
    break;
  case GL_DEPTH_COMPONENT32F:
    depthTex = new Z3DTexture(size3, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT);
    break;
  default:
    depthTex = new Z3DTexture(size3, GL_DEPTH_COMPONENT, internalDepthFormat, GL_FLOAT);
    LERROR() << "unknown internal depth format!";
  }
  depthTex->uploadTexture();

  attachTextureToFBO(depthTex, GL_DEPTH_ATTACHMENT);

  //  if (m_multisample) {
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_colorBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_colorTex->getInternalFormat(),
  //                                     m_colorTex->getWidth(), m_colorTex->getHeight());
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_depthTex->getInternalFormat(),
  //                                     m_depthTex->getWidth(), m_depthTex->getHeight());
  //    //It's time to attach the RBs to the FBO
  //    glBindFramebuffer(GL_FRAMEBUFFER, m_multisampleFBOID);
  //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorBufferID);
  //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferID);
  //  }
}

void Z3DRenderTarget::bind()
{
  glViewport(0, 0, m_size.x, m_size.y);
  if (isBound())
    return;
  m_previousDrawFBOID = getCurrentBoundDrawFBO();
  m_previousReadFBOID = getCurrentBoundReadFBO();
  if (!m_multisample)
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  //else
    //glBindFramebuffer(GL_FRAMEBUFFER, m_multisampleFBOID);
}

void Z3DRenderTarget::release()
{
  //  if (m_multisample) {
  //    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_multisampleFBOID);
  //    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
  //    glBlitFramebuffer(0, 0, m_colorTex->getWidth(), m_colorTex->getHeight(), 0, 0,
  //                      m_colorTex->getWidth(), m_colorTex->getHeight(), GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT,
  //                      GL_LINEAR);
  //    CHECK_GL_ERROR;
  //  }
  //LINFO() << m_previousDrawFBOID << m_previousReadFBOID;
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_previousReadFBOID);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_previousDrawFBOID);
}

bool Z3DRenderTarget::isBound() const
{
  return getCurrentBoundDrawFBO() == m_fboID;
}

GLuint Z3DRenderTarget::handle() const
{
  return m_fboID;
}

glm::vec4 Z3DRenderTarget::getFloatColorAtPos(glm::ivec2 pos)
{
  bind();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glm::vec4 pixel;
  glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, GL_FLOAT, &pixel[0]);
  release();
  return pixel;
}

glm::col4 Z3DRenderTarget::getColorAtPos(glm::ivec2 pos)
{
  bind();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glm::col4 pixel;
  glReadPixels(pos.x, pos.y, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &pixel[0]);
  std::swap(pixel.r, pixel.b);
  release();
  return pixel;
}

glm::col4 *Z3DRenderTarget::downloadColorBuffer(GLenum attachment) const
{
  const Z3DTexture *tex = getAttachment(attachment);
  //GLubyte* buf = tex->downloadTextureToBuffer(GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV);
  // even with swap, BGRA is faster than RGBA
  GLubyte* buf = tex->downloadTextureToBuffer(GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV);
  for (int i=0; i<getSize().x * getSize().y * 4; i+=4)
    std::swap(buf[i], buf[i+2]);
  return reinterpret_cast<glm::col4*>(buf);
}

glm::ivec2 Z3DRenderTarget::getSize() const
{
  return m_size;
}

void Z3DRenderTarget::resize(glm::ivec2 newsize)
{
  if (newsize == m_size)
    return;

  m_size = newsize;

  glActiveTexture(GL_TEXTURE0);
  for (std::map<GLenum, Z3DTexture*>::iterator it = m_attachments.begin();
       it != m_attachments.end(); ++it) {
    if (it->second) {
      it->second->setDimensions(glm::ivec3(m_size.x, m_size.y, 1));
      it->second->uploadTexture();
    }
  }

  //  if (m_multisample) {
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_colorBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_colorTex->getInternalFormat(),
  //                                     m_colorTex->getWidth(), m_colorTex->getHeight());
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_depthTex->getInternalFormat(),
  //                                     m_depthTex->getWidth(), m_depthTex->getHeight());
  //  }
}

void Z3DRenderTarget::changeColorAttachmentFormat(GLint internalColorFormat, GLenum attachment)
{
  if (m_attachments.find(attachment) == m_attachments.end() ||
      m_attachments[attachment] == NULL ||
      m_attachments[attachment]->getInternalFormat() == internalColorFormat)
    return;

  m_attachments[attachment]->setInternalFormat(internalColorFormat);
  m_attachments[attachment]->uploadTexture();

  //  if (m_multisample) {
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_colorBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_colorTex->getInternalFormat(),
  //                                     m_colorTex->getWidth(), m_colorTex->getHeight());
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_depthTex->getInternalFormat(),
  //                                     m_depthTex->getWidth(), m_depthTex->getHeight());
  //  }
}

void Z3DRenderTarget::changeDepthAttachmentFormat(GLint internalDepthFormat)
{
  if (m_attachments.find(GL_DEPTH_ATTACHMENT) == m_attachments.end() ||
      m_attachments[GL_DEPTH_ATTACHMENT] == NULL ||
      m_attachments[GL_DEPTH_ATTACHMENT]->getInternalFormat() == internalDepthFormat)
    return;

  m_attachments[GL_DEPTH_ATTACHMENT]->setInternalFormat(internalDepthFormat);
  m_attachments[GL_DEPTH_ATTACHMENT]->uploadTexture();
}

bool Z3DRenderTarget::isFBOComplete()
{
  bool complete = false;

  bind();

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  switch(status) {
  case GL_FRAMEBUFFER_COMPLETE:
    complete = true;
    break;
  case GL_FRAMEBUFFER_UNDEFINED:
    LERROR() << "GL_FRAMEBUFFER_UNDEFINED: target is the default framebuffer, but the default framebuffer does not exist.";
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: some of the framebuffer attachment points are framebuffer incomplete.";
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: framebuffer does not have at least one image attached to it.";
    break;
#if defined(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) || defined(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT)
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
#else
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
#endif
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE "
                "is GL_NONE for some color attachment point(s) named by GL_DRAWBUFFERi.";
    break;
#endif
#if defined(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) || defined(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT)
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
#else
  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
#endif
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: GL_READ_BUFFER is not GL_NONE "
                "and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named "
                "by GL_READ_BUFFER.";
    break;
#endif
  case GL_FRAMEBUFFER_UNSUPPORTED:
    LERROR() << "GL_FRAMEBUFFER_UNSUPPORTED: the combination of internal formats of the attached images violates "
                "an implementation-dependent set of restrictions";
    break;
#if defined(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) || defined(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT)
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
#else
  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
#endif
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: the value of GL_RENDERBUFFER_SAMPLES is not the same "
                "for all attached renderbuffers; the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; "
                "or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES "
                "does not match the value of GL_TEXTURE_SAMPLES; or, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is "
                "not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, "
                "the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.";
    break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
  case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: some framebuffer attachment is layered, and some populated "
                "attachment is not layered, or all populated color attachments are not from textures of the same target.";
    break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT
  case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT:
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT: duplicate attachment.";
    break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
  case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: attached images must have same dimensions.";
    break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_FORMATS
  case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_FORMATS: attached images must have same format.";
    break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
  case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: attached images must have same dimensions.";
    break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT
  case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
    LERROR() << "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: attached images must have same format.";
    break;
#endif
  default:
    LERROR() << "Unknown error!";
  }

  release();
  return complete;
}

void Z3DRenderTarget::attachTextureToFBO(Z3DTexture *texture, GLenum attachment, int mipLevel, int zSlice,
                                         bool takeOwnership)
{
  if (m_size != glm::ivec2(texture->getDimensions())) {
    LWARN() << "attached texture has imcompatible size with current fbo";
  }
  bind();
  switch(texture->getTextureTarget()) {
  case GL_TEXTURE_1D:
    glFramebufferTexture1D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_1D, texture->getId(), mipLevel);
    break;
  case GL_TEXTURE_3D:
    glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_3D, texture->getId(), mipLevel, zSlice);
    break;
  case GL_TEXTURE_2D_ARRAY:
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, texture->getId(), mipLevel, zSlice);
    break;
  default: //GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, ...
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture->getTextureTarget(), texture->getId(), mipLevel);
    break;
  }
  //LINFO() << texture->getId() << texture->getTextureTarget() << GL_TEXTURE_RECTANGLE << texture->getDimensions();
  CHECK_GL_ERROR;
  release();
  m_attachments[attachment] = texture;
  if (takeOwnership)
    m_ownTextures.insert(texture);
}

GLuint Z3DRenderTarget::getCurrentBoundDrawFBO()
{
  GLint fbo;
  //glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);
  return static_cast<GLuint>(fbo);
}

GLuint Z3DRenderTarget::getCurrentBoundReadFBO()
{
  GLint fbo;
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &fbo);
  return static_cast<GLuint>(fbo);
}

void Z3DRenderTarget::generateId()
{
  m_fboID = 0;
  m_multisampleFBOID = 0;
  m_colorBufferID = 0;
  m_depthBufferID = 0;
  glGenFramebuffers(1, &m_fboID);
  //  if (m_samples < 2)
  //    m_multisample = false;
  //  if (m_multisample) {
  //    glGetIntegerv(GL_MAX_SAMPLES, &m_maxSamples);
  //    if (m_maxSamples > 1) {
  //      glGenFramebuffers(1, &m_multisampleFBOID);
  //      glGenRenderbuffers(1, &m_colorBufferID);
  //      glGenRenderbuffers(1, &m_depthBufferID);
  //    } else {
  //      LWARN() << "Multisample not supported?";
  //      m_multisample = false;
  //    }
  //  }
}
