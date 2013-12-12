#ifndef ZGENERIZSwcTree_H
#define ZGENERIZSwcTree_H

#include "ztree.cpp"
#include "zswcnode.h"

class ZSwcTreeNode;
class MBuffer;
class CWeightedPointArray;

class ZGenericSwcTree : public ZTree<ZSwcNode>
{
public:
  ZGenericSwcTree();
  ZGenericSwcTree(ZTreeNode<ZSwcNode> *root);
  virtual ~ZGenericSwcTree(void);

public:
  //Return true if the SWC tree contains a soma
  bool hasSoma();

  //Search a node that has the given id
  ZTreeNode<ZSwcNode>* query(int id);
  //Read an SWC tree from a file
  bool readSwcFile(const char *filePath);
  //Export the tree to an SWC file
  bool exportSwcFile(const char *filePath);
};

#endif // ZGENERIZSwcTree_H
