#include "z3dlinewithfixedwidthcolorrenderer.h"

Z3DLineWithFixedWidthColorRenderer::Z3DLineWithFixedWidthColorRenderer(QObject *parent)
  : Z3DLineRenderer(parent)
  , m_lineWidth("Line Width", 2.0f, 1.0f, 10.0f)
  , m_lineColor("Line Color", glm::vec4(1.f, 1.f, 0.f, 1.f))
{
  setNeedLighting(false);
  setUseDisplayList(true);
  m_lineColor.setStyle("COLOR");
  connect(&m_lineWidth, SIGNAL(valueChanged()), this, SLOT(invalidateOpenglRenderer()));
  connect(&m_lineWidth, SIGNAL(valueChanged()), this, SLOT(invalidateOpenglPickingRenderer()));
  connect(&m_lineColor, SIGNAL(valueChanged()), this, SLOT(invalidateOpenglRenderer()));
  connect(&m_lineColor, SIGNAL(valueChanged()), this, SLOT(setLineColors()));
  addParameter(m_lineWidth);
  addParameter(m_lineColor);
}

void Z3DLineWithFixedWidthColorRenderer::setData(std::vector<glm::vec3> *linesInput)
{
  Z3DLineRenderer::setData(linesInput);
  setLineColors();
}

void Z3DLineWithFixedWidthColorRenderer::setLineWidthGuiName(const QString &name)
{
  m_lineWidth.setName(name);
}

void Z3DLineWithFixedWidthColorRenderer::setLineColorGuiName(const QString &name)
{
  m_lineColor.setName(name);
}

float Z3DLineWithFixedWidthColorRenderer::getLineWidth() const
{
  return m_lineWidth.get();
}

void Z3DLineWithFixedWidthColorRenderer::setLineColors()
{
  m_lineColorsPrivate.clear();
  if (!m_linesPt)
    return;
  for (size_t i=0; i<m_linesPt->size(); i++) {
    m_lineColorsPrivate.push_back(m_lineColor.get());
  }
  setDataColors(&m_lineColorsPrivate);
}
