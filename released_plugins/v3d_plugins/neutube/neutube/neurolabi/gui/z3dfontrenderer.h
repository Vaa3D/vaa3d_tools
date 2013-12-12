#ifndef Z3DFONTRENDERER_H
#define Z3DFONTRENDERER_H

#include "z3dprimitiverenderer.h"
class Z3DSDFont;
#include "z3dshadergroup.h"

class Z3DFontRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  explicit Z3DFontRenderer(QObject *parent = 0);
  virtual ~Z3DFontRenderer();

  void setData(std::vector<glm::vec3> *positions, const QStringList &texts);
  void setDataColors(std::vector<glm::vec4> *colors);
  void setDataPickingColors(std::vector<glm::vec4> *pickingColors = NULL);

protected slots:
  void adjustWidgets();
  virtual void compile();

protected:
  virtual void initialize();
  virtual void deinitialize();
  virtual std::vector<glm::vec4>* getColors();
  virtual QString generateHeader();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void renderUsingGLSL(Z3DEye eye);
  virtual void renderPickingUsingGLSL(Z3DEye);

  void prepareFontShaderData(Z3DEye eye);

  Z3DShaderGroup m_fontShaderGrp;

  ZOptionParameter<QString, size_t> m_allFontNames;  // font name and index into m_allFonts
  ZFloatParameter m_fontSize;   //font size in world coordinate
  ZBoolParameter m_fontUseSoftEdge;
  ZFloatParameter m_fontSoftEdgeScale;
  ZBoolParameter m_showFontOutline;
  ZOptionParameter<QString> m_fontOutlineMode;
  ZVec4Parameter m_fontOutlineColor;
  ZBoolParameter m_showFontShadow;
  ZVec4Parameter m_fontShadowColor;

  std::vector<Z3DSDFont*> m_allFonts;

  std::vector<glm::vec3> *m_positionsPt;
  std::vector<glm::vec4> *m_colorsPt;
  std::vector<glm::vec4> *m_pickingColorsPt;
  QStringList m_texts;
  std::vector<glm::vec4> m_colors;
  std::vector<glm::vec4> m_fontColors;
  std::vector<glm::vec4> m_fontPickingColors;
  std::vector<glm::vec3> m_fontPositions;
  std::vector<glm::vec2> m_fontTextureCoords;
  std::vector<GLuint> m_indexs;

  std::vector<GLuint> m_VBOs;
  std::vector<GLuint> m_pickingVBOs;
  bool m_dataChanged;
  bool m_pickingDataChanged;
};

#endif // Z3DFONTRENDERER_H
