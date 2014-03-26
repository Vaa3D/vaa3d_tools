#ifndef Z3DVOLUMESLICERENDERER_H
#define Z3DVOLUMESLICERENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dmesh.h"

class Z3DVolume;

// render 2d slices of volume without color transfer function
// support up to 5 channel, use color of each volume to composite final image
class Z3DVolumeSliceRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  explicit Z3DVolumeSliceRenderer(QObject *parent = 0);
  ~Z3DVolumeSliceRenderer();

  // input can be NULL means don't have this channel
  void setChannel1(Z3DVolume *vol);
  void setChannel2(Z3DVolume *vol);
  void setChannel3(Z3DVolume *vol);
  void setChannel4(Z3DVolume *vol);
  void setChannel5(Z3DVolume *vol);
  void setChannels(std::vector<Z3DVolume*> vols);

  // a slice (quad) in 3D volume contains corner vertex and 3d texture coordinates
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

  Z3DShaderProgram m_volumeSliceShader;

  std::vector<Z3DVolume *> m_volumes;
  std::vector<QString> m_volumeUniformNames;

private:
  std::vector<Z3DTriangleList> m_quads;
};

#endif // Z3DVOLUMESLICERENDERER_H
