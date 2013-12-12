#ifndef ZSWCTREENODE_H
#define ZSWCTREENODE_H

#include "ztreenode.h"
#include "zswcnode.h"

class ZSwcTreeNode : public ZTreeNode<ZSwcNode>
{
public:
    ZSwcTreeNode(void);
    ~ZSwcTreeNode(void);

public:
    virtual inline int id() const { return m_data.id(); }
    inline ZSwcNode::EType type() const { return m_data.type(); }
    inline double r() const { return m_data.r(); }
    inline double x() const { return m_data.x(); }
    inline double y() const { return m_data.y(); }
    inline double z() const { return m_data.z(); }

    virtual inline void setId(int id) { m_data.setId(id); }
    inline void setType(ZSwcNode::EType type) { m_data.setType(type); }
    inline void setRadius(double r) { m_data.setR(r); }
    inline void setX(double x) { m_data.setX(x); }
    inline void setY(double y) { m_data.setY(y); }
    inline void setZ(double z) { m_data.setZ(z); }

    //Get the parent that has the same type as the node
    ZSwcTreeNode *getCompleteParent() const;
    //Get the first child that has the same type as the node
    ZSwcTreeNode *getCompleteFirstChild() const;

    double computeBendingAngle();

public:
    virtual bool isVirtual(bool virtualCheck = true) const;
    bool hasSomaParent() const;
    bool hasNeuriteParent() const;

    //Compute the distance between the node and its parent. It returns 0 if
    //the node is a root
    double computeDistanceToParent() const;
    //Compute the distance between two swc nodes
    double computeDistance(const ZSwcTreeNode *node) const;
    //Compute the neurite length between the node and its parent
    double computeNeuriteLength() const;
    //Compute the neurite area between the node and its parent
    double computeNeuriteArea() const;
};


#endif // ZSWCTREENODE_H
