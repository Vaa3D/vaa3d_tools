#ifndef Z3DCONERENDERER_H
#define Z3DCONERENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dshadergroup.h"

class Z3DConeRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  // default use display list and lighting in opengl mode
  // Round cap style might have bug. It only works when we are dealing with cylinder with slightly different radius.
  explicit Z3DConeRenderer(QObject *parent = 0);
  virtual ~Z3DConeRenderer();

  // base radius should be smaller than top radius
  void setData(std::vector<glm::vec4> *baseAndBaseRadius, std::vector<glm::vec4> *axisAndTopRadius);
  void setDataColors(std::vector<glm::vec4> *coneColors);
  void setDataColors(std::vector<glm::vec4> *coneBaseColors, std::vector<glm::vec4> *coneTopColors);
  void setDataPickingColors(std::vector<glm::vec4> *conePickingColors = NULL);

signals:

protected slots:

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

  Z3DShaderGroup m_coneShaderGrp;

  ZOptionParameter<QString, QString> m_coneCapStyle;
  ZIntParameter m_cylinderSubdivisionAroundZ;
  ZIntParameter m_cylinderSubdivisionAlongZ;

private:
  std::vector<glm::vec4> m_baseAndBaseRadius;
  std::vector<glm::vec4> m_axisAndTopRadius;
  std::vector<glm::vec4> m_coneBaseColors;
  std::vector<glm::vec4> m_coneTopColors;
  std::vector<glm::vec4> m_conePickingColors;
  std::vector<GLfloat> m_allFlags;
  std::vector<GLuint> m_indexs;

  bool m_sameColorForBaseAndTop;

  bool m_useConeShader2;

  std::vector<GLuint> m_VBOs;
  std::vector<GLuint> m_pickingVBOs;
  bool m_dataChanged;
  bool m_pickingDataChanged;
};

#endif // Z3DCONERENDERER_H
