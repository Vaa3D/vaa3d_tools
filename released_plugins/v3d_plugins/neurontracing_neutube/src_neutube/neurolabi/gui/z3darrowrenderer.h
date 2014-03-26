#ifndef Z3DARROWRENDERER_H
#define Z3DARROWRENDERER_H

#include "z3dconerenderer.h"

class Z3DArrowRenderer : public Z3DConeRenderer
{
  Q_OBJECT
public:
  explicit Z3DArrowRenderer(QObject *parent = 0);
  virtual ~Z3DArrowRenderer();

  // head length is in proportion to whole length
  void setArrowData(std::vector<glm::vec4> *tailPosAndTailRadius, std::vector<glm::vec4> *headPosAndHeadRadius,
               float headLengthProportion = 0.1);
  // head length is fixed
  void setFixedHeadLengthArrowData(std::vector<glm::vec4> *tailPosAndTailRadius, std::vector<glm::vec4> *headPosAndHeadRadius,
               float fixedHeadLength);
  void setArrowColors(std::vector<glm::vec4> *arrowColors);
  void setArrowColors(std::vector<glm::vec4> *arrowTailColors, std::vector<glm::vec4> *arrowHeadColors);
  void setArrowPickingColors(std::vector<glm::vec4> *arrowPickingColors = NULL);
  
signals:
  
public slots:

private:
  std::vector<glm::vec4> m_arrowConeBaseAndBaseRadius;
  std::vector<glm::vec4> m_arrowConeAxisAndTopRadius;
  std::vector<glm::vec4> m_arrowConeColors;
  std::vector<glm::vec4> m_arrowConePickingColors;
};

#endif // Z3DARROWRENDERER_H
