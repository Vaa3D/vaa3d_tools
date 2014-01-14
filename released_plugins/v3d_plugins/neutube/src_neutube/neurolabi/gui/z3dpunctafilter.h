#ifndef Z3DPUNCTAFILTER_H
#define Z3DPUNCTAFILTER_H

class ZPunctum;
class Z3DSphereRenderer;
class Z3DLineWithFixedWidthColorRenderer;

#include <QObject>
#include "z3dgeometryfilter.h"
#include "zoptionparameter.h"
#include <map>
#include <QString>
#include <QPoint>
#include <vector>
#include "z3drendererbase.h"
#include "zwidgetsgroup.h"
#include "znumericparameter.h"

class Z3DPunctaFilter : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  explicit Z3DPunctaFilter();
  virtual ~Z3DPunctaFilter();

  void setData(std::vector<ZPunctum*> *punctaList);
  void setData(QList<ZPunctum*> *punctaList);
  inline void setSelectedPuncta(std::set<ZPunctum*> *list) { m_selectedPuncta = list; }

  std::vector<double> getPunctumBound(ZPunctum* p) const;

  virtual bool isReady(Z3DEye eye) const;

  // caller should clean up this (by delete parent of this zwidgetgroup)
  ZWidgetsGroup *getWidgetsGroup();

  inline void setColorMode(const std::string &mode)
  {
    m_colorMode.select(mode.c_str());
  }

signals:
  void punctumSelected(ZPunctum*, bool append);

protected slots:
  virtual void prepareColor();
  void setClipPlanes();
  virtual void adjustWidgets();
  void changePunctaSize();
  void selectPuncta(QMouseEvent *e, int w, int h);

protected:
  void initialize();
  void deinitialize();
  virtual void process(Z3DEye);

  virtual void render(Z3DEye eye);
  virtual void renderPicking(Z3DEye eye);
  void renderSelectionBox(Z3DEye eye);

  virtual void registerPickingObjects(Z3DPickingManager *pm);
  virtual void deregisterPickingObjects(Z3DPickingManager *pm);

  virtual void prepareData();

private:
  void updateWidgetsGroup();

  // get visible data from origPunctaList put into punctaList
  void getVisibleData();

public slots:
  void updatePunctumVisibleState();

private:
  Z3DSphereRenderer *m_sphereRenderer;
  Z3DLineWithFixedWidthColorRenderer *m_boundBoxRenderer;

  ZBoolParameter m_showPuncta;
  ZOptionParameter<QString> m_colorMode;
  ZVec4Parameter m_singleColorForAllPuncta;
  //std::vector<ZVec4Parameter*> m_colorsForDifferentSource;
  std::map<QString, ZVec4Parameter*> m_sourceColorMapper;
  ZBoolParameter m_useSameSizeForAllPuncta;

  //std::map<QString, size_t> m_sourceColorMapper;   // should use unordered_map
  // puncta list used for rendering, it is a subset of m_origPunctaList. Some puncta are
  // hidden because they are unchecked from the object model. This allows us to control
  // the visibility of each single punctum.
  std::vector<ZPunctum*> m_punctaList;
  std::vector<ZPunctum*> m_registeredPunctaList;    // used for picking

  ZEventListenerParameter* m_selectPunctumEvent;
  glm::ivec2 m_startCoord;
  ZPunctum *m_pressedPunctum;
  std::set<ZPunctum*> *m_selectedPuncta;   //point to all selected puncta, managed by other class

  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_specularAndShininess;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec4> m_pointPickingColors;

  ZIntSpanParameter m_xCut;
  ZIntSpanParameter m_yCut;
  ZIntSpanParameter m_zCut;

  ZWidgetsGroup *m_widgetsGroup;
  std::vector<ZWidgetsGroup*> m_colorsForDifferentSourceWidgetsGroup;
  bool m_dataIsInvalid;

  std::vector<ZPunctum*> m_origPunctaList;
};

#endif // Z3DPUNCTAFILTER_H
