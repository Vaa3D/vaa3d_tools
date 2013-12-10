#ifndef Z3DTEXTUREBLENDRENDERER_H
#define Z3DTEXTUREBLENDRENDERER_H

#include "z3dprimitiverenderer.h"

class Z3DTexture;

class Z3DTextureBlendRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  // supported modes:
  // "DepthTest", "FirstOnTop", "SecondOnTop"
  // "DepthTestBlending", "FirstOnTopBlending", "SecondOnTopBlending"
  explicit Z3DTextureBlendRenderer(const QString &mode = "DepthTestBlending", QObject *parent = 0);

  void setColorTexture1(const Z3DTexture *colorTex) { m_colorTexture1 = colorTex; }
  void setDepthTexture1(const Z3DTexture *depthTex) { m_depthTexture1 = depthTex; }
  void setColorTexture2(const Z3DTexture *colorTex) { m_colorTexture2 = colorTex; }
  void setDepthTexture2(const Z3DTexture *depthTex) { m_depthTexture2 = depthTex; }

  void setBlendMode(const QString &mode) { m_blendMode.select(mode); }

signals:

public slots:

protected:
  virtual void compile();
  virtual void initialize();
  virtual void deinitialize();
  virtual QString generateHeader();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void renderUsingGLSL(Z3DEye eye);
  virtual void renderPickingUsingGLSL(Z3DEye);

  const Z3DTexture *m_colorTexture1;
  const Z3DTexture *m_depthTexture1;
  const Z3DTexture *m_colorTexture2;
  const Z3DTexture *m_depthTexture2;

  Z3DShaderProgram m_blendTextureShader;

  ZOptionParameter<QString, QString> m_blendMode;
};

#endif // Z3DTEXTUREBLENDRENDERER_H
