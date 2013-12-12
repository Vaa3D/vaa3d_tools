#ifndef Z3DBACKGROUNDRENDERER_H
#define Z3DBACKGROUNDRENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dshadergroup.h"

class Z3DBackgroundRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  explicit Z3DBackgroundRenderer(QObject *parent = 0);

  void setFirstColor(glm::vec3 color) { m_firstColor.set(glm::vec4(color, 1.f)); }
  void setSecondColor(glm::vec3 color) { m_secondColor.set(glm::vec4(color, 1.f)); }
  
  void setFirstColor(double r, double g, double b, double alpha)
    { m_firstColor.set(glm::vec4(r*alpha, g*alpha, b*alpha, alpha)); }
  void setSecondColor(double r, double g, double b, double alpha)
    { m_secondColor.set(glm::vec4(r*alpha, g*alpha, b*alpha, alpha)); }

signals:
  
protected slots:
  void adjustWidgets();

protected:
  virtual void compile();
  virtual void initialize();
  virtual void deinitialize();
  virtual QString generateHeader();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void renderUsingGLSL(Z3DEye eye);
  virtual void renderPickingUsingGLSL(Z3DEye);

  Z3DShaderGroup m_backgroundShaderGrp;

  ZVec4Parameter m_firstColor;
  ZVec4Parameter m_secondColor;
  ZOptionParameter<QString> m_gradientOrientation;
  ZOptionParameter<QString> m_mode;

  GLuint m_VBO;
};

#endif // Z3DBACKGROUNDRENDERER_H
