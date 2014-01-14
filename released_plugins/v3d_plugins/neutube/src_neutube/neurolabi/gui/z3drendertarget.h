#ifndef Z3DRENDERTARGET_H
#define Z3DRENDERTARGET_H

#include <map>
#include <set>
#include "z3dgl.h"

class Z3DTexture;

class Z3DRenderTarget
{
public:
  // create one color and one depth attachment
  Z3DRenderTarget(GLint internalColorFormat = GL_RGBA16, GLint internalDepthFormat = GL_DEPTH_COMPONENT24,
                  glm::ivec2 size = glm::ivec2(128,128), bool multisample = false, int sample = 4);
  // empty fbo with no attachment
  Z3DRenderTarget(glm::ivec2 size);
  virtual ~Z3DRenderTarget();

  void createColorAttachment(GLint internalColorFormat = GL_RGBA16, GLenum attachment = GL_COLOR_ATTACHMENT0);
  void createDepthAttachment(GLint internalDepthFormat = GL_DEPTH_COMPONENT24);

  void bind();
  void release();
  bool isBound() const;

  //Returns the OpenGL framebuffer object handle for this framebuffer object (returned by the glGenFrameBuffers() function).
  // returned fbo should only be used for read if multisample is used
  GLuint handle() const;

  const Z3DTexture* getAttachment(GLenum attachment) const
  {
    std::map<GLenum, Z3DTexture*>::const_iterator it = m_attachments.find(attachment);
    if (it != m_attachments.end())
      return it->second;
    else
      return NULL;
  }

  Z3DTexture* getAttachment(GLenum attachment) { return m_attachments[attachment]; }

  //Get the color at position pos. This method will bind the RenderTarget!
  glm::vec4 getFloatColorAtPos(glm::ivec2 pos);
  glm::col4 getColorAtPos(glm::ivec2 pos);
  // returned buffer should be deleted by caller using delete[] buffer;
  glm::col4* downloadColorBuffer(GLenum attachment = GL_COLOR_ATTACHMENT0) const;

  glm::ivec2 getSize() const;
  void resize(glm::ivec2 newsize);

  void changeColorAttachmentFormat(GLint internalColorFormat, GLenum attachment = GL_COLOR_ATTACHMENT0);
  void changeDepthAttachmentFormat(GLint internalDepthFormat);

  bool isFBOComplete();
  void attachTextureToFBO(Z3DTexture* texture, GLenum attachment, int mipLevel = 0, int zSlice = 0,
                          bool takeOwnership = true);

  static GLuint getCurrentBoundDrawFBO();
  static GLuint getCurrentBoundReadFBO();

protected:
  void generateId();

  GLuint m_fboID, m_multisampleFBOID;
  GLuint m_colorBufferID, m_depthBufferID;

  GLuint m_previousDrawFBOID;
  GLuint m_previousReadFBOID;

  std::map<GLenum, Z3DTexture*> m_attachments;
  // textures created by this rendertarget
  std::set<Z3DTexture*> m_ownTextures;

  bool m_multisample;
  int m_samples;
  int m_maxSamples;

  glm::ivec2 m_size;
};

#endif // Z3DRENDERTARGET_H
