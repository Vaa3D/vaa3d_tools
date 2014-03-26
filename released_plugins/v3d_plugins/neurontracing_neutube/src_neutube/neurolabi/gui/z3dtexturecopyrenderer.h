#ifndef Z3DTEXTURECOPYRENDERER_H
#define Z3DTEXTURECOPYRENDERER_H

#include "z3dprimitiverenderer.h"

class Z3DTexture;

class Z3DTextureCopyRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  enum OutputColorOption
  {
    None, Divide_By_Alpha, Multiply_By_Alpha
  };

  explicit Z3DTextureCopyRenderer(QObject *parent = 0);

  void setColorTexture(const Z3DTexture *colorTex) { m_colorTexture = colorTex; }
  void setDepthTexture(const Z3DTexture *depthTex) { m_depthTexture = depthTex; }
  // Multiply_By_Alpha : output color will be multiplied by alpha value (convert to premultiplied format)
  // None (default): just copy
  // Divide_By_Alpha : output color will be divided by alpha value (input is premultiplied format)
  void setOutputColorOption(OutputColorOption option)
    { m_colorOpAlpha = option == None ? 0.f : option == Divide_By_Alpha ? -1.f : 1.f; }

  // if true, color with zero alpha value should be discarded, which might save many depth texture lookup. default is false
  // Make sure your color and depth buffer are cleared before if set to true
  // glClear + discard transparent  is usually faster than   not discard transparent if many pixels are empty
  void setDiscardTransparent(bool v) { m_discardTransparent = v; }
  
signals:
  
public slots:

protected:
  virtual void compile();
  virtual void initialize();
  virtual void deinitialize();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void renderUsingGLSL(Z3DEye eye);
  virtual void renderPickingUsingGLSL(Z3DEye);

  const Z3DTexture *m_colorTexture;
  const Z3DTexture *m_depthTexture;

  Z3DShaderProgram m_copyTextureShader;
  float m_colorOpAlpha;
  bool m_discardTransparent;
};

#endif // Z3DTEXTURECOPYRENDERER_H
