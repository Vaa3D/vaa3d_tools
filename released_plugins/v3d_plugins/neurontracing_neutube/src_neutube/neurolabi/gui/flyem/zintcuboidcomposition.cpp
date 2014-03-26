#include "zintcuboidcomposition.h"

#include <math.h>

FlyEm::ZIntCuboidComposition::ZIntCuboidComposition() : m_operator(SINGULAR)
{
  Cuboid_I_Set_S(&m_cuboid, 0, 0, 0, 0, 0, 0);
}

bool FlyEm::ZIntCuboidComposition::hitTestF(double x, double y, double z)
{
  int ix = floor(x);
  int iy = floor(y);
  int iz = floor(z);

  return hitTest(ix, iy, iz);
}

void FlyEm::ZIntCuboidComposition::setComposition(
    std::tr1::shared_ptr<ZIntCuboidComposition> firstComponent,
    std::tr1::shared_ptr<ZIntCuboidComposition> secondComponent,
    EOperator opr)
{
  m_firstComponent = firstComponent;
  m_secondComponent = secondComponent;
  m_operator = opr;
}

bool FlyEm::ZIntCuboidComposition::hitTest(int x, int y, int z)
{
  if (m_operator == SINGULAR) {
    return Cuboid_I_Hit(&m_cuboid, x, y, z);
  } else {
    if (m_firstComponent == NULL || m_secondComponent == NULL) {
      if (m_firstComponent != NULL) {
        return m_firstComponent->hitTest(x, y, z);
      } else if (m_secondComponent != NULL) {
        return m_secondComponent->hitTest(x, y, z);
      } else {
        return false;
      }
    }

    switch (m_operator) {
    case OR:
      return m_firstComponent->hitTest(x, y, z) ||
          m_secondComponent->hitTest(x, y, z);
    case AND:
      return m_firstComponent->hitTest(x, y, z) &&
          m_secondComponent->hitTest(x, y, z);
    case XOR:
      return (m_firstComponent->hitTest(x, y, z) ||
              m_secondComponent->hitTest(x, y, z)) &&
          !(m_firstComponent->hitTest(x, y, z) &&
            m_secondComponent->hitTest(x, y, z));
    default:
      break;
    }
  }

  return false;
}

void FlyEm::ZIntCuboidComposition::setSingular(
    int x, int y, int z, int width, int height, int depth)
{
  m_operator = SINGULAR;
  Cuboid_I_Set_S(&m_cuboid, x, y, z, width, height, depth);
}
