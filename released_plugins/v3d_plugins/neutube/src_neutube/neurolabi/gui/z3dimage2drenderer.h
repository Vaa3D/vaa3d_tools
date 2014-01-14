#ifndef Z3DIMAGE2DRENDERER_H
#define Z3DIMAGE2DRENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dmesh.h"

class Z3DVolume;

// render 2d image without color transfer function
// support up to 5 channel, use color of each volume to composite final image
class Z3DImage2DRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  explicit Z3DImage2DRenderer(QObject *parent = 0);
  ~Z3DImage2DRenderer();

  // input can be NULL means don't have this channel
  void setChannel1(Z3DVolume *vol);
  void setChannel2(Z3DVolume *vol);
  void setChannel3(Z3DVolume *vol);
  void setChannel4(Z3DVolume *vol);
  void setChannel5(Z3DVolume *vol);
  void setChannels(std::vector<Z3DVolume *> vols);

  // quad contains corner vertex and 2d texture coordinates
  // clear
  void clearQuads() { m_quads.clear(); }
  // add quad
  void addQuad(const Z3DTriangleList &quad);

signals:

protected slots:

protected:
  void bindVolumes(Z3DShaderProgram &shader);
  bool hasVolume() const;

  virtual void compile();
  virtual void initialize();
  virtual void deinitialize();
  virtual QString generateHeader();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void renderUsingGLSL(Z3DEye eye);
  virtual void renderPickingUsingGLSL(Z3DEye);

  Z3DShaderProgram m_image2DShader;

  std::vector<Z3DVolume *> m_volumes;
  std::vector<QString> m_volumeUniformNames;

private:
  std::vector<Z3DTriangleList> m_quads;
};

#endif // Z3DIMAGE2DRENDERER_H
