#ifndef Z3DCOMPOSITOR_H
#define Z3DCOMPOSITOR_H

#include "z3drenderport.h"
#include "z3drenderprocessor.h"
#include "z3dgeometryfilter.h"
#include "z3dpickingmanager.h"
#include "z3dcameraparameter.h"
#include "z3dbackgroundrenderer.h"
#include "zwidgetsgroup.h"

class Z3DTextureBlendRenderer;
class Z3DTextureCopyRenderer;
class Z3DTrackballInteractionHandler;

class Z3DCompositor : public Z3DRenderProcessor
{
  Q_OBJECT
public:
  Z3DCompositor();
  ~Z3DCompositor();

  virtual void initialize();
  virtual void deinitialize();
  virtual bool isReady(Z3DEye eye) const;

  inline Z3DTrackballInteractionHandler* getInteractionHandler() { return m_interactionHandler; }
  inline Z3DCameraParameter* getCamera() { return &m_camera; }

  void setShowBackground(bool v) { m_showBackground.set(v); }
  void setBackgroundFirstColor(glm::vec3 color) { m_backgroundRenderer->setFirstColor(color); }
  void setBackgroundSecondColor(glm::vec3 color) { m_backgroundRenderer->setSecondColor(color); }
  void setBackgroundFirstColor(double r, double g, double b, double alpha)
    { m_backgroundRenderer->setFirstColor(r, g, b, alpha); }
  void setBackgroundSecondColor(double r, double g, double b, double alpha)
    { m_backgroundRenderer->setSecondColor(r, g, b, alpha); }

  // caller should clean up this (by delete parent of this zwidgetgroup)
  ZWidgetsGroup *getBackgroundWidgetsGroup();

  void savePickingBufferToImage(const QString &filename);

protected:
  virtual void process(Z3DEye eye);

private:
  // little helper function
  void renderGeometries(const std::vector<Z3DGeometryFilter*> &filters,
                        Z3DRenderOutputPort &port, Z3DEye eye);

  void renderGeomsBlendDelayed(const std::vector<Z3DGeometryFilter*> &filters,
                               Z3DRenderOutputPort &port, Z3DEye eye);
  void renderGeomsBlendNoDepthMask(const std::vector<Z3DGeometryFilter*> &filters,
                                   Z3DRenderOutputPort &port, Z3DEye eye);
  void renderGeomsOIT(const std::vector<Z3DGeometryFilter*> &filters,
                      Z3DRenderOutputPort &port, Z3DEye eye, const QString &method);

  void renderOpaque(const std::vector<Z3DGeometryFilter*> &filters,
                    Z3DRenderOutputPort &port, Z3DEye eye);

  void renderTransparentDDP(const std::vector<Z3DGeometryFilter*> &filters,
                            Z3DRenderOutputPort &port, Z3DEye eye);
  bool createDDPRenderTarget(glm::ivec2 size);

  void renderTransparentWA(const std::vector<Z3DGeometryFilter*> &filters,
                           Z3DRenderOutputPort &port, Z3DEye eye);
  bool createWARenderTarget(glm::ivec2 size);

private:
  Z3DTextureBlendRenderer *m_alphaBlendRenderer;
  Z3DTextureBlendRenderer *m_firstOnTopBlendRenderer;
  Z3DTextureCopyRenderer *m_textureCopyRenderer;
  Z3DBackgroundRenderer *m_backgroundRenderer;
  Z3DPickingManager m_pickingManager;

  ZBoolParameter m_showBackground;
  ZBoolParameter m_renderGeometries;
  Z3DCameraParameter m_camera;
  ZOptionParameter<QString> m_geometriesMultisampleMode;
  ZOptionParameter<QString> m_transparencyMethod;

  Z3DRenderInputPort m_inport;
  Z3DRenderInputPort m_leftEyeInport;
  Z3DRenderInputPort m_rightEyeInport;
  Z3DRenderOutputPort m_outport;
  Z3DRenderOutputPort m_leftEyeOutport;
  Z3DRenderOutputPort m_rightEyeOutport;
  Z3DRenderOutputPort m_tempPort;
  Z3DRenderOutputPort m_tempPort2;
  Z3DRenderOutputPort m_tempPort3;
  Z3DRenderOutputPort m_tempPort4;
  Z3DRenderOutputPort m_pickingPort;
  Z3DProcessorInputPort<Z3DGeometryFilter> m_pPort;

  Z3DRenderTarget *m_ddpRT;
  Z3DShaderProgram *m_ddpBlendShader;
  Z3DShaderProgram *m_ddpFinalShader;

  Z3DRenderTarget *m_waRT;
  Z3DShaderProgram *m_waFinalShader;

  GLuint m_queryId;

  Z3DTrackballInteractionHandler* m_interactionHandler;

  ZWidgetsGroup *m_backgroundWidgetsGroup;
};

#endif // Z3DCOMPOSITOR_H
