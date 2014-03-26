#include "zswctreenode.h"

#include <math.h>

using namespace std;

ZSwcTreeNode::ZSwcTreeNode(void)
{
}

ZSwcTreeNode::~ZSwcTreeNode(void)
{
}

bool ZSwcTreeNode::isVirtual(
    bool virtualCheck //The functin always returns false if this is false
    ) const
{
    bool bIsVirtual = false;

    if (virtualCheck) {
        if (m_data.id() == -1) {
            bIsVirtual = true;
        }
    }

    return bIsVirtual;
}


double ZSwcTreeNode::computeDistanceToParent() const
{
    return computeDistance(getCompleteParent());
}

ZSwcTreeNode* ZSwcTreeNode::getCompleteParent() const
{
    if (parent() == NULL) {
        return NULL;
    }

    return dynamic_cast<ZSwcTreeNode*>(parent());
}

ZSwcTreeNode* ZSwcTreeNode::getCompleteFirstChild() const
{
    if (firstChild() == NULL) {
        return NULL;
    }

    return dynamic_cast<ZSwcTreeNode*>(firstChild());
}


double ZSwcTreeNode::computeDistance(
    const ZSwcTreeNode *node   //The target node for distance calculation
    ) const
{
    double distance = 0.0;

    if (node != NULL) {
        if (node->isRegular()) {
            double dx = x() - node->x();
            double dy = y() - node->y();
            double dz = z() - node->z();
            distance = sqrt(dx * dx + dy * dy + dz * dz);
        }
    }

    return distance;
}
