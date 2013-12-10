#ifndef Z3DLINERENDERER_H
#define Z3DLINERENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dshadergroup.h"

class Z3DLineRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  // default use display list but not lighing in opengl mode
  explicit Z3DLineRenderer(QObject *parent = 0);
  virtual ~Z3DLineRenderer();

  void setData(std::vector<glm::vec3> *linesInput);
  inline void setLineWidth(const std::vector<float> &lineWidthArray) {
    m_lineWidthArray = lineWidthArray;
  }

  void setDataColors(std::vector<glm::vec4> *lineColorsInput);
  void setDataPickingColors(std::vector<glm::vec4> *linePickingColorsInput = NULL);

  inline void setLineWidth(float lineWidth) { m_lineWidth = lineWidth; }

protected:
  virtual void compile();

  virtual void initialize();
  virtual void deinitialize();
  virtual float getLineWidth() const;
  virtual std::vector<glm::vec4>* getLineColors();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void renderUsingGLSL(Z3DEye eye);
  virtual void renderPickingUsingGLSL(Z3DEye eye);

  void enableLineSmooth();
  void disableLineSmooth();

  Z3DShaderGroup m_lineShaderGrp;

  std::vector<glm::vec3> *m_linesPt;
  std::vector<glm::vec4> *m_lineColorsPt;
  std::vector<glm::vec4> *m_linePickingColorsPt;

  float m_lineWidth;
  std::vector<float> m_lineWidthArray;

private:
  std::vector<glm::vec4> m_lineColors;

  std::vector<GLuint> m_VBOs;
  std::vector<GLuint> m_pickingVBOs;
  bool m_dataChanged;
  bool m_pickingDataChanged;
};

#endif // Z3DLINERENDERER_H
