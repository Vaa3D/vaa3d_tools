#include "z3darrowrenderer.h"

Z3DArrowRenderer::Z3DArrowRenderer(QObject *parent) :
  Z3DConeRenderer(parent)
{
}

Z3DArrowRenderer::~Z3DArrowRenderer()
{
}

void Z3DArrowRenderer::setArrowData(std::vector<glm::vec4> *tailPosAndTailRadius,
                                    std::vector<glm::vec4> *headPosAndHeadRadius,
                                    float headLengthProportion)
{
  m_arrowConeBaseAndBaseRadius.clear();
  m_arrowConeAxisAndTopRadius.clear();

  for (size_t i=0; i<tailPosAndTailRadius->size(); i++) {
    glm::vec3 tail = tailPosAndTailRadius->at(i).xyz();
    glm::vec3 head = headPosAndHeadRadius->at(i).xyz();
    glm::vec3 cutPos = glm::mix(head, tail, headLengthProportion);
    m_arrowConeBaseAndBaseRadius.push_back(glm::vec4(cutPos, tailPosAndTailRadius->at(i).w));
    m_arrowConeAxisAndTopRadius.push_back(glm::vec4(tail-cutPos, tailPosAndTailRadius->at(i).w));
    m_arrowConeBaseAndBaseRadius.push_back(glm::vec4(head, 0.f));
    m_arrowConeAxisAndTopRadius.push_back(glm::vec4(cutPos-head, headPosAndHeadRadius->at(i).w));
  }

  setData(&m_arrowConeBaseAndBaseRadius, &m_arrowConeAxisAndTopRadius);
}

void Z3DArrowRenderer::setFixedHeadLengthArrowData(std::vector<glm::vec4> *tailPosAndTailRadius,
                                                   std::vector<glm::vec4> *headPosAndHeadRadius,
                                                   float fixedHeadLength)
{
  m_arrowConeBaseAndBaseRadius.clear();
  m_arrowConeAxisAndTopRadius.clear();

  for (size_t i=0; i<tailPosAndTailRadius->size(); i++) {
    glm::vec3 tail = tailPosAndTailRadius->at(i).xyz();
    glm::vec3 head = headPosAndHeadRadius->at(i).xyz();
    float totalLength = glm::length(head - tail);
    glm::vec3 cutPos = head + glm::normalize(tail-head) *
        (fixedHeadLength<totalLength ? fixedHeadLength : .5f*totalLength);
    m_arrowConeBaseAndBaseRadius.push_back(glm::vec4(cutPos, tailPosAndTailRadius->at(i).w));
    m_arrowConeAxisAndTopRadius.push_back(glm::vec4(tail-cutPos, tailPosAndTailRadius->at(i).w));
    m_arrowConeBaseAndBaseRadius.push_back(glm::vec4(head, 0.f));
    m_arrowConeAxisAndTopRadius.push_back(glm::vec4(cutPos-head, headPosAndHeadRadius->at(i).w));
  }

  setData(&m_arrowConeBaseAndBaseRadius, &m_arrowConeAxisAndTopRadius);
}

void Z3DArrowRenderer::setArrowColors(std::vector<glm::vec4> *arrowColors)
{
  m_arrowConeColors.clear();

  for (size_t i=0; i<arrowColors->size(); ++i) {
    m_arrowConeColors.push_back(arrowColors->at(i));
    m_arrowConeColors.push_back(arrowColors->at(i));
  }

  setDataColors(&m_arrowConeColors);
}

void Z3DArrowRenderer::setArrowColors(std::vector<glm::vec4> *arrowTailColors, std::vector<glm::vec4> *arrowHeadColors)
{
  m_arrowConeColors.clear();

  for (size_t i=0; i<arrowTailColors->size(); ++i) {
    m_arrowConeColors.push_back(arrowTailColors->at(i));
    m_arrowConeColors.push_back(arrowHeadColors->at(i));
  }

  setDataColors(&m_arrowConeColors);
}

void Z3DArrowRenderer::setArrowPickingColors(std::vector<glm::vec4> *arrowPickingColors)
{
  m_arrowConePickingColors.clear();

  if (!arrowPickingColors) {
    setDataPickingColors(NULL);
    return;
  }

  for (size_t i=0; i<arrowPickingColors->size(); ++i) {
    m_arrowConePickingColors.push_back(arrowPickingColors->at(i));
    m_arrowConePickingColors.push_back(arrowPickingColors->at(i));
  }

  setDataPickingColors(&m_arrowConePickingColors);
}
