#ifndef ZTREENODE_H
#define ZTREENODE_H

#include <vector>
#include "tz_cdefs.h"

template<typename T>
class ZTreeNode
{
public:
    ZTreeNode();
    ZTreeNode(const ZTreeNode<T> &node);
    ZTreeNode(const T &data);
    virtual ~ZTreeNode(void);

public:
    inline ZTreeNode<T>* parent() const { return m_parent; }
    inline ZTreeNode<T>* firstChild() const { return m_firstChild; }
    inline ZTreeNode<T>* nextSibling() const { return m_nextSibling; }
    void setFirstChild(ZTreeNode<T> *child, bool updatingConsisitency = true);
    void setNextSibling(ZTreeNode<T> *sibling, bool updatingConsistency = true);
    ZTreeNode<T>* getNextAt(int index) const;

public:
    virtual inline bool isVirtual(bool virtualCheck = true) const {
      UNUSED_PARAMETER(virtualCheck);
      return true;
    }
    inline bool isRegular(bool virtualCheck = true) const {
        return !isVirtual(virtualCheck);
    }

    inline T& data() { return m_data; }
    inline T* dataRef() { return &m_data; }
    inline const T* constDataRef() const { return &m_data; }

    inline void setData(const T &data) { m_data = data; }

    inline double weight() const { return m_weight; }
    inline void setWeight(double weight) { m_weight = weight; }

    inline int label() const { return m_label; }
    inline void setLabel(int label) { m_label = label; }

    inline int index() const { return m_index; }
    inline void setIndex(int index) { m_index = index; }

    ZTreeNode<T>* lastChild() const;

    bool isChildOf(const ZTreeNode<T> *parent) const;

    int childNumber()  const;

    inline bool hasChild()  const{ return (m_firstChild != NULL); }

    ZTreeNode<T>* previousSibling() const;

public:
    bool isRoot(bool virtualCheck = true) const;
    bool isLastChild() const;
    bool isLeaf(bool virtualCheck = true) const;
    bool isBranchPoint(bool virtualCheck = true) const;
    bool isContinuation(bool virtualCheck = true) const;
    bool isSpur(bool virtualCheck = true) const;
    bool isSibling(const ZTreeNode<T> *node) const;


    ZTreeNode<T>* addChild(ZTreeNode<T> *node);
    ZTreeNode<T>* addChild(const T& data);
    void removeChild(ZTreeNode<T> *child);
    void replaceChild(ZTreeNode<T> *oldChild, ZTreeNode<T> *newChild);

    ZTreeNode<T>* detachParent();
    void setParent(ZTreeNode<T> *p, bool updatingConsistency = true);

    void insert(ZTreeNode<T> *node);

    void becomeFirstChild();

    void mergeSibling(ZTreeNode<T> *node);

    virtual ZTreeNode<T>* mergeToParent();

    void becomeRoot(bool virtualCheck = true);

    double getBacktraceWeight(int n, bool virtualCheck = true) const;

    void labelBranch(int label, bool virtualCheck = true);

    virtual inline int id() const { return m_index; }
    int parentId() const;
    virtual inline void setId(int id) { m_index = id; }

protected:
    double m_weight;                    //weight to connect to its parent
    std::vector<double> m_featureVector;              //feature of the node
    int m_label;                   //label of identifying the node
    int m_index;                   //0-based index in iteration
    T m_data;                           //data
    ZTreeNode<T> *m_parent;        //parent
    ZTreeNode<T> *m_firstChild;    //first child
    ZTreeNode<T> *m_nextSibling;   //next sibling
};

#include "ztreenode.cpp"

#endif // ZTREENODE_H
