#include "ztree.h"
#include <queue>

template <typename T>
ZTree<T>::ZTree(void)
{
  m_root = NULL;
}

template <typename T>
ZTree<T>::ZTree(ZTreeNode<T> *root)
{
  m_root = root;
}

template <typename T>
ZTree<T>::~ZTree(void)
{
  clear();
}


template <typename T>
void ZTree<T>::clear()
{
  ZTreeIterator<T> iterator(*this);
  while (iterator.hasNext()) {
    ZTreeNode<T> *node = iterator.nextNode();
    delete node;
  }

  /*
  updateIterator(ITERATOR_DEPTH_FIRST, false);
  for (ZTreeNode<T> *node = begin(); node != end(); node = next()) {
    delete node;
  }

  m_root = NULL;
  m_iterator = NULL;
  m_begin = NULL;
  */
}
#if 0
template <typename T>
int ZTree<T>::updateIterator(
    EIteratorOption option,  //Iterator option
    bool indexing                //Create indices for the nodes or not
    )
{
  if (m_root == NULL) {
    m_iterator = NULL;
    m_begin = NULL;
    return 0;
  }

  ZTreeNode<T> *tn = NULL;
  int count = 1;

  switch(option) {
  case ITERATOR_NO_UPDATE:
    if (indexing) {
      count = 0;
      tn = m_begin;
      while (tn != NULL) {
        tn->setIndex(count);
        count++;
        tn = tn->getNext();
      }
    } else {
      count = -1;
    }
    break;
  case ITERATOR_DEPTH_FIRST:
  {
    m_begin = m_root;
    tn = m_begin;
    if (indexing) {
      tn->setIndex(0);
    }
    while (tn != NULL) {
      if (tn->firstChild() != NULL) {
        tn->setNext(tn->firstChild());
      } else {
        if (tn->nextSibling() != NULL) {
          tn->setNext(tn->nextSibling());
        } else {
          ZTreeNode<T> *parent_tn = tn->parent();
          while (parent_tn != NULL) {
            if (parent_tn->nextSibling() != NULL) {
              tn->setNext(parent_tn->nextSibling());
              break;
            } else {
              parent_tn = parent_tn->parent();
            }
          }
          if (parent_tn == NULL) {
            tn->setNext(NULL);
          }
        }
      }

      if (tn->getNext() != NULL) {
        if (indexing) {
          tn->getNext()->setIndex(tn->index() + 1);
        }
        count++;
      }
      tn = tn->getNext();
    }
  }
    break;
  case ITERATOR_BREADTH_FIRST:
  {
    m_begin = m_root;
    tn = m_begin;
    if (indexing) {
      tn->setIndex(0);
    }
    ZTreeNode<T> *pointer = tn;
    pointer->setNext(NULL);
    while (pointer != NULL) {
      ZTreeNode<T> *child = pointer->firstChild();
      while (child != NULL) {
        tn->setNext(child);
        if (indexing) {
          tn->getNext()->setIndex(tn->index() + 1);
        }
        count++;
        tn = tn->getNext();
        if (tn != NULL) {
          tn->setNext(NULL);
        }
        child = child->nextSibling();
      }
      pointer = pointer->getNext();
    }
  }
    break;
  case ITERATOR_REVERSE:
  {
    tn = m_begin;

    if (tn == NULL) { /* no iteration available*/
      return -1;
    }

    ZTreeNode<T> *tmp_tn1 = NULL;
    ZTreeNode<T> *tmp_tn2 = tn;
    count = 0;
    while (tn != NULL) { /* <-o<-o o->o-> */
      tmp_tn2 = tn->getNext();
      tn->setNext(tmp_tn1);
      tmp_tn1 = tn;
      tn = tmp_tn2;
      count++;
    }
    m_begin = tmp_tn1;

    if (indexing) {
      tn = m_begin;
      tn->setIndex(0);
      while (tn->getNext() != NULL) {
        tn->getNext()->setIndex(tn->index() + 1);
        tn = tn->getNext();
      }
    }
  }
    break;
  default:
    break;
  }

  m_iterator = m_begin;

  return count;
}
#endif

template <typename T>
bool ZTree<T>::isEmpty(bool virtualCheck)
{
  bool bIsEmpty = true;

  if (m_root != NULL) {
    if (!m_root->hasChild() && virtualCheck) {
      bIsEmpty = m_root->isVirtual();
    } else {
      bIsEmpty = false;
    }
  }

  return bIsEmpty;
}
#if 0
template <typename T>
ZTreeNode<T>* ZTree<T>::begin()
{
  ZTreeNode<T> *tn = m_begin;

  if (tn != NULL) {
    m_iterator = tn->getNext();
  }

  return tn;
}

template <typename T>
ZTreeNode<T>* ZTree<T>::next()
{
  ZTreeNode<T> *tn = m_iterator;

  if (tn != NULL) {
    m_iterator = tn->getNext();
  }

  return tn;
}
#endif

template <typename T>
ZTreeNode<T>* ZTree<T>::getRegularRoot()
{
  ZTreeNode<T> *root = NULL;

  if (m_root->isRegular()) {
    root = m_root;
  } else {
    if (m_root != NULL) {
      assert(m_root->getFirstChild()->isRegular());
      root = m_root->m_getFirstChild();
    }
  }

  return root;
}

template <typename T>
int ZTree<T>::treeNumber()
{
  int n = 0;

  if (m_root != NULL) {
    if (m_root->isVirtual()) {
      n = m_root->getChildNumber();
    } else {
      n = 1;
    }
  }

  return n;
}
#if 0
template <typename T>
int ZTree<T>::labelTreeIdentity(
    int startLabel  //The label of the first tree
    )
{
  if (!isEmpty()) {
    //Label the roots
    if (m_root->isRegular()) {
      m_root->setLabel(startLabel);
    } else {
      ZTreeNode<T> *treeRoot = m_root->getFirstChild();

      while (treeRoot != NULL) {
        //Make sure it's a regular node
        assert(treeRoot->isRegular());
        treeRoot->setLabel(startLabel++);
        treeRoot = treeRoot->getNextSibling();
      }
    }

    updateIterator(ITERATOR_BREADTH_FIRST, false);
    for (ZTreeNode<T> *tn = begin(); tn != end(); tn = next()) {
      if (!tn->isRoot()) {
        tn->setLabel(tn->getParent()->getLabel());
      }
    }
  }

  return startLabel - 1;
}


template <typename T>
void ZTree<T>::resortId()
{
  updateIterator(ITERATOR_BREADTH_FIRST, false);
  int id = 1;
  ZTreeNode<T> *tn = NULL;

  while ((tn = next()) != NULL) {
    if (tn->getId() >= 0) {
      tn->setId(id);
      id++;
    }
  }
}
#endif

template <typename T>
void ZTree<T>::setRoot(
    ZTreeNode<T>* root,   //New root
    bool clearOld         //Clear the old root or not
    )
{
  if (clearOld) {
    clear();
  }

  m_root = root;
}


template <typename T>
void ZTree<T>::canonicalize()
{
  if (m_root != NULL) {
    if (m_root->isVirtual()) {
      if (m_root->getChildNumber() < 2) {
        if (!m_root->hasChild()) {
          delete m_root;
          m_root = NULL;
        } else {
          ZTreeNode<T> *tn = m_root->getFirstChild();
          tn->detachParent();
          delete m_root;
          m_root = tn;
        }
      }
    }
  }
}

template <typename T>
ZTreeIterator<T>::ZTreeIterator(const ZTree<T> &tree, EIteratorOption option)
{
    m_filter = NO_FILTER;
    update(tree, option);
}

template <typename T>
void ZTreeIterator<T>::update(const ZTree<T> &tree, EIteratorOption option)
{
  m_nodeArray.clear();

  const ZTreeNode<T> *tn = tree.getRoot();
  const ZTreeNode<T> *lastNode = tree.getRoot();

  if (tn != NULL) {
    switch(option) {
    case DEPTH_FIRST:
    {
      //m_nodeArray.push_back(const_cast<ZTreeNode<T>*>(tree.getRoot()));

      tryAppendingNode(lastNode);

      while (tn != NULL) {
        if (tn->firstChild() != NULL) {
          lastNode = tn->firstChild();
          tryAppendingNode(lastNode);

          //m_nodeArray.push_back(tn->firstChild());
          //tn->setNext(tn->firstChild());
        } else {
          if (tn->nextSibling() != NULL) {
              lastNode = tn->nextSibling();
              tryAppendingNode(lastNode);
            //m_nodeArray.push_back(tn->nextSibling());
            //tn->setNext(tn->nextSibling());
          } else {
            ZTreeNode<T> *parent_tn = tn->parent();
            while (parent_tn != NULL) {
              if (parent_tn->nextSibling() != NULL) {
                  lastNode = parent_tn->nextSibling();
                  tryAppendingNode(lastNode);
                //m_nodeArray.push_back(parent_tn->nextSibling());
                //tn->setNext(parent_tn->nextSibling());
                break;
              } else {
                parent_tn = parent_tn->parent();
              }
            }
            if (parent_tn == NULL) {
              tn = NULL;
            }
          }
        }

        if (tn != NULL) {
          //tn = m_nodeArray.back();
            tn = lastNode;
        }
      }
    }
      break;
    case BREADTH_FIRST:
    {
      std::queue<const ZTreeNode<T>*> currentSiblingQueue;
      currentSiblingQueue.push(lastNode);
      while (!currentSiblingQueue.empty()) {
        tn = currentSiblingQueue.front();
        tryAppendingNode(tn);
        currentSiblingQueue.pop();

        const ZTreeNode<T> *child = tn->firstChild();
        while (child != NULL) {
          currentSiblingQueue.push(child);
          child = child->nextSibling();
        }
      }
    }
      break;
    default:
      break;
    }
  }

  m_currentIndex = 0;
}

template <typename T>
bool ZTreeIterator<T>::hasNext()
{
  return m_currentIndex < m_nodeArray.size();
}

template <typename T>
T& ZTreeIterator<T>::next()
{
  T& data =  m_nodeArray[m_currentIndex]->data();
  ++m_currentIndex;

  return data;
}

template <typename T>
ZTreeNode<T>* ZTreeIterator<T>::nextNode()
{
  ZTreeNode<T> *node =  m_nodeArray[m_currentIndex];
  ++m_currentIndex;

  return node;
}

template <typename T>
bool ZTreeIterator<T>::passedFilter(const ZTreeNode<T> *node) const
{
    if (node == NULL) {
        return false;
    }

    switch (m_filter) {
    case NO_FILTER:
        return true;
    case LEAF:
        return node->isLeaf();
    case BRANCH_POINT:
        return node->isBranchPoint();
    case REGULAR:
        return node->isRegular();
    default:
        break;
    }

    return false;
}

template <typename T>
void ZTreeIterator<T>::tryAppendingNode(const ZTreeNode<T> *node)
{
    if (passedFilter(node)) {
        m_nodeArray.push_back(const_cast<ZTreeNode<T>*>(node));
    }
}
