#ifndef Z3DRENDERERBASE_H
#define Z3DRENDERERBASE_H

#include <QObject>
#include <vector>
#include <map>
#include "znumericparameter.h"
#include "zoptionparameter.h"
#include "z3dshaderprogram.h"
#include "z3dcamera.h"
class Z3DPrimitiveRenderer;
class Z3DTexture;

// contains basic properties such as lighting, method, size for rendering.
// A renderBase usually contains multiple primitive renderers. Some of those are
// combined to draw a complicated object. Some of those are just sharing the enviroment
// (rendering parameters).  You can use function "enableRenderer" to choose current needed renderers.
class Z3DRendererBase : public QObject
{
  Q_OBJECT
public:
  enum ActivateRendererOption
  {
    None, DeactivateOthers
  };

  enum ShaderHookType
  {
    Normal, DualDepthPeelingInit, DualDepthPeelingPeel, WeightedAverageInit
  };

  struct ShaderHookParameter
  {
    Z3DTexture *dualDepthPeelingDepthBlenderTexture;
    Z3DTexture *dualDepthPeelingFrontBlenderTexture;
  };

  explicit Z3DRendererBase(QObject *parent = 0);
  virtual ~Z3DRendererBase();

  inline void setCamera(const Z3DCamera& c) { m_camera = c; }
  inline void setViewport(glm::ivec4 viewport) { m_viewport = viewport; }
  inline void setViewport(glm::ivec2 viewport) { m_viewport = glm::ivec4(0,0,viewport); }
  inline Z3DCamera& getCamera() {return m_camera;}
  inline glm::ivec4 getViewport() const {return m_viewport;}

  // need valid camera and viewport
  virtual void setGlobalShaderParameters(Z3DShaderProgram &shader, Z3DEye eye);
  virtual void setGlobalShaderParameters(Z3DShaderProgram *shader, Z3DEye eye);
  virtual QString generateHeader() const;

  // rendererbase will take ownership of this renderer and delete it in destructor
  void addRenderer(Z3DPrimitiveRenderer* renderer);
  void removeRenderer(Z3DPrimitiveRenderer* renderer);

  // Only activated renderers will be called by rendererbase.
  // By default all renderers are deactivated. These functions will activate or
  // deactivate certain renderers
  void deactivateAllRenderers();
  void activateRenderer(Z3DPrimitiveRenderer* renderer, ActivateRendererOption option = DeactivateOthers);
  void activateRenderer(Z3DPrimitiveRenderer *renderer1, Z3DPrimitiveRenderer *renderer2,
                        ActivateRendererOption option = DeactivateOthers);
  void activateRenderer(Z3DPrimitiveRenderer *renderer1, Z3DPrimitiveRenderer *renderer2,
                        Z3DPrimitiveRenderer *renderer3, ActivateRendererOption option = DeactivateOthers);
  void activateRenderer(const std::vector<Z3DPrimitiveRenderer*> &renderers,
                        ActivateRendererOption option = DeactivateOthers);

  // "Old openGL" or "GLSL" (default)
  inline void setRenderMethod(const QString &method) { m_renderMethod.select(method);}
  inline QString getRenderMethod() const { return m_renderMethod.get(); }

  inline void setSizeScale(float s) { m_sizeScale.set(s); }
  inline void setXScale(float s) { m_coordXScale.set(s); }
  inline void setYScale(float s) { m_coordYScale.set(s); }
  inline void setZScale(float s) { m_coordZScale.set(s); }
  inline void setOpacity(float o) { m_opacity.set(o); }

  inline void setViewMatrix(const glm::mat4 &vm) {m_hasCustomViewMatrix=true; m_customViewMatrix=vm;}
  // use view matrix from camera
  inline void unsetViewMatrix() {m_hasCustomViewMatrix=false;}
  inline void setProjectionMatrix(const glm::mat4 &pm) {m_hasCustomProjectionMatrix=true; m_customProjectionMatrix=pm;}
  // use projection matrix from camera
  inline void unsetProjectionMatrix() {m_hasCustomProjectionMatrix=false;}
  glm::mat4 getViewMatrix(Z3DEye eye);
  // singular matrix does not matter for opengl
  glm::mat4 getViewMatrixInverse(Z3DEye eye);
  glm::mat4 getProjectionMatrix(Z3DEye eye);
  glm::mat4 getProjectionMatrixInverse(Z3DEye eye);
  glm::mat3 getNormalMatrix(Z3DEye eye);
  glm::mat4 getViewportMatrix();
  glm::mat4 getViewportMatrixInverse();

  void setClipPlanes(std::vector<glm::dvec4> *clipPlanes);

  void addParameter(ZParameter &para);
  void addParameter(ZParameter *para);
  std::vector<ZParameter*> getParameters() const;
  std::vector<ZParameter*> getRendererParameters(Z3DPrimitiveRenderer *renderer) const;

  // return scale of x,y,z coordinate
  inline glm::vec3 getCoordScales() const {return glm::vec3(m_coordXScale.get(), m_coordYScale.get(), m_coordZScale.get());}
  inline float getOpacity() const {return m_opacity.get();}
  inline float getSizeScale() const {return m_sizeScale.get();}

  virtual void render(Z3DEye eye);
  virtual void renderPicking(Z3DEye eye);

  inline void setShaderHookType(ShaderHookType t) { m_shaderHookType = t; }
  inline ShaderHookType getShaderHookType() const { return m_shaderHookType; }
  inline ShaderHookParameter& shaderHookPara() { return m_shaderHookPara; }

protected:

  virtual void generateDisplayList();
  virtual void generatePickingDisplayList();

  virtual void adjustWidgets();

  void renderInstant();
  void renderPickingInstant();
  void renderUsingGLSL(Z3DEye eye);
  void renderPickingUsingGLSL(Z3DEye eye);

  bool needLighting() const;
  bool useDisplayList() const;

  inline bool hasClipPlanes() { return !m_clipPlanes.empty(); }
  void activateClipPlanesOpenGL();
  void deactivateClipPlanesOpenGL();
  void activateClipPlanesGLSL();
  void deactivateClipPlanesGLSL();

signals:
  void coordScalesChanged();
  void sizeScaleChanged();
  
public slots:
  void invalidateDisplayList();
  void invalidatePickingDisplayList();
  void compile();

protected:
  // display list generated from the geometry.
  GLuint m_displayList;
  GLuint m_pickingDisplayList;

  // scale of x, y, z coordinate
  ZFloatParameter m_coordXScale;
  ZFloatParameter m_coordYScale;
  ZFloatParameter m_coordZScale;
  ZOptionParameter<QString> m_renderMethod;

  ZFloatParameter m_sizeScale;
  ZFloatParameter m_opacity;

  ZBoolParameter m_filterNotFrontFacing;

  ZVec4Parameter m_materialAmbient;
  ZVec4Parameter m_materialSpecular;
  ZFloatParameter m_materialShininess;

  ZIntParameter m_lightCount;
  std::vector<ZVec4Parameter*> m_lightPositions;
  std::vector<ZVec4Parameter*> m_lightAmbients;
  std::vector<ZVec4Parameter*> m_lightDiffuses;
  std::vector<ZVec4Parameter*> m_lightSpeculars;
  // The light source's attenuation factors (x = constant, y = linear, z = quadratic)
  std::vector<ZVec3Parameter*> m_lightAttenuations;
  std::vector<ZFloatParameter*> m_lightSpotCutoff;
  std::vector<ZFloatParameter*> m_lightSpotExponent;
  std::vector<ZVec3Parameter*> m_lightSpotDirection;
  ZVec4Parameter m_sceneAmbient;

  // fog
  ZOptionParameter<QString> m_fogMode;
  ZVec3Parameter m_fogTopColor;
  ZVec3Parameter m_fogBottomColor;
  ZIntSpanParameter m_fogRange;
  ZFloatParameter m_fogDensity;

  Z3DCamera m_camera;
  glm::ivec4 m_viewport;

  std::vector<ZParameter*> m_parameters;
  // renderers and its activation state
  std::map<Z3DPrimitiveRenderer*,bool> m_renderers;
  //std::vector<Z3DPrimitiveRenderer*> m_renderers;
  //std::vector<bool> m_rendererEnableStates;

  bool m_hasCustomViewMatrix;
  bool m_hasCustomProjectionMatrix;
  glm::mat4 m_customViewMatrix;
  glm::mat4 m_customProjectionMatrix;

  std::vector<glm::dvec4> m_clipPlanes;

  ShaderHookType m_shaderHookType;
  ShaderHookParameter m_shaderHookPara;

private:
  std::map<Z3DPrimitiveRenderer*,bool>::iterator m_renderersIt;
  std::map<Z3DPrimitiveRenderer*,bool> m_lastOpenglRenderingState;
  std::map<Z3DPrimitiveRenderer*,bool> m_lastOpenglPickingRenderingState;
};

#endif // Z3DRENDERERBASE_H
