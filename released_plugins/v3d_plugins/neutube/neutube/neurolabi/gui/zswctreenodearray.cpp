#include "zswctreenodearray.h"
#include <algorithm>


ZSwcTreeNodeArray::ZSwcTreeNodeArray()
{
}

ZSwcTreeNodeArray::ZSwcTreeNodeArray(
    const std::vector<Swc_Tree_Node *> &nodeArray)
{
  resize(nodeArray.size());
  copy(nodeArray.begin(), nodeArray.end(), this->begin());
}

int ZSwcTreeNodeArray::getId(size_t index) const
{
  return SwcTreeNode::id((*this)[index]);
}

void ZSwcTreeNodeArray::sortByWeight()
{
  std::sort(begin(), end(), SwcTreeNode::lessThanWeight);
}

void ZSwcTreeNodeArray::sort(Swc_Tree_Node_Compare_B compare)
{
  std::sort(begin(), end(), compare);
}
