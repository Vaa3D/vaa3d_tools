#ifndef Z3DAXIS_H
#define Z3DAXIS_H

#include <QObject>
#include "z3dgeometryfilter.h"
class Z3DArrowRenderer;
class Z3DLineRenderer;
class Z3DFontRenderer;
class ZWidgetsGroup;

class Z3DAxis : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  explicit Z3DAxis();
  virtual ~Z3DAxis();

  virtual bool isReady(Z3DEye eye) const;

  void setVisible(bool visible);

  // caller should clean up this (by delete parent of this zwidgetgroup)
  ZWidgetsGroup *getWidgetsGroup();

signals:

public slots:

protected:
  void initialize();
  void deinitialize();

  virtual void render(Z3DEye eye);

  virtual void prepareData(Z3DEye eye);

  void setupCamera();

  Z3DLineRenderer *m_lineRenderer;
  Z3DArrowRenderer *m_arrowRenderer;
  Z3DFontRenderer *m_fontRenderer;

  ZBoolParameter m_showAxis;
  ZVec4Parameter m_XAxisColor;
  ZVec4Parameter m_YAxisColor;
  ZVec4Parameter m_ZAxisColor;
  ZFloatParameter m_axisRegionRatio;
  ZOptionParameter<QString> m_mode;

  std::vector<glm::vec4> m_tailPosAndTailRadius;
  std::vector<glm::vec4> m_headPosAndHeadRadius;
  std::vector<glm::vec4> m_lineColors;
  std::vector<glm::vec3> m_lines;
  std::vector<glm::vec4> m_textColors;
  std::vector<glm::vec3> m_textPositions;

  glm::vec3 m_XEnd;
  glm::vec3 m_YEnd;
  glm::vec3 m_ZEnd;

  ZWidgetsGroup *m_widgetsGroup;
};

#endif // Z3DAXIS_H
