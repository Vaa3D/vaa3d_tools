#include "zswcgenerator.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "swc/zswcresampler.h"
#include "flyem/zflyemneuronrangecompare.h"

ZSwcGenerator::ZSwcGenerator()
{
}

ZSwcTree* ZSwcGenerator::createVirtualRootSwc()
{
  ZSwcTree *tree = new ZSwcTree;
  tree->forceVirtualRoot();

  return tree;
}

ZSwcTree* ZSwcGenerator::createCircleSwc(double cx, double cy, double cz, double r)
{
  if (r < 0.0) {
    return NULL;
  }

  ZSwcTree *tree = createVirtualRootSwc();

  Swc_Tree_Node *parent = tree->root();

  double nodeRadius = r * 0.05;

  for (double angle = 0.0; angle < 6.0; angle += 0.314) {
    double x, y, z;

    x = r * cos(angle) + cx;
    y = r * sin(angle) + cy;
    z = cz;

    Swc_Tree_Node *tn = SwcTreeNode::makePointer(x, y, z, nodeRadius);
    SwcTreeNode::setParent(tn, parent);
    parent = tn;
  }

  Swc_Tree_Node *tn = SwcTreeNode::makePointer();
  SwcTreeNode::copyProperty(SwcTreeNode::firstChild(tree->root()), tn);
  SwcTreeNode::setParent(tn, parent);

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcGenerator::createBoxSwc(const ZCuboid &box)
{
  return ZSwcTree::createCuboidSwc(box);
}

ZSwcTree* ZSwcGenerator::createSwc(const ZFlyEmNeuronRange &range)
{
  if (range.isEmpty()) {
    return NULL;
  }

  double minZ = range.getMinZ();
  double maxZ = range.getMaxZ();

  double dz = (maxZ - minZ) / 50.0;

  if (dz == 0.0) { //Avoid dead loop
    dz = 1.0;
  }

  ZSwcTree *tree = createVirtualRootSwc();

  for (double z = minZ; z <= maxZ; z += dz) {
    double r = range.getRadius(z);
    ZSwcTree *subtree = createCircleSwc(0, 0, z, r);
    tree->merge(subtree, true);
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcGenerator::createSwc(
    const ZFlyEmNeuronRange &range, const ZFlyEmNeuronAxis &axis)
{
  double minZ = range.getMinZ();
  double maxZ = range.getMaxZ();

  double dz = (maxZ - minZ) / 50.0;

  ZSwcTree *tree = createVirtualRootSwc();

  for (double z = minZ; z <= maxZ; z += dz) {
    double r = range.getRadius(z);
    ZPoint pt = axis.getCenter(z);
    ZSwcTree *subtree = createCircleSwc(pt.x(), pt.y(), z, r);
    tree->merge(subtree, true);
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcGenerator::createRangeCompareSwc(
    const ZFlyEmNeuronRange &range, const ZFlyEmNeuronRange &reference)
{
  if (range.isEmpty()) {
    return NULL;
  }

  double minZ = range.getMinZ();
  double maxZ = range.getMaxZ();

  double dz = (maxZ - minZ) / 50.0;

  if (dz == 0.0) { //Avoid dead loop
    dz = 1.0;
  }

  ZSwcTree *tree = createVirtualRootSwc();

  double minReferenceZ = reference.getMinZ();
  double maxReferenceZ = reference.getMaxZ();

  for (double z = minReferenceZ; z < minZ; z += dz) {
    ZSwcTree *subtree = createCircleSwc(0, 0, z, reference.getRadius(z));
    subtree->setType(5);
    tree->merge(subtree, true);
  }

  for (double z = maxZ + dz; z <= maxReferenceZ; z += dz) {
    ZSwcTree *subtree = createCircleSwc(0, 0, z, reference.getRadius(z));
    subtree->setType(5);
    tree->merge(subtree, true);
  }

  for (double z = minZ; z <= maxZ; z += dz) {
    double r = range.getRadius(z);
    ZSwcTree *subtree = createCircleSwc(0, 0, z, r);
    ZFlyEmNeuronRangeCompare comp;
    ZFlyEmNeuronRangeCompare::EMatchStatus status =
        comp.compare(range, reference, z);
    switch (status) {
    case ZFlyEmNeuronRangeCompare::MISSING_BRANCH:
      subtree->setType(2);
      break;
    case ZFlyEmNeuronRangeCompare::EXTRA_BRANCH:
      subtree->setType(3);
      break;
    case ZFlyEmNeuronRangeCompare::GOOD_MATCH:
      subtree->setType(0);
      break;
    default:
      subtree->setType(0);
    }

    tree->merge(subtree, true);
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcGenerator::createSwc(
    const ZVoxelArray &voxelArray, ZSwcGenerator::EPostProcess option)
{
  size_t startIndex = 0;
  size_t endIndex = voxelArray.size() - 1;

  Swc_Tree *tree = New_Swc_Tree();

  ZVoxel prevVoxel = voxelArray[startIndex];

  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  SwcTreeNode::setPos(tn, prevVoxel.x(), prevVoxel.y(), prevVoxel.z());
  SwcTreeNode::setRadius(tn, prevVoxel.value());

  Swc_Tree_Node *prevTn = tn;

  for (size_t i = startIndex + 1; i < endIndex; i++) {
    double dist = voxelArray[i].distanceTo(prevVoxel);
    bool sampling = false;
    if (dist > prevVoxel.value()) {
      sampling = true;
    }

    if (sampling) {
      tn = New_Swc_Tree_Node();

      SwcTreeNode::setPos(tn, voxelArray[i].x(), voxelArray[i].y(), voxelArray[i].z());
      SwcTreeNode::setRadius(tn, voxelArray[i].value());
      Swc_Tree_Node_Set_Parent(prevTn, tn);
      prevTn = tn;
      prevVoxel = voxelArray[i];
    }
  }

  if (endIndex - startIndex > 0) { //last node
    tn = New_Swc_Tree_Node();

    SwcTreeNode::setPos(tn, voxelArray[endIndex].x(), voxelArray[endIndex].y(),
                        voxelArray[endIndex].z());
    SwcTreeNode::setRadius(tn, voxelArray[endIndex].value());
    Swc_Tree_Node_Set_Parent(prevTn, tn);
    /*
    if (SwcTreeNode::hasOverlap(prevTn, tn) && SwcTreeNode::hasChild(prevTn)) {
      SwcTreeNode::mergeToParent(prevTn);
    }
    */
  }

  tree->root = tn;

  ZSwcTree *treeWrapper = new ZSwcTree;
  treeWrapper->setData(tree);

  if (option == OPTIMAL_SAMPLING) {
    ZSwcResampler resampler;
    resampler.optimalDownsample(treeWrapper);
  }

  return treeWrapper;
}
