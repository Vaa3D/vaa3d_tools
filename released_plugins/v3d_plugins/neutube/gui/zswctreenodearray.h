#ifndef ZSWCTREENODEARRAY_H
#define ZSWCTREENODEARRAY_H

#include <vector>
#include "tz_swc_tree.h"
#include "swctreenode.h"

class ZSwcTreeNodeArray : public std::vector<Swc_Tree_Node*>
{
public:
  ZSwcTreeNodeArray();
  ZSwcTreeNodeArray(const std::vector<Swc_Tree_Node *> &nodeArray);

public:
  /*!
   * \brief Get the ID of an element
   */
  int getId(size_t index) const;

  /*!
   * \brief Sort the node by its weight
   */
  void sortByWeight();

  void sort(Swc_Tree_Node_Compare_B compare);

};

#endif // ZSWCTREENODEARRAY_H
