#ifndef Z3DCANVASRENDERER_H
#define Z3DCANVASRENDERER_H

#include "z3drenderprocessor.h"
#include "znumericparameter.h"

#include <QString>

class Z3DTexture;
class Z3DCanvas;
class Z3DTextureCopyRenderer;

class Z3DCanvasRenderer : public Z3DRenderProcessor
{
  Q_OBJECT
public:
  Z3DCanvasRenderer();
  ~Z3DCanvasRenderer();

  virtual void invalidate(InvalidationState inv = InvalidAllResult);

  void setCanvas(Z3DCanvas* canvas);

  Z3DCanvas *getCanvas() const;

  const Z3DTexture* getImageColorTexture(Z3DEye eye) const;
  const Z3DTexture* getImageDepthTexture(Z3DEye eye) const;

  bool renderToImage(const QString &filename, Z3DScreenShotType sst);
  bool renderToImage(const QString &filename, int width, int height, Z3DScreenShotType sst);

  QString getRenderToImageError() const;

  template<typename T>
  glm::detail::tvec4<T,glm::highp>* readBGRAColorBuffer(Z3DEye eye);

  template<typename T>
  glm::detail::tvec3<T,glm::highp>* readRGBColorBuffer(Z3DEye eye);

protected slots:
  void onCanvasResized(int w, int h);

protected:
  virtual void process(Z3DEye eye);
  virtual void initialize();
  virtual void deinitialize();

  virtual bool isReady(Z3DEye) const;
  virtual bool isValid(Z3DEye eye) const;

  void renderInportToImage(const QString& filename, Z3DEye eye);

  Z3DTextureCopyRenderer *m_textureCopyRenderer;

  Z3DCanvas* m_canvas;
  Z3DRenderInputPort m_inport;
  Z3DRenderInputPort m_leftEyeInport;
  Z3DRenderInputPort m_rightEyeInport;
  bool m_renderToImage;
  QString m_renderToImageFilename;
  QString m_renderToImageError;
  Z3DScreenShotType m_renderToImageType;
};

template<typename T>
glm::detail::tvec4<T, glm::highp> *Z3DCanvasRenderer::readBGRAColorBuffer(Z3DEye eye)
{
  if (!getImageColorTexture(eye)) {
    //LERROR() << "no texture to read";
    //return NULL;
    throw Exception("no texture to read");
  }

  // determine OpenGL data type from template parameter
  GLenum dataType;
  if (typeid(T) == typeid(uint8_t))
    dataType = GL_UNSIGNED_INT_8_8_8_8_REV;
  else if (typeid(T) == typeid(uint16_t))
    dataType = GL_UNSIGNED_SHORT;
  else if (typeid(T) == typeid(float))
    dataType = GL_FLOAT;
  else {
    //LERROR() << "unsupported data type. Expected: uint8_t, uint16_t, float";
    //return NULL;
    throw Exception("unsupported data type. Expected: uint8_t, uint16_t, float");
  }

  GLubyte* pixels = 0;
  pixels = getImageColorTexture(eye)->downloadTextureToBuffer(GL_BGRA, dataType);
  CHECK_GL_ERROR;

  if (pixels)
    return reinterpret_cast<glm::detail::tvec4<T,glm::highp>*>(pixels);
  else {
    //LERROR() << "failed to download texture";
    //return NULL;
    throw Exception("failed to download texture");
  }
}

template<typename T>
glm::detail::tvec3<T,glm::highp>* Z3DCanvasRenderer::readRGBColorBuffer(Z3DEye eye)
{
  if (!getImageColorTexture(eye)) {
    //LERROR() << "no texture to read";
    //return NULL;
    throw Exception("no texture to read");
  }

  // determine OpenGL data type from template parameter
  GLenum dataType;
  if (typeid(T) == typeid(uint8_t))
    dataType = GL_UNSIGNED_BYTE;
  else if (typeid(T) == typeid(uint16_t))
    dataType = GL_UNSIGNED_SHORT;
  else if (typeid(T) == typeid(float))
    dataType = GL_FLOAT;
  else {
    //    LERROR() << "unsupported data type. Expected: uint8_t, uint16_t, float";
    //    return NULL;
    throw Exception("unsupported data type. Expected: uint8_t, uint16_t, float");
  }

  GLubyte* pixels = 0;
  pixels = getImageColorTexture(eye)->downloadTextureToBuffer(GL_RGB, dataType);
  CHECK_GL_ERROR;

  if (pixels)
    return reinterpret_cast<glm::detail::tvec3<T,glm::highp>*>(pixels);
  else {
    //    LERROR() << "failed to download texture";
    //    return NULL;
    throw Exception("failed to download texture");
  }
}

#endif // Z3DCANVASRENDERER_H
