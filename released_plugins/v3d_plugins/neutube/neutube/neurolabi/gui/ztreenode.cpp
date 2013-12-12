#include "ztreenode.h"

template<typename T>
ZTreeNode<T>::ZTreeNode() : m_weight(0.0), m_parent(NULL), m_firstChild(NULL),
  m_nextSibling(NULL)
{
  m_index = 0;
  m_label = 0;
  m_weight = 0;
}

template<typename T>
ZTreeNode<T>::ZTreeNode(const T &data) : m_weight(0.0), m_parent(NULL), m_firstChild(NULL),
  m_nextSibling(NULL)
{
  m_index = 0;
  m_label = 0;
  m_weight = 0;
  m_data = data;
}

template<typename T>
ZTreeNode<T>::ZTreeNode(const ZTreeNode<T> &node)
{
  m_weight = node.m_weight;
  m_featureVector = node.m_featureVector;
  m_parent = NULL;
  m_firstChild = NULL;
  m_nextSibling = NULL;
  m_index = node.m_index;
  m_label = node.m_label;
  m_data = node.m_data;
}

template<typename T>
ZTreeNode<T>::~ZTreeNode(void)
{
}

template<typename T>
ZTreeNode<T>* ZTreeNode<T>::lastChild() const
{
  ZTreeNode<T> *child = m_firstChild;

  if (child != NULL) {
    while (child->nextSibling() != NULL) {
      child = child->nextSibling();
    }
  }

  return child;
}

template<typename T>
bool ZTreeNode<T>::isChildOf(const ZTreeNode<T> *parent) const
{
  bool found = false;
  ZTreeNode<T> *tn = parent->firstChild();
  while (tn != NULL) {
    if (tn == this) {
      found = true;
      break;
    }
    tn = tn->nextSibling();
  }

  return found;
}

template<typename T>
int ZTreeNode<T>::childNumber() const
{
  int n = 0;
  ZTreeNode<T> *child = m_firstChild;
  while (child != NULL) {
    n++;
    child = child->nextSibling();
  }

  return n;
}

template<typename T>
ZTreeNode<T>* ZTreeNode<T>::previousSibling() const
{
  ZTreeNode<T> *sibling = NULL;

  if ((parent() != NULL) && (parent()->firstChild() != this)) {
    sibling = parent()->firstChild();
    while ((sibling != NULL) && (sibling->nextSibling() != this)) {
      sibling = sibling->nextSibling();
    }
  }

  return sibling;
}

template<typename T>
bool ZTreeNode<T>::isRoot(
    bool virtualCheck  //Perform virtual check or not
    ) const
{
  bool bIsRoot = true;

  ZTreeNode<T> *p = parent();

  while (p != NULL) {
    if (virtualCheck) {
      if (p->isRegular(virtualCheck)) {
        bIsRoot = false;
        break;
      }
    }
    p = p->parent();
  }

  return bIsRoot;
}

template<typename T>
bool ZTreeNode<T>::isLastChild() const
{
  bool bIsLastChild = false;

  if (m_parent != NULL) {
    if (m_nextSibling == NULL) {
      bIsLastChild = true;
    }
  }

  return bIsLastChild;
}

template<typename T>
bool ZTreeNode<T>::isLeaf(bool virtualCheck) const
{
  bool bIsLeaf = false;

  if (!virtualCheck || isRegular()) {
    if (firstChild() == NULL) {
      if (isRoot(virtualCheck) == false) {
        bIsLeaf = true;
      }
    }
  }

  return bIsLeaf;
}

template<typename T>
bool ZTreeNode<T>::isBranchPoint(bool virtualCheck) const
{
  bool bIsBranchPoint = false;

  if (isRegular(virtualCheck)) {
    if (firstChild() != NULL) {
      if (firstChild()->nextSibling() != NULL) {
        bIsBranchPoint = true;
      }
    }
  }

  return bIsBranchPoint;
}

template<typename T>
bool ZTreeNode<T>::isContinuation(bool virtualCheck) const
{
  return (isRegular(virtualCheck) && !isRoot(virtualCheck) &&
          !isLeaf(virtualCheck) && !isBranchPoint(virtualCheck));
}

template<typename T>
bool ZTreeNode<T>::isSpur(bool virtualCheck) const
{
  bool bIsSpur = false;

  if (isLeaf(virtualCheck)) {
    bIsSpur = parent()->isBranchPoint(virtualCheck);
  }

  return bIsSpur;
}

template<typename T>
bool ZTreeNode<T>::isSibling(const ZTreeNode<T> *node) const
{
  bool bIsSibling = false;

  if (node != NULL) {
    bIsSibling = (this->parent() == node->parent());
  }

  return bIsSibling;
}

template<typename T>
ZTreeNode<T>* ZTreeNode<T>::addChild(
    ZTreeNode<T> *child  //The new child to be added
    )
{
  ZTreeNode<T> *tmp;

  if (child != NULL) {
    child->detachParent();
    child->setParent(this, false);

    if ((tmp = lastChild()) != NULL) {
      tmp->setNextSibling(child, false);
    } else {
      setFirstChild(child, false);
    }
  }

  return child;
}

template<typename T>
ZTreeNode<T>* ZTreeNode<T>::addChild(const T &data)
{
  ZTreeNode<T> *child = new ZTreeNode<T>(data);

  return addChild(child);
}

template<typename T>
void ZTreeNode<T>::removeChild(
    ZTreeNode<T> *child  //The child to be removed
    )
{
  if (child != NULL) {
    bool succ = false;

    if (m_firstChild == child) {
      setFirstChild(child->nextSibling());
      succ = true;
    } else {
      ZTreeNode<T> *sibling = firstChild();
      while ((sibling != NULL) && (sibling->nextSibling() != child)) {
        sibling = sibling->nextSibling();
      }

      if ((sibling != NULL) && (sibling->nextSibling() == child)) {
        sibling->setNextSibling(child->nextSibling());
        succ = true;
      }
    }

    if (succ) {
      child->setParent(NULL, false);
      child->setNextSibling(NULL, false);
    }
  }
}

template<typename T>
ZTreeNode<T>* ZTreeNode<T>::detachParent()
{
  ZTreeNode<T> *p = NULL;

  if (this->parent() != NULL) {
    p = this->parent();
    p->removeChild(this);
  }

  return p;
}

template<typename T>
void ZTreeNode<T>::setParent(ZTreeNode<T> *p, bool updatingConsistency)
{
  if (p != parent()) {
    if (updatingConsistency) {
      if (p == NULL) {
        ZTreeNode<T> *oldParent = parent();
        if (oldParent != NULL) {
          if (oldParent->firstChild() == this) {
            oldParent->setFirstChild(nextSibling(), false);
          } else {
            previousSibling()->setNextSibling(nextSibling(), false);
          }
          m_nextSibling = NULL;
          m_parent = NULL;
        }
      } else {
        p->addChild(this);
      }
    } else {
      m_parent = p;
    }
  }
}

template <typename T>
void ZTreeNode<T>::setFirstChild(ZTreeNode<T> *child, bool updatingConsistency)
{
  if (child != firstChild()) {
    if (updatingConsistency) {
      if (child == NULL) {
        firstChild()->detachParent();
      } else if (firstChild() == NULL) {
        child->setParent(this);
      } else {
        child->setParent(this, false);
        child->setNextSibling(firstChild(), false);
        setFirstChild(child, false);
      }
    } else {
      m_firstChild = child;
    }
  }
}

template <typename T>
void ZTreeNode<T>::setNextSibling(ZTreeNode<T> *sibling, bool updatingConsistency)
{
  if (sibling != nextSibling()) {
    if (updatingConsistency) {
      if (!isRoot(false)) {
        sibling->detachParent();
        sibling->setParent(parent(), false);
        sibling->setNextSibling(nextSibling(), false);
        setNextSibling(sibling, false);
      }
    } else {
      m_nextSibling = sibling;
    }
  }
}

template<typename T>
void ZTreeNode<T>::replaceChild(ZTreeNode<T> *oldChild, ZTreeNode<T> *newChild)
{
  if ((oldChild != NULL) && (newChild != NULL)) {
    if ((oldChild->parent() != NULL) && (oldChild != newChild)) {
      ZTreeNode<T> *p = oldChild->parent();
      ZTreeNode<T> *prevSibling = oldChild->previousSibling();
      detachParent(newChild);
      if (prevSibling != NULL) {
        prevSibling->setNextSibling(newChild, false);
      } else {
        p->setFirstChild(newChild, false);
      }
      newChild->setNextSibling(oldChild->nextSibling(), false);
      newChild->setParent(oldChild->parent(), false);
      oldChild->setParent(NULL, false);
      oldChild->setNextSibling(NULL, false);
    }
  }
}

template<typename T>
void ZTreeNode<T>::insert(ZTreeNode<T> *node)
{
  if (parent() != NULL) {
    if (parent()->firstChild() == this) {
      parent()->firstChild() = node;
    } else {
      previousSibling()->nextSibling() = node;
    }
  }
  node->setNextSibling(nextSibling(), false);
  node->setParent(parent(), false);
  setNextSibling(node->firstChild(), false);
  node->setFirstChild(this, false);
}

template<typename T>
void ZTreeNode<T>::becomeFirstChild()
{
  if (parent() != NULL) {
    if (this != parent()->firstChild()) {
      ZTreeNode<T> *p = parent();
      detachParent();
      setNextSibling(p->firstChild(), false);
      setParent(p, false);
      p->setFirstChild(this, false);
    }
  }
}

template<typename T>
void ZTreeNode<T>::mergeSibling(ZTreeNode<T> *node)
{
  if ((parent() == node->parent()) && (parent() != NULL)) {
    node->setParent(this);
    node->mergeToParent();
  }
}

template<typename T>
ZTreeNode<T>* ZTreeNode<T>::mergeToParent()
{
  ZTreeNode<T> *p = parent();

  if (p != NULL) {
    if (firstChild() != NULL) { /* tn has children */
      /* link all its children with its parent */
      ZTreeNode<T> *child = firstChild();
      while (child != NULL) {
        child->setParent(parent(), false);
        if (child->nextSibling() == NULL) {
          /* link the last child of <tn> with the next sibling of <tn> */
          child->setNextSibling(nextSibling(), false);
          child = NULL;
        } else {
          child = child->nextSibling();
        }
      }
    }

    if (p->firstChild() == this) { /* the node is the first child */
      if (firstChild() != NULL) { /* the node has children */
        p->setFirstChild(firstChild(), false);
      } else { /* tn doesn't have children */
        p->setFirstChild(nextSibling(), false);
      }
    } else { /* tn isn't the first child */
      ZTreeNode<T> *sibling = previousSibling();
      if (firstChild() != NULL) { /* tn has children */
        sibling->setNextSibling(firstChild(), false);
      } else {
        sibling->setNextSibling(nextSibling(), false);
      }
    }
  }

  return p;
}

template <typename T>
void ZTreeNode<T>::becomeRoot(bool virtualCheck)
{
  if (!isRoot(virtualCheck)) {
    double weight[2];

    ZTreeNode<T> *buffer1 = this;
    ZTreeNode<T> *buffer2 = buffer1->parent();
    ZTreeNode<T> *buffer3 = NULL;

    buffer1->detachParent();

    weight[0] = buffer1->weight();
    while (buffer1->isRegular(virtualCheck)){
      if (buffer2 != NULL) {
        if (buffer2->isRegular(virtualCheck)) {
          if (buffer2->isRegular(virtualCheck)) {
            weight[1] = buffer2->weight();
            buffer3 = buffer2->parent();
            buffer2->setWeight(weight[0]);
            weight[0] = weight[1];
            buffer1->addChild(buffer2);
          }
        }
      }
      buffer1 = buffer2;
      buffer2 = buffer3;
      if (buffer1 == NULL) {
        break;
      }
    }

    setParent(buffer1);
  }
}

template <typename T>
double ZTreeNode<T>::getBacktraceWeight(int n, bool virtualCheck) const
{
  double w = weight();
  ZTreeNode<T> *tn = this;

  while (n != 0) {
    if (parent() != NULL) {
      tn = tn->parent();
      w += tn->weight();
    } else {
      break;
    }

    if (tn->isBranchPoint(virtualCheck)) {
      n--;
    }
  }

  if (tn->isBranchPoint(virtualCheck)) {
    /* do not include the weight of the last branch point */
    w -= tn->weight();
  }

  return w;
}

template <typename T>
void ZTreeNode<T>::labelBranch(int label, bool virtualCheck)
{
  setLabel(label);

  if (!isBranchPoint(virtualCheck)) {
    ZTreeNode<T> *current = parent();

    while (current->isContinuation(virtualCheck)) {
      current->setLabel(label);
      current = current->parent();
    }

    if (current != NULL) {
      current->setLabel(label);
    }

    ZTreeNode<T> *child = firstChild();
    while (child != NULL) {
      current = child;
      while (current->isContinuation()) {
        current->setLabel(label);
        current = current->firstChild();
      }
      if (current != NULL) {
        current->setLabel(label);
      }
      child = child->nextSibling();
    }
  }
}

template <typename T>
int ZTreeNode<T>::parentId() const
{
  int id = -1;

  if (parent() != NULL) {
    id = parent()->id();
  }

  return id;
}

template <typename T>
ZTreeNode<T>* ZTreeNode<T>::getNextAt(int index) const
{
  ZTreeNode<T> *next = const_cast<ZTreeNode<T>*>(this);

  while (index > 0) {
    if (next == NULL) {
      break;
    }
    next = next->getNext();
    --index;
  }

  return next;
}
