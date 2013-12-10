#include "zflyemneuronaxis.h"
#include "zerror.h"
#include "zswctree.h"
#include "swctreenode.h"

ZFlyEmNeuronAxis::ZFlyEmNeuronAxis()
{
}

bool ZFlyEmNeuronAxis::isEmpty() const
{
  return m_axis.empty();
}

double ZFlyEmNeuronAxis::getMinZ() const
{
  if (m_axis.empty()) {
    return 0.0;
  }

  return m_axis.begin()->first;
}

double ZFlyEmNeuronAxis::getMaxZ() const
{
  if (m_axis.empty()) {
    return 0.0;
  }

  return m_axis.rbegin()->first;
}

ZPoint ZFlyEmNeuronAxis::getCenter(double z) const
{
  AxisPoint pt(0, 0);
  if (z <= getMinZ()) {
    pt = m_axis.begin()->second;
  } else if (z >= getMaxZ()) {
    pt = m_axis.rbegin()->second;
  } else {
    double z1 = 0.0;
    double z2 = 0.0;

    for (AxisMap::const_iterator iter = m_axis.begin();
         iter != m_axis.end(); ++iter) {
      z2 = iter->first;
      if (z2 >= z) {
        break;
      }
      z1 = iter->first;
    }

    PROCESS_WARNING(z1 == z2, "Unexpected identical plane", return ZPoint());
    double ratio = (z - z1) / (z2 - z1);

    AxisPoint pt1 = m_axis.at(z1);
    AxisPoint pt2 = m_axis.at(z2);

    pt.first = pt1.first * (1 - ratio) + pt2.first * ratio;
    pt.second = pt1.second * (1 - ratio) + pt2.second * ratio;
  }

  ZPoint center(pt.first, pt.second, z);

  return center;
}

void ZFlyEmNeuronAxis::setCenter(const ZPoint &center)
{
  m_axis[center.z()] = AxisPoint(center.x(), center.y());
}

ZSwcTree* ZFlyEmNeuronAxis::toSwc(double radius, double sampleRate)
{
  if (m_axis.empty()) {
    return NULL;
  }

  ZSwcTree *tree = new ZSwcTree;
  tree->forceVirtualRoot();

  Swc_Tree_Node *parent = tree->root();

  if (sampleRate <= 0.0) {
    for (AxisMap::const_iterator iter = m_axis.begin();
         iter != m_axis.end(); ++iter) {
      AxisPoint pt = iter->second;

      Swc_Tree_Node *tn =
          SwcTreeNode::makePointer(pt.first, pt.second, iter->first, radius);
      SwcTreeNode::setParent(tn, parent);
      parent = tn;
    }
  } else {
    double minZ = getMinZ();
    double maxZ = getMaxZ();

    for (double z = minZ; z < maxZ; z += sampleRate) {
      ZPoint center = getCenter(z);
      Swc_Tree_Node *tn =
          SwcTreeNode::makePointer(center, radius);
      SwcTreeNode::setParent(tn, parent);
      parent = tn;
    }

    ZPoint center = getCenter(maxZ);
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(center, radius);
    SwcTreeNode::setParent(tn, parent);
  }

  tree->resortId();

  return tree;
}
