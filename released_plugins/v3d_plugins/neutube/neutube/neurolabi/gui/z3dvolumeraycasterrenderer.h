#ifndef Z3DVOLUMERAYCASTERRENDERER_H
#define Z3DVOLUMERAYCASTERRENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dtransferfunction.h"
#include "z3dmesh.h"

// use raycaster to render volume or 2D Image (stack with depth==1) with color
// transfer functions
// only support up to 5 channels now. todo:support arbitrary number of channels(need to
// find a way to tell processors that parameters from this renderer has been changed and
// let processors to change there UI)
class Z3DVolumeRaycasterRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  explicit Z3DVolumeRaycasterRenderer(QObject *parent = 0);
  ~Z3DVolumeRaycasterRenderer();

  // input can be NULL means don't have this channel
  void setChannel1(Z3DVolume *vol);
  void setChannel2(Z3DVolume *vol);
  void setChannel3(Z3DVolume *vol);
  void setChannel4(Z3DVolume *vol);
  void setChannel5(Z3DVolume *vol);
  // or set all channels at once
  void setChannels(std::vector<Z3DVolume*> vols);

  // quad or entryexit texture should be set before rendering

  // For 2D Image rendering, once set, entry exit textures will be cleared and
  // renderer switch to 2D mode
  // To render a 2D image, quad should contains corner vertex and 2d texture coordinates
  // To render a slice in 3D volume, quad should contains corner vertex and 3d texture coordinates
  // DO NOT call this function for 3d Raycaster
  // clear
  void clearQuads() { m_quads.clear(); }
  // add quad
  void addQuad(const Z3DTriangleList &quad);
  // For 3D Raycasting rendering, once called, 2d quads will be cleared and renderer
  // switch to 3D mode
  void setEntryExitCoordTextures(const Z3DTexture *entryCoordTexture,
                                 const Z3DTexture *entryDepthTexture,
                                 const Z3DTexture *exitCoordTexture,
                                 const Z3DTexture *exitDepthTexture);

  void translate(double dx, double dy, double dz);

  //
  bool channel1IsVisible() const { return m_channel1Visible.get(); }
  bool channel2IsVisible() const { return m_channel2Visible.get(); }
  bool channel3IsVisible() const { return m_channel3Visible.get(); }
  bool channel4IsVisible() const { return m_channel4Visible.get(); }
  bool channel5IsVisible() const { return m_channel5Visible.get(); }
  void setChannel1Visible(bool v) { m_channel1Visible.set(v); }
  void setChannel2Visible(bool v) { m_channel2Visible.set(v); }
  void setChannel3Visible(bool v) { m_channel3Visible.set(v); }
  void setChannel4Visible(bool v) { m_channel4Visible.set(v); }
  void setChannel5Visible(bool v) { m_channel5Visible.set(v); }

  bool channel1Exist() const { return m_volumes[0] != NULL; }
  bool channel2Exist() const { return m_volumes[1] != NULL; }
  bool channel3Exist() const { return m_volumes[2] != NULL; }
  bool channel4Exist() const { return m_volumes[3] != NULL; }
  bool channel5Exist() const { return m_volumes[4] != NULL; }

  // return true if something is rendered by this renderer
  bool hasVisibleRendering() const
  {
    return (channel1Exist() && channel1IsVisible()) ||
        (channel2Exist() && channel2IsVisible()) ||
        (channel3Exist() && channel3IsVisible()) ||
        (channel4Exist() && channel4IsVisible()) ||
        (channel5Exist() && channel5IsVisible());
  }

  inline void setOpaque(bool opaque) {
    m_opaque = opaque;
  }

  inline void setAlpha(double alpha) {
    m_alpha = alpha;
    resetTransferFunctions();
  }

  inline double getAlpha() {
    return m_alpha;
  }

  void setCompositeMode(const QString &option);
  void setTextureFilterMode(const QString &option);

signals:

protected slots:
  void adjustWidgets();

protected:
  void bindVolumesAndTransferFuncs(Z3DShaderProgram &shader);

  virtual void compile();
  virtual void initialize();
  virtual void deinitialize();
  virtual QString generateHeader();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void renderUsingGLSL(Z3DEye eye);
  virtual void renderPickingUsingGLSL(Z3DEye);

  Z3DShaderProgram m_raycasterShader;
  Z3DShaderProgram m_2dImageShader;
  Z3DShaderProgram m_volumeSliceWithTransferfunShader;

  ZFloatParameter m_samplingRate;  // Sampling rate of the raycasting, specified relative to the size of one voxel
  ZFloatParameter m_isoValue;  // The used isovalue, when isosurface raycasting is enabled
  ZFloatParameter m_localMIPThreshold;

  ZOptionParameter<QString> m_gradientMode;   // What type of calculation should be used for on-the-fly gradients
  ZOptionParameter<QString> m_compositingMode;  // What compositing mode should be applied

  ZBoolParameter m_channel1Visible;
  ZBoolParameter m_channel2Visible;
  ZBoolParameter m_channel3Visible;
  ZBoolParameter m_channel4Visible;
  ZBoolParameter m_channel5Visible;

  Z3DTransferFunctionParameter m_transferFunc1;       // transfer function to apply to channel 1
  Z3DTransferFunctionParameter m_transferFunc2;       // transfer function to apply to channel 2
  Z3DTransferFunctionParameter m_transferFunc3;       // transfer function to apply to channel 3
  Z3DTransferFunctionParameter m_transferFunc4;       // transfer function to apply to channel 4
  Z3DTransferFunctionParameter m_transferFunc5;       // transfer function to apply to channel 5

  ZOptionParameter<QString, GLint> m_texFilterMode1;      // filter mode for channel 1
  ZOptionParameter<QString, GLint> m_texFilterMode2;      // filter mode for channel 2
  ZOptionParameter<QString, GLint> m_texFilterMode3;      // filter mode for channel 3
  ZOptionParameter<QString, GLint> m_texFilterMode4;      // filter mode for channel 4
  ZOptionParameter<QString, GLint> m_texFilterMode5;      // filter mode for channel 5

  std::vector<Z3DVolume *> m_volumes;
  std::vector<QString> m_volumeUniformNames;
  std::vector<QString> m_transferFuncUniformNames;

private:
  // update number of channel and dimension and adjust widgets and recompile shaders if necessary
  void updateChannel();
  // this function is used to get proper default
  // transfer functions (grey or color depends on current number of channel)
  void resetTransferFunctions();

  size_t m_nChannel;
  bool m_is2DImage;

  std::vector<Z3DTriangleList> m_quads;
  const Z3DTexture *m_entryCoordTexture;
  const Z3DTexture *m_entryDepthTexture;
  const Z3DTexture *m_exitCoordTexture;
  const Z3DTexture *m_exitDepthTexture;

  // for convinience
  std::vector<ZBoolParameter*> m_channelVisibleParas;
  std::vector<Z3DTransferFunctionParameter*> m_transferFuncParas;
  std::vector<ZOptionParameter<QString, GLint>*> m_texFilterModeParas;

  bool m_opaque;
  double m_alpha; //only takes effect when m_opaque is true
};

#endif // Z3DVOLUMERAYCASTERRENDERER_H
