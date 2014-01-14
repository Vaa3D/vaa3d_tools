#include "z3dpunctafilter.h"

#include <iostream>

#include "zpunctum.h"
#include "zrandom.h"
#include "z3dsphererenderer.h"
#include "z3dlinewithfixedwidthcolorrenderer.h"
#include "zeventlistenerparameter.h"

Z3DPunctaFilter::Z3DPunctaFilter()
  : Z3DGeometryFilter()
  , m_sphereRenderer(NULL)
  , m_boundBoxRenderer(NULL)
  , m_showPuncta("Show Puncta", true)
  , m_colorMode("Color Mode")
  , m_singleColorForAllPuncta("Puncta Color", glm::vec4(ZRandomInstance.randReal<float>(),
                                                       ZRandomInstance.randReal<float>(),
                                                       ZRandomInstance.randReal<float>(),
                                                       1.f))
  , m_useSameSizeForAllPuncta("Use Same Size", false)
  , m_pressedPunctum(NULL)
  , m_selectedPuncta(NULL)
  , m_xCut("X Cut", glm::ivec2(0,0), 0, 0)
  , m_yCut("Y Cut", glm::ivec2(0,0), 0, 0)
  , m_zCut("Z Cut", glm::ivec2(0,0), 0, 0)
  , m_widgetsGroup(NULL)
  , m_dataIsInvalid(false)
{
  m_singleColorForAllPuncta.setStyle("COLOR");
  connect(&m_singleColorForAllPuncta, SIGNAL(valueChanged()), this, SLOT(prepareColor()));

  // Color Mode
  m_colorMode.addOptions("Same Color", "Random Color", "Based on Point Source", "Original Point Color");
  m_colorMode.select("Based on Point Source");

  connect(&m_colorMode, SIGNAL(valueChanged()), this, SLOT(prepareColor()));
  connect(&m_colorMode, SIGNAL(valueChanged()), this, SLOT(adjustWidgets()));

  connect(&m_useSameSizeForAllPuncta, SIGNAL(valueChanged()), this, SLOT(changePunctaSize()));


  addParameter(m_showPuncta);
  addParameter(m_colorMode);

  addParameter(m_singleColorForAllPuncta);

  addParameter(m_useSameSizeForAllPuncta);

  m_selectPunctumEvent = new ZEventListenerParameter("Select Puncta", true, false, this);
  m_selectPunctumEvent->listenTo("select punctum", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectPunctumEvent->listenTo("select punctum", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  m_selectPunctumEvent->listenTo("append select punctum", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectPunctumEvent->listenTo("append select punctum", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonRelease);
  connect(m_selectPunctumEvent, SIGNAL(mouseEventTriggered(QMouseEvent*,int,int)), this, SLOT(selectPuncta(QMouseEvent*,int,int)));
  addEventListener(m_selectPunctumEvent);

  addParameter(m_xCut);
  addParameter(m_yCut);
  addParameter(m_zCut);
  connect(&m_xCut, SIGNAL(valueChanged()), this, SLOT(setClipPlanes()));
  connect(&m_yCut, SIGNAL(valueChanged()), this, SLOT(setClipPlanes()));
  connect(&m_zCut, SIGNAL(valueChanged()), this, SLOT(setClipPlanes()));
  adjustWidgets();
}

Z3DPunctaFilter::~Z3DPunctaFilter()
{
  //for(size_t i=0; i<m_colorsForDifferentSource.size(); i++) {
    //delete m_colorsForDifferentSource[i];
  //}
  for (std::map<QString, ZVec4Parameter*>::iterator it = m_sourceColorMapper.begin();
       it != m_sourceColorMapper.end(); ++it) {
    delete it->second;
  }
  delete m_selectPunctumEvent;
}

void Z3DPunctaFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DPunctaFilter::setData(std::vector<ZPunctum *> *punctaList)
{
  m_origPunctaList.clear();
  if (punctaList) {
    m_origPunctaList = *punctaList;
    LINFO() << getClassName() << "Read" << m_origPunctaList.size() << "puncta.";
  }
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DPunctaFilter::setData(QList<ZPunctum *> *punctaList)
{
  m_origPunctaList.clear();
  if (punctaList) {
    for (int i=0; i<punctaList->size(); i++)
      m_origPunctaList.push_back(punctaList->at(i));
    LINFO() << getClassName() << "Read" << m_origPunctaList.size() << "puncta.";
  }
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

std::vector<double> Z3DPunctaFilter::getPunctumBound(ZPunctum *p) const
{
  std::vector<double> result(6);
  double radius = p->radius() * m_rendererBase->getSizeScale();
  if (m_useSameSizeForAllPuncta.get())
    radius = 2.0 * m_rendererBase->getSizeScale();
  result[0] = p->x() * getCoordScales().x - radius;
  result[1] = p->x() * getCoordScales().x + radius;
  result[2] = p->y() * getCoordScales().y - radius;
  result[3] = p->y() * getCoordScales().y + radius;
  result[4] = p->z() * getCoordScales().z - radius;
  result[5] = p->z() * getCoordScales().z + radius;
  return result;
}

bool Z3DPunctaFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && m_showPuncta.get() && !m_origPunctaList.empty();
}

namespace {

bool compareParameterName(const ZParameter *p1, const ZParameter *p2)
{
  QString n1 = p1->getName().mid(7); // "Source "
  QString n2 = p2->getName().mid(7);
  n1.remove(n1.size()-6, 6); //" Color"
  n2.remove(n2.size()-6, 6);
  return n1.toInt() < n2.toInt();
}

}

ZWidgetsGroup *Z3DPunctaFilter::getWidgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = new ZWidgetsGroup("Puncta", NULL, 1);
    new ZWidgetsGroup(&m_showPuncta, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_stayOnTop, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_colorMode, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_singleColorForAllPuncta, m_widgetsGroup, 1);
    //for(size_t i=0; i<m_colorsForDifferentSource.size(); i++) {
      //m_colorsForDifferentSourceWidgetsGroup.push_back(new ZWidgetsGroup(m_colorsForDifferentSource[i], m_widgetsGroup, 1));
    //}
    std::vector<ZParameter*> allPunctaColorParas;
    for (std::map<QString, ZVec4Parameter*>::iterator it = m_sourceColorMapper.begin();
         it != m_sourceColorMapper.end(); ++it) {
      allPunctaColorParas.push_back(it->second);
    }
    std::sort(allPunctaColorParas.begin(), allPunctaColorParas.end(), compareParameterName);
    for (size_t i=0; i<allPunctaColorParas.size(); ++i) {
      m_colorsForDifferentSourceWidgetsGroup.push_back(new ZWidgetsGroup(allPunctaColorParas[i], m_widgetsGroup, 1));
    }

    new ZWidgetsGroup(&m_useSameSizeForAllPuncta, m_widgetsGroup, 1);

    std::vector<ZParameter*> paras = m_rendererBase->getParameters();
    for (size_t i=0; i<paras.size(); i++) {
      ZParameter *para = paras[i];
      if (para->getName() == "Z Scale")
        new ZWidgetsGroup(para, m_widgetsGroup, 2);
      else if (para->getName() == "Size Scale")
        new ZWidgetsGroup(para, m_widgetsGroup, 3);
      else if (para->getName() == "Rendering Method")
        new ZWidgetsGroup(para, m_widgetsGroup, 4);
      else if (para->getName() == "Opacity")
        new ZWidgetsGroup(para, m_widgetsGroup, 5);
      else if (para->getName() == "Calculate Material Property From Intensity")
        new ZWidgetsGroup(para, m_widgetsGroup, 5);
      else
        new ZWidgetsGroup(para, m_widgetsGroup, 7);
    }
    new ZWidgetsGroup(&m_xCut, m_widgetsGroup, 5);
    new ZWidgetsGroup(&m_yCut, m_widgetsGroup, 5);
    new ZWidgetsGroup(&m_zCut, m_widgetsGroup, 5);
    m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

void Z3DPunctaFilter::render(Z3DEye eye)
{
  if (m_punctaList.empty())
    return;
  if (!m_showPuncta.get())
    return;

  m_rendererBase->activateRenderer(m_sphereRenderer);
  m_rendererBase->render(eye);
  renderSelectionBox(eye);
}

void Z3DPunctaFilter::renderPicking(Z3DEye eye)
{
  if (!getPickingManager())
      return;
  if (m_punctaList.empty())
    return;
  if (!m_showPuncta.get())
    return;

  if (!m_pickingObjectsRegistered)
    registerPickingObjects(getPickingManager());
  m_rendererBase->activateRenderer(m_sphereRenderer);
  m_rendererBase->renderPicking(eye);
}

void Z3DPunctaFilter::renderSelectionBox(Z3DEye eye)
{
  if (m_selectedPuncta && m_selectedPuncta->size() > 0) {
    std::vector<glm::vec3> lines;

    for (std::set<ZPunctum*>::iterator it=m_selectedPuncta->begin(); it != m_selectedPuncta->end(); it++) {
      ZPunctum *selectedPunctum = *it;
      if (!selectedPunctum->isVisible())
        return;

      std::vector<double> bound = getPunctumBound(selectedPunctum);
      float xmin = bound[0];
      float xmax = bound[1];
      float ymin = bound[2];
      float ymax = bound[3];
      float zmin = bound[4];
      float zmax = bound[5];
      lines.push_back(glm::vec3(xmin, ymin, zmin));
      lines.push_back(glm::vec3(xmin, ymin, zmax));
      lines.push_back(glm::vec3(xmin, ymax, zmin));
      lines.push_back(glm::vec3(xmin, ymax, zmax));

      lines.push_back(glm::vec3(xmax, ymin, zmin));
      lines.push_back(glm::vec3(xmax, ymin, zmax));
      lines.push_back(glm::vec3(xmax, ymax, zmin));
      lines.push_back(glm::vec3(xmax, ymax, zmax));

      lines.push_back(glm::vec3(xmin, ymin, zmin));
      lines.push_back(glm::vec3(xmax, ymin, zmin));
      lines.push_back(glm::vec3(xmin, ymax, zmin));
      lines.push_back(glm::vec3(xmax, ymax, zmin));

      lines.push_back(glm::vec3(xmin, ymin, zmax));
      lines.push_back(glm::vec3(xmax, ymin, zmax));
      lines.push_back(glm::vec3(xmin, ymax, zmax));
      lines.push_back(glm::vec3(xmax, ymax, zmax));

      lines.push_back(glm::vec3(xmin, ymin, zmin));
      lines.push_back(glm::vec3(xmin, ymax, zmin));
      lines.push_back(glm::vec3(xmax, ymin, zmin));
      lines.push_back(glm::vec3(xmax, ymax, zmin));

      lines.push_back(glm::vec3(xmin, ymin, zmax));
      lines.push_back(glm::vec3(xmin, ymax, zmax));
      lines.push_back(glm::vec3(xmax, ymin, zmax));
      lines.push_back(glm::vec3(xmax, ymax, zmax));
    }
    m_rendererBase->activateRenderer(m_boundBoxRenderer);
    m_boundBoxRenderer->setData(&lines);
    m_rendererBase->render(eye);
    m_boundBoxRenderer->setData(NULL); // lines will go out of scope
  }
}

void Z3DPunctaFilter::registerPickingObjects(Z3DPickingManager *pm)
{
  if (pm && !m_pickingObjectsRegistered) {
    for (size_t i=0; i<m_punctaList.size(); i++) {
      pm->registerObject(m_punctaList[i]);
    }
    m_registeredPunctaList = m_punctaList;
    m_pointPickingColors.clear();
    for (size_t i=0; i<m_punctaList.size(); i++) {
      glm::col4 pickingColor = pm->getColorFromObject(m_punctaList[i]);
      glm::vec4 fPickingColor(pickingColor[0]/255.f, pickingColor[1]/255.f, pickingColor[2]/255.f, pickingColor[3]/255.f);
      m_pointPickingColors.push_back(fPickingColor);
    }
    m_sphereRenderer->setDataPickingColors(&m_pointPickingColors);
  }

  m_pickingObjectsRegistered = true;
}

void Z3DPunctaFilter::deregisterPickingObjects(Z3DPickingManager *pm)
{
  if (pm && m_pickingObjectsRegistered) {
    for (size_t i=0; i<m_registeredPunctaList.size(); i++) {
      pm->deregisterObject(m_registeredPunctaList[i]);
    }
    m_registeredPunctaList.clear();
  }

  m_pickingObjectsRegistered = false;
}

void Z3DPunctaFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  deregisterPickingObjects(getPickingManager());

  // convert puncta to format that glsl can use
  m_specularAndShininess.clear();
  m_pointAndRadius.clear();
  //m_sourceColorMapper.clear();
  int xMin = std::numeric_limits<int>::max();
  int xMax = std::numeric_limits<int>::min();
  int yMin = std::numeric_limits<int>::max();
  int yMax = std::numeric_limits<int>::min();
  int zMin = std::numeric_limits<int>::max();
  int zMax = std::numeric_limits<int>::min();
  for (size_t i=0; i<m_punctaList.size(); i++) {
    if (m_useSameSizeForAllPuncta.get())
      m_pointAndRadius.push_back(glm::vec4(m_punctaList[i]->x(), m_punctaList[i]->y(), m_punctaList[i]->z(), 2.f));
    else
      m_pointAndRadius.push_back(glm::vec4(m_punctaList[i]->x(), m_punctaList[i]->y(), m_punctaList[i]->z(), m_punctaList[i]->radius()));
    if (m_punctaList[i]->x() > xMax)
      xMax = static_cast<int>(std::ceil(m_punctaList[i]->x()));
    if (m_punctaList[i]->x() < xMin)
      xMin = static_cast<int>(std::floor(m_punctaList[i]->x()));
    if (m_punctaList[i]->y() > yMax)
      yMax = static_cast<int>(std::ceil(m_punctaList[i]->y()));
    if (m_punctaList[i]->y() < yMin)
      yMin = static_cast<int>(std::floor(m_punctaList[i]->y()));
    if (m_punctaList[i]->z() > zMax)
      zMax = static_cast<int>(std::ceil(m_punctaList[i]->z()));
    if (m_punctaList[i]->z() < zMin)
      zMin = static_cast<int>(std::floor(m_punctaList[i]->z()));
    m_specularAndShininess.push_back(glm::vec4(m_punctaList[i]->maxIntensity()/255.f,
                                               m_punctaList[i]->maxIntensity()/255.f,
                                               m_punctaList[i]->maxIntensity()/255.f,
                                               m_punctaList[i]->maxIntensity()/2.f));
  }
  //for (size_t i=0; i<m_origPunctaList.size(); ++i)
    //m_sourceColorMapper.insert(std::pair<QString, size_t>(m_origPunctaList[i]->source(), 0));

  m_xCut.setRange(xMin, xMax);
  m_xCut.set(glm::ivec2(xMin, xMax));
  m_yCut.setRange(yMin, yMax);
  m_yCut.set(glm::ivec2(yMin, yMax));
  m_zCut.setRange(zMin, zMax);
  m_zCut.set(glm::ivec2(zMin, zMax));

  //  std::map<QString,size_t>::iterator it;
  //  size_t index = 0;
  //  size_t numOfPrevColor = m_colorsForDifferentSource.size();
  //  for (it = m_sourceColorMapper.begin(); it != m_sourceColorMapper.end(); it++) {
  //    m_sourceColorMapper[it->first] = index++;
  //    if (index > numOfPrevColor) {
  //      QString guiname = QString("Source %1 Color").arg(index);
  //      m_colorsForDifferentSource.push_back(new ZVec4Parameter(guiname, glm::vec4(
  //                                                                ZRandomInstance.randReal<float>(),
  //                                                                ZRandomInstance.randReal<float>(),
  //                                                                ZRandomInstance.randReal<float>(),
  //                                                                1.f)));
  //      m_colorsForDifferentSource[index-1]->setStyle("COLOR");
  //      connect(m_colorsForDifferentSource[index-1], SIGNAL(valueChanged()), this, SLOT(prepareColor()));
  //    }
  //  }
  //  if (m_widgetsGroup) {
  //    for (size_t i=0; i<m_colorsForDifferentSourceWidgetsGroup.size(); i++) {
  //      delete m_colorsForDifferentSourceWidgetsGroup[i];
  //    }
  //    m_colorsForDifferentSourceWidgetsGroup.clear();
  //  }
  //  if (numOfPrevColor < index) {
  //    for (size_t i=numOfPrevColor; i<m_colorsForDifferentSource.size(); i++) {
  //      addParameter(m_colorsForDifferentSource[i]);
  //    }
  //  } else if (numOfPrevColor > index) {
  //    for (size_t i=index; i<m_colorsForDifferentSource.size(); i++) {
  //      removeParameter(m_colorsForDifferentSource[i]);
  //      delete m_colorsForDifferentSource[i];
  //    }
  //    m_colorsForDifferentSource.resize(index);
  //  }

  bool needUpdateWidget = false;
  QList<QString> allSources;
  for (size_t i=0; i<m_origPunctaList.size(); ++i) {
    int idx = allSources.indexOf(m_origPunctaList[i]->source());
    if (idx == -1) {
      allSources.push_back(m_origPunctaList[i]->source());
      idx = allSources.size() - 1;
    }
    QString guiname = QString("Source %1 Color").arg(idx + 1);
    if (m_sourceColorMapper.find(m_origPunctaList[i]->source()) == m_sourceColorMapper.end()) {
      m_sourceColorMapper[m_origPunctaList[i]->source()] =
          new ZVec4Parameter(guiname, glm::vec4(ZRandomInstance.randReal<float>(),
                                                ZRandomInstance.randReal<float>(),
                                                ZRandomInstance.randReal<float>(),
                                                1.f));
      m_sourceColorMapper[m_origPunctaList[i]->source()]->setStyle("COLOR");
      connect(m_sourceColorMapper[m_origPunctaList[i]->source()], SIGNAL(valueChanged()),
          this, SLOT(prepareColor()));
      addParameter(m_sourceColorMapper[m_origPunctaList[i]->source()]);
      needUpdateWidget = true;
    } else {
      m_sourceColorMapper[m_origPunctaList[i]->source()]->setName(guiname);
    }
  }
  // remove colors for not exist puncta source
  std::map<QString, ZVec4Parameter*>::iterator it = m_sourceColorMapper.begin();
  while (it != m_sourceColorMapper.end()) {
    if (!allSources.contains(it->first)) {
      std::map<QString, ZVec4Parameter*>::iterator itCopy = it;
      ++it;
      removeParameter(itCopy->second);
      delete itCopy->second;
      m_sourceColorMapper.erase(itCopy);
      needUpdateWidget = true;
    } else
      ++it;
  }
  if (needUpdateWidget)
    updateWidgetsGroup();

  m_sphereRenderer->setData(&m_pointAndRadius, &m_specularAndShininess);
  prepareColor();
  adjustWidgets();
  //if (numOfPrevColor != index)   // number of puncta source changed
    //updateWidgetsGroup();
  m_dataIsInvalid = false;
}

void Z3DPunctaFilter::prepareColor()
{
  m_pointColors.clear();

  if (m_colorMode.isSelected("Original Point Color")) {
    for (size_t i=0; i<m_punctaList.size(); i++) {
      glm::vec4 color(m_punctaList[i]->color().redF(), m_punctaList[i]->color().greenF(), m_punctaList[i]->color().blueF(), m_punctaList[i]->color().alphaF());
      m_pointColors.push_back(color);
    }
  } else if (m_colorMode.isSelected("Based on Point Source")) {
    for (size_t i=0; i<m_punctaList.size(); i++) {
#ifdef _DEBUG_
      //std::cout << m_punctaList[i]->source().toStdString() << std::endl;
#endif
      glm::vec4 color = m_sourceColorMapper[m_punctaList[i]->source()]->get();
      m_pointColors.push_back(color);
    }
  } else if (m_colorMode.isSelected("Random Color")) {
    for (size_t i=0; i<m_punctaList.size(); i++) {
      glm::vec4 color(ZRandomInstance.randReal<float>(), ZRandomInstance.randReal<float>(), ZRandomInstance.randReal<float>(), 1.0f);
      m_pointColors.push_back(color);
    }
  } else if (m_colorMode.isSelected("Same Color")) {
    for (size_t i=0; i<m_punctaList.size(); i++) {
      m_pointColors.push_back(m_singleColorForAllPuncta.get());
    }
  }

  m_sphereRenderer->setDataColors(&m_pointColors);
}

void Z3DPunctaFilter::setClipPlanes()
{
  std::vector<glm::dvec4> clipPlanes;
  if (m_xCut.lowerValue() != m_xCut.minimum())
    clipPlanes.push_back(glm::dvec4(1., 0., 0., -m_xCut.lowerValue()));
  if (m_xCut.upperValue() != m_xCut.maximum())
    clipPlanes.push_back(glm::dvec4(-1., 0., 0., m_xCut.upperValue()));
  if (m_yCut.lowerValue() != m_yCut.minimum())
    clipPlanes.push_back(glm::dvec4(0., 1., 0., -m_yCut.lowerValue()));
  if (m_yCut.upperValue() != m_yCut.maximum())
    clipPlanes.push_back(glm::dvec4(0., -1., 0., m_yCut.upperValue()));
  if (m_zCut.lowerValue() != m_zCut.minimum())
    clipPlanes.push_back(glm::dvec4(0., 0., 1., -m_zCut.lowerValue()));
  if (m_zCut.upperValue() != m_zCut.maximum())
    clipPlanes.push_back(glm::dvec4(0., 0., -1., m_zCut.upperValue()));
  m_rendererBase->setClipPlanes(&clipPlanes);
}

void Z3DPunctaFilter::adjustWidgets()
{
  //  for (size_t i=0; i<m_colorsForDifferentSource.size(); i++) {
  //    if (m_colorMode.isSelected("Based on Point Source"))
  //      m_colorsForDifferentSource[i]->setVisible(true);
  //    else
  //      m_colorsForDifferentSource[i]->setVisible(false);
  //  }
  for (std::map<QString, ZVec4Parameter*>::iterator it = m_sourceColorMapper.begin();
       it != m_sourceColorMapper.end(); ++it) {
    if (m_colorMode.isSelected("Based on Point Source"))
      it->second->setVisible(true);
    else
      it->second->setVisible(false);
  }
  if (m_colorMode.isSelected("Same Color"))
    m_singleColorForAllPuncta.setVisible(true);
  else
    m_singleColorForAllPuncta.setVisible(false);
}

void Z3DPunctaFilter::selectPuncta(QMouseEvent *e, int, int h)
{
  if (m_punctaList.empty())
    return;
  if (!getPickingManager())
    return;

  e->ignore();
  // Mouse button pressend
  // can not accept the event in button press, because we don't know if it is a selection or interaction
  if (e->type() == QEvent::MouseButtonPress) {
    m_startCoord.x = e->x();
    m_startCoord.y = e->y();
    const void* obj = getPickingManager()->getObjectAtPos(glm::ivec2(e->x(), h - e->y()));
    if (obj == NULL) {
      return;
    }

    // Check if any point was selected...
    for (std::vector<ZPunctum*>::iterator it=m_punctaList.begin(); it!=m_punctaList.end(); ++it)
      if (*it == obj) {
        m_pressedPunctum = *it;
        break;
      }
    return;
  }

  if (e->type() == QEvent::MouseButtonRelease) {
    if (std::abs(e->x() - m_startCoord.x) < 2 && std::abs(m_startCoord.y - e->y()) < 2) {
      if (e->modifiers() == Qt::ControlModifier)
        emit punctumSelected(m_pressedPunctum, true);
      else
        emit punctumSelected(m_pressedPunctum, false);
      if (m_pressedPunctum)
        e->accept();
    }
    m_pressedPunctum = NULL;
  }
}

void Z3DPunctaFilter::updateWidgetsGroup()
{
  if (m_widgetsGroup) {
    for (size_t i=0; i<m_colorsForDifferentSourceWidgetsGroup.size(); i++) {
      delete m_colorsForDifferentSourceWidgetsGroup[i];
    }
    m_colorsForDifferentSourceWidgetsGroup.clear();

    //    for(size_t i=0; i<m_colorsForDifferentSource.size(); i++) {
    //      m_colorsForDifferentSourceWidgetsGroup.push_back(new ZWidgetsGroup(m_colorsForDifferentSource[i], m_widgetsGroup, 1));
    //    }
    std::vector<ZParameter*> allPunctaColorParas;
    for (std::map<QString, ZVec4Parameter*>::iterator it = m_sourceColorMapper.begin();
         it != m_sourceColorMapper.end(); ++it) {
      allPunctaColorParas.push_back(it->second);
    }
    std::sort(allPunctaColorParas.begin(), allPunctaColorParas.end(), compareParameterName);
    for (size_t i=0; i<allPunctaColorParas.size(); ++i) {
      m_colorsForDifferentSourceWidgetsGroup.push_back(new ZWidgetsGroup(allPunctaColorParas[i], m_widgetsGroup, 1));
    }
    m_widgetsGroup->emitWidgetsGroupChangedSignal();
  }
}

void Z3DPunctaFilter::getVisibleData()
{
  m_punctaList.clear();
  for (size_t i=0; i<m_origPunctaList.size(); ++i) {
    if (m_origPunctaList[i]->isVisible())
      m_punctaList.push_back(m_origPunctaList[i]);
  }
}

void Z3DPunctaFilter::updatePunctumVisibleState()
{
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DPunctaFilter::changePunctaSize()
{
  for (size_t i=0; i<m_pointAndRadius.size(); i++) {
    if (m_useSameSizeForAllPuncta.get())
      m_pointAndRadius.at(i).w = 2.f;
    else
      m_pointAndRadius.at(i).w = m_punctaList[i]->radius();
  }
  m_sphereRenderer->setData(&m_pointAndRadius, &m_specularAndShininess);
}

void Z3DPunctaFilter::initialize()
{
  Z3DGeometryFilter::initialize();
  m_sphereRenderer = new Z3DSphereRenderer();
  m_rendererBase->addRenderer(m_sphereRenderer);
  m_boundBoxRenderer = new Z3DLineWithFixedWidthColorRenderer();
  m_boundBoxRenderer->setUseDisplayList(false);
  m_boundBoxRenderer->setRespectRendererBaseCoordScales(false);
  m_boundBoxRenderer->setLineColorGuiName("Selection BoundBox Line Color");
  m_boundBoxRenderer->setLineWidthGuiName("Selection BoundBox Line Width");
  m_rendererBase->addRenderer(m_boundBoxRenderer);

  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    //connect(paras[i], SIGNAL(valueChanged()), this, SLOT(invalidateResult()));
    addParameter(paras[i]);
  }
}

void Z3DPunctaFilter::deinitialize()
{
  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    //paras[i]->disconnect(this);
    removeParameter(paras[i]);
  }
  Z3DGeometryFilter::deinitialize();
}

