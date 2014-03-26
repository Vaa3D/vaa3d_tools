#ifndef Z3DPRIMITIVERENDERER_H
#define Z3DPRIMITIVERENDERER_H

#include <QObject>
#include "z3drendererbase.h"

class Z3DTriangleList;

class Z3DPrimitiveRenderer : public QObject
{
  Q_OBJECT

  friend class Z3DRendererBase;
public:
  explicit Z3DPrimitiveRenderer(QObject *parent);
  virtual ~Z3DPrimitiveRenderer();
  inline virtual QString getClassName() const {return metaObject()->className();}

  // for opengl mode only, if set, display list will be build in opengl mode.
  // for large amount of objects, display list can render faster but it is expensive to build.
  // not necessary if number of objects is small (can be rendered in a few opengl calls)
  // default is false, subclass should call this function if needed
  void setUseDisplayList(bool v) { m_useDisplayList = v; }
  // If set, lighting will be enabled.
  // default is false, subclass should call this function if needed
  void setNeedLighting(bool v) { m_needLighting = v; }

  //sometimes z scale transfrom is not appropriate, for example: bound box. we need to disable it and
  // precalc the correct location. Default is true
  void setRespectRendererBaseCoordScales(bool v) {m_respectRendererBaseCoordScale = v;}
  
  inline void setZScale(float s) { m_rendererBase->setZScale(s); }
  inline glm::vec3 getCoordScales() const
  {
    if (m_respectRendererBaseCoordScale)
      return m_rendererBase->getCoordScales();
    else
      return glm::vec3(1.f,1.f,1.f);
  }

signals:
  void openglRendererInvalid();
  void openglPickingRendererInvalid();
  
public slots:
  virtual void coordScalesChanged();

protected slots:
  void invalidateOpenglRenderer();
  void invalidateOpenglPickingRenderer();
  virtual void compile() = 0;

protected:
  inline float getOpacity()  const {return m_rendererBase->getOpacity();}
  inline float getSizeScale() const {return m_rendererBase->getSizeScale();}
  inline Z3DCamera& getCamera() const {return m_rendererBase->getCamera();}
  inline glm::ivec4 getViewport() const {return m_rendererBase->getViewport();}

  virtual inline bool getNeedLighting() {return m_needLighting;}
  virtual inline bool getUseDisplayList() {return m_useDisplayList;}
  virtual inline void setRendererBase(Z3DRendererBase* base) { m_rendererBase = base;}

  virtual void initialize();
  virtual void deinitialize();

  virtual void renderUsingOpengl() = 0;
  virtual void renderPickingUsingOpengl() = 0;

  virtual void renderUsingGLSL(Z3DEye) = 0;
  virtual void renderPickingUsingGLSL(Z3DEye) = 0;

  void addParameter(ZParameter &para);
  void addParameter(ZParameter *para);
  std::vector<ZParameter*> getParameters();

  inline void setInitialized(bool i) {m_initialized = i;}

  virtual QString generateHeader();

  // commonly used render functions
  // Render a screen-aligned quad (whole screen) with depth func GL_ALWAYS.
  void renderScreenQuad(const Z3DShaderProgram &shader, bool depthAlwaysPass = true);
  // render a trianglelist with whatever it contains
  void renderTriangleList(const Z3DShaderProgram &shader, const Z3DTriangleList &mesh);

  Z3DRendererBase* m_rendererBase;
  bool m_initialized;
  bool m_needLighting;
  bool m_useDisplayList;
  bool m_respectRendererBaseCoordScale;

  std::vector<ZParameter*> m_parameters;

  bool m_hardwareSupportVAO;
  GLuint m_VAO;
  GLuint m_pickingVAO;

private:
  GLuint m_privateVAO;
};

#endif // Z3DPRIMITIVERENDERER_H
