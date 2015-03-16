#ifndef TREE_H
#define TREE_H
#include "graph.h"
#include <v3d_interface.h>
template<class TreeNode>
class Tree
{
public:
    TreeNode node;
    int num;
    int parent;
    QList<Tree*> child;
public:
    Tree(TreeNode node,int num,int parent,Tree* parentNode)
    {
        this->node = node;
        this->num = num;
        this->parent = parent;
        if(parent > 0)
            parentNode->child.append(this);
    }
    Tree(TreeNode node,int num,int parent)
    {
        this->node = node;
        this->num = num;
        this->parent = parent;
    }
    ~Tree()
    {
        child.clear();
    }
    QList<Tree<TreeNode> > mst(QMap<V3DLONG,Graph<TreeNode>*> &map);
};
#endif // TREE_H
