#ifndef Z3DGRAPHFILTER_H
#define Z3DGRAPHFILTER_H

#include <QObject>
#include "z3dgeometryfilter.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_graph.h"
#include "z3dgraph.h"
#include "zwidgetsgroup.h"

class Z3DLineRenderer;
class Z3DConeRenderer;
class Z3DSphereRenderer;

class Z3DGraphFilter : public Z3DGeometryFilter
{
  Q_OBJECT

public:
  explicit Z3DGraphFilter();
  virtual ~Z3DGraphFilter();

  virtual void initialize();
  virtual void deinitialize();

  void prepareData();
  void setData(const ZPointNetwork &pointCloud, ZNormColorMap *colorMap = NULL);
  void setData(const Z3DGraph &graph);

  virtual void process(Z3DEye);

  virtual void render(Z3DEye eye);

  std::vector<double> boundBox();

  ZWidgetsGroup *getWidgetsGroup();

  inline bool showingArrow() { return m_showingArrow; }

public slots:
  void prepareColor();

private:
  Z3DGraph m_graph;

  Z3DLineRenderer *m_lineRenderer;
  Z3DConeRenderer *m_coneRenderer;
  Z3DConeRenderer *m_arrowRenderer;
  Z3DSphereRenderer *m_sphereRenderer;

  std::vector<glm::vec4> m_baseAndBaseRadius;
  std::vector<glm::vec4> m_axisAndTopRadius;

  std::vector<glm::vec4> m_arrowBaseAndBaseRadius;
  std::vector<glm::vec4> m_arrowAxisAndTopRadius;

  std::vector<glm::vec3> m_lines;
  std::vector<glm::vec4> m_lineColors;
  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec4> m_lineStartColors;
  std::vector<glm::vec4> m_lineEndColors;
  std::vector<glm::vec4> m_arrowStartColors;
  std::vector<glm::vec4> m_arrowEndColors;

  bool m_dataIsInvalid;
  ZIntSpanParameter m_xCut;
  ZIntSpanParameter m_yCut;
  ZIntSpanParameter m_zCut;

  ZWidgetsGroup *m_widgetsGroup;

  bool m_showingArrow;
};

#endif // Z3DGRAPHFILTER_H
