#include "zswcnodeselector.h"
#include "swctreenode.h"

using namespace std;

ZSwcNodeSelector::ZSwcNodeSelector() : m_excludingVirtual(true)
{
}

ZSwcNodeSelector::~ZSwcNodeSelector()
{

}

vector<Swc_Tree_Node*> ZSwcNodeSelector::selectFrom(const ZSwcTree &tree)
{
  tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  vector<Swc_Tree_Node*> nodeArray;

  for(Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
    if (isSelected(tn)) {
      nodeArray.push_back(tn);
    }
  }

  return nodeArray;
}

vector<Swc_Tree_Node*> ZSwcNodeSelector::antiSelectFrom(const ZSwcTree &tree)
{
  tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  vector<Swc_Tree_Node*> nodeArray;

  for(Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
    if (!isSelected(tn)) {
      if (m_excludingVirtual) {
        if (SwcTreeNode::isRegular(tn)) {
          nodeArray.push_back(tn);
        }
      } else {
        nodeArray.push_back(tn);
      }
    }
  }

  return nodeArray;
}
