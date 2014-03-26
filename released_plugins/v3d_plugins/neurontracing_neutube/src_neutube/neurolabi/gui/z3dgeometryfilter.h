#ifndef Z3DGEOMETRYFILTER_H
#define Z3DGEOMETRYFILTER_H

#include "z3dprocessor.h"
#include "z3drendererbase.h"
#include "z3dport.h"
#include "z3dpickingmanager.h"

class Z3DGeometryFilter : public Z3DProcessor
{
  friend class Z3DCompositor;
public:
  Z3DGeometryFilter();
  virtual ~Z3DGeometryFilter();

  virtual void render(Z3DEye eye) = 0;
  virtual void renderPicking(Z3DEye) {}

  virtual bool isStayOnTop() const { return m_stayOnTop.get(); }
  virtual void setStayOnTop(bool s) { m_stayOnTop.set(s); }

  inline void setCamera(const Z3DCamera& c) {m_rendererBase->setCamera(c);}
  inline void setViewport(glm::ivec2 viewport) {m_rendererBase->setViewport(viewport);}
  inline void setViewport(glm::ivec4 viewport) {m_rendererBase->setViewport(viewport);}
  void setPickingManager(Z3DPickingManager* pm);
  inline Z3DCamera& getCamera() const {return m_rendererBase->getCamera();}
  inline glm::ivec4 getViewport() const {return m_rendererBase->getViewport();}
  inline Z3DPickingManager* getPickingManager() const {return m_pickingManager;}

  inline glm::vec3 getCoordScales() const {return m_rendererBase->getCoordScales();}
  inline float getOpacity() const {return m_rendererBase->getOpacity();}
  inline float getSizeScale() const {return m_rendererBase->getSizeScale();}
  inline Z3DRendererBase* getRendererBase() const {return m_rendererBase;}
  inline void setSizeScale(float s) { m_rendererBase->setSizeScale(s); }
  // "Old openGL" or "GLSL" (default)
  inline QString getRendererMethod() const { return m_rendererBase->getRenderMethod(); }

  inline void setShaderHookType(Z3DRendererBase::ShaderHookType t) { m_rendererBase->setShaderHookType(t); }
  inline Z3DRendererBase::ShaderHookType getShaderHookType() const { return m_rendererBase->getShaderHookType(); }
  inline Z3DRendererBase::ShaderHookParameter& shaderHookPara() { return m_rendererBase->shaderHookPara(); }

  // need blending if it will render objects with less than 1.0 alpha value
  // Z3Dcompositor will call this function to decide how to render this filter. This is important
  // because correct alpha blending (like dual depth peeling) can be very slow. We need this
  // function to optimize out opaque rendering.
  bool needBlending() const { return m_needBlending || getOpacity() < 1.f; }
  // call this if contained renderers will render transparent pixels even if global alpha value is 1.0.
  // for example, 3dfontrenderer will always render fonts surrounded by transparent pixels. linerenderer that
  // has line smooth hint will also have alpha output, but it can be ignored since there are little transparent pixels.
  // If such renderer exists and this function is not called, 3d compositor will treat it like opaque
  // rendering thus correct blending might not be applied as it should be. We said 'might' because in hardware
  // blending mode (use glEnable(GL_BLEND)) this does not matter. We can turn on GL_BLEND in that specific renderer
  // to make it correct (like we did in fontrenderer and linerenderer). In other blending mode like weighted average
  // or depth peeling, there is a high level pipeline in which renderers only provide their color, alpha and depth result.
  // All hardware flags have to be controlled by 3d compositor. In that case, we need this function to tell compositor
  // to put this filter into the pipeline.
  void setNeedBlending(bool v) { m_needBlending = v; }

protected:
  virtual void process(Z3DEye) {}

  // once processed, should be valid for both stereo view and mono view
  virtual void setValid(Z3DEye eye) { Z3DProcessor::setValid(eye); m_invalidationState = Valid; }

  // functions for picking, use these two function and m_pickingObjectsRegistered to control picking
  // note: input Z3DPickingManager might be NULL
  // after deregister, m_pickingObjectsRegistered should be false;
  virtual void deregisterPickingObjects(Z3DPickingManager*) {}
  // after register, m_pickingObjectsRegistered should be true and data picking color should be set
  // for renderers
  virtual void registerPickingObjects(Z3DPickingManager*) {}

  // output v1 is start point of ray, v2 is a point on the ray, v2-v1 is normalized
  // x and y are input screen point, width and height are input screen dimension
  void get3DRayUnderScreenPoint(glm::vec3 &v1, glm::vec3 &v2, int x, int y, int width, int height);
  void get3DRayUnderScreenPoint(glm::dvec3 &v1, glm::dvec3 &v2, int x, int y, int width, int height);

  Z3DProcessorOutputPort<Z3DGeometryFilter> m_outPort;
  Z3DRendererBase *m_rendererBase;

  ZBoolParameter m_stayOnTop;

  Z3DPickingManager *m_pickingManager;
  bool m_pickingObjectsRegistered;

  bool m_needBlending;
};

#endif // Z3DGEOMETRYFILTER_H
