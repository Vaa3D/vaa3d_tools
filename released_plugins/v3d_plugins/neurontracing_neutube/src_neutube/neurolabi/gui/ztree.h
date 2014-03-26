#ifndef ZTREE_H
#define ZTREE_H

#include <vector>
#include <fstream>
#include "ztreenode.h"
#include "ztreeiterator.h"

template <typename T>
class ZTree
{
public:
  ZTree(void);
  ZTree(ZTreeNode<T> *root);
  virtual ~ZTree(void);

  inline ZTreeNode<T>* getRoot() { return m_root; }
  inline const ZTreeNode<T>* getRoot() const { return m_root; }
  void setRoot(ZTreeNode<T>* root, bool clearOld = true);

  //Get the first regular root of <tree>.
  ZTreeNode<T>* getRegularRoot();

  bool isEmpty(bool virtualCheck = true);
  void clear();

  //Get the number of trees.
  int treeNumber();

  //Clean the root of the tree. Only the largest tree will be kept.
  void cleanRoot();

  //Merge all nodes that are close to each other (within
  int mergeCloseNode(double threshold);

  //Merge a tree. After merging, all regular trees
  //in <prey> are moved to the tree with the regular structures unchanged.
  void merge(ZTree<T> *prey);


  //Get the number of leaves of <tree>.
  int leafNumber();

  //Get the number of continuous branches of the tree.
  int branchNumber();

  //Remove all virtual nodes except the root in a <tree>
  //by merging to their parents. See sm037 for more details.
  //
  void regularize();

  //Make the tree canonical. A canonical tree is the
  //simplest representation of the set of logically identical trees.
  void canonicalize();


  //Swc_Tree_Cut_Node() assumes <tn> is a <node> in a <tree> and cuts it off. All
  //descendents of <tn> are put under the vitual root of <tree>. The memory of
  //<tn> will be freed.
  void cutNode(ZTreeNode<T> *tn);


  void resortId();

protected:
  ZTreeNode<T> *m_root;         /**< root of the tree */
};

template<typename T>
class ZTreeIterator
{
public:
  /* Options for iterating. */
  enum EIteratorOption{
    DEPTH_FIRST,  //depth first search
    BREADTH_FIRST //breadth first search
  };

  enum EIteratorFilter {
    NO_FILTER, LEAF, BRANCH_POINT, REGULAR, CONTIUATION
  };

  ZTreeIterator(const ZTree<T> &tree,
                EIteratorOption option = DEPTH_FIRST);
  void update(const ZTree<T> &tree,
              EIteratorOption option = DEPTH_FIRST);
  bool hasNext();
  T &next();
  ZTreeNode<T>* nextNode();
  inline size_t size() const { return m_nodeArray.size(); }
  inline void reset() { m_currentIndex = 0; }
  void reverse();

private:
  bool passedFilter(const ZTreeNode<T> *node) const;
  void tryAppendingNode(const ZTreeNode<T> *node);

private:
  std::vector<ZTreeNode<T>*> m_nodeArray;
  size_t m_currentIndex;
  EIteratorFilter m_filter;
};

#include "ztree.cpp"





#endif // ZTREE_H
