#ifndef ZINTCUBOIDCOMPOSITION_H
#define ZINTCUBOIDCOMPOSITION_H

#include "tz_cuboid_i.h"
#include <utility>
#ifdef __GLIBCXX__
#include <tr1/memory>
#else
#include <memory>
#endif

namespace FlyEm {
class ZIntCuboidComposition
{
public:
  ZIntCuboidComposition();

  enum EOperator {
    AND, OR, XOR, SINGULAR
  };

  bool hitTestF(double x, double y, double z);

  bool hitTest(int x, int y, int z);

  void setComposition(std::tr1::shared_ptr<ZIntCuboidComposition> firstComponent,
                      std::tr1::shared_ptr<ZIntCuboidComposition> secondComponent,
                      EOperator opr);
  void setSingular(int x, int y, int z, int width, int height, int depth);

private:
  std::tr1::shared_ptr<ZIntCuboidComposition> m_firstComponent;
  std::tr1::shared_ptr<ZIntCuboidComposition> m_secondComponent;
  Cuboid_I m_cuboid;
  EOperator m_operator;
};
}

#endif // ZINTCUBOIDCOMPOSITION_H
