#ifndef Z3DSPHERERENDERER_H
#define Z3DSPHERERENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dshadergroup.h"

class Z3DSphereRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  // default use display list and lighting for opengl mode
  explicit Z3DSphereRenderer(QObject *parent = 0);
  virtual ~Z3DSphereRenderer();

  void setData(std::vector<glm::vec4> *pointAndRadiusInput, std::vector<glm::vec4> *specularAndShininessInput = NULL);
  void setDataColors(std::vector<glm::vec4> *pointColorsInput);
  void setDataPickingColors(std::vector<glm::vec4> *pointPickingColorsInput = NULL);

protected:
  virtual void compile();
  virtual void initialize();
  virtual void deinitialize();
  virtual QString generateHeader();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void renderUsingGLSL(Z3DEye eye);
  virtual void renderPickingUsingGLSL(Z3DEye eye);

  void appendDefaultColors();

  Z3DShaderGroup m_sphereShaderGrp;

  ZIntParameter m_sphereSlicesStacks;
  ZBoolParameter m_useDynamicMaterial;

private:
  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_specularAndShininess;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec4> m_pointPickingColors;
  std::vector<GLfloat> m_allFlags;
  std::vector<GLuint> m_indexs;

  //std::vector<GLuint> m_VBOs;
  //std::vector<GLuint> m_pickingVBOs;
  std::vector<GLuint> m_VAOs;
  std::vector<GLuint> m_pickingVAOs;
  std::vector<std::vector<GLuint> > m_VBOs;
  std::vector<std::vector<GLuint> > m_pickingVBOs;
  bool m_dataChanged;
  bool m_pickingDataChanged;
  size_t m_oneBatchNumber;
};

#endif // Z3DSPHERERENDERER_H
