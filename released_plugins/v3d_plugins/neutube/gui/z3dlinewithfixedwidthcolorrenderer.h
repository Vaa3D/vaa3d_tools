#ifndef Z3DLINEWITHFIXEDWIDTHCOLORRENDERER_H
#define Z3DLINEWITHFIXEDWIDTHCOLORRENDERER_H

#include "z3dlinerenderer.h"

class Z3DLineWithFixedWidthColorRenderer : public Z3DLineRenderer
{
  Q_OBJECT
public:
  // default use display list but not lighting in opengl mode
  explicit Z3DLineWithFixedWidthColorRenderer(QObject *parent = 0);

  void setData(std::vector<glm::vec3> *linesInput);
  void setLineWidthGuiName(const QString &name);
  void setLineColorGuiName(const QString &name);
  
signals:
  
protected slots:
  virtual void setLineColors();

protected:
  ZFloatParameter m_lineWidth;
  ZVec4Parameter m_lineColor;

  virtual float getLineWidth() const;

  std::vector<glm::vec4> m_lineColorsPrivate;
  
};

#endif // Z3DLINEWITHFIXEDWIDTHCOLORRENDERER_H
