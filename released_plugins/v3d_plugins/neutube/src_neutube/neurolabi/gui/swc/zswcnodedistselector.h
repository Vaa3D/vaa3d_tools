#ifndef ZSWCNODEDISTSELECTOR_H
#define ZSWCNODEDISTSELECTOR_H

#include <vector>
#include "zswctreenodearray.h"

class ZSwcTree;

/*!
 * \brief A class of selected a set of nodes from a tree based on some criteria
 *
 * Still a prototype.
 */
class ZSwcNodeDistSelector
{
public:
  ZSwcNodeDistSelector();

  ZSwcTreeNodeArray select(const ZSwcTree &tree) const;

  inline void setMinDistance(double dist) {
    m_minDistance = dist;
  }

private:
  double m_minDistance;
};

#endif // ZSWCNODESELECTOR_H
