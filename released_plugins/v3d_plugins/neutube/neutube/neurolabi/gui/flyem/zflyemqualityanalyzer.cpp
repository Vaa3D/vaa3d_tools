#include "zflyemqualityanalyzer.h"
#include "neutubeconfig.h"
#include "zswctree.h"
#include "swctreenode.h"

ZFlyEmQualityAnalyzer::ZFlyEmQualityAnalyzer()
{
}

bool ZFlyEmQualityAnalyzer::isExitingOrphanBody(const ZObject3dScan &obj)
{
  //Expand bottom blocks
  Cuboid_I objBox;
  obj.getBoundBox(&objBox);

  //Test if the body touches the boundary
  ZObject3dScan tmpObj = obj;
  tmpObj.dilate();
  for (size_t i = 0; i < tmpObj.getStripeNumber(); ++i) {
    ZObject3dStripe stripe = tmpObj.getStripe(i);
    if (stripe.getZ() > objBox.ce[2]) {
      return false;
    }

    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      if (m_substackRegion.hitTest(stripe.getSegmentStart(j), stripe.getY(),
                                   stripe.getZ()) < 0) {
        return true;
      }

      if (m_substackRegion.hitTest(stripe.getSegmentEnd(j), stripe.getY(),
                                   stripe.getZ()) < 0) {
        return true;
      }
    }
  }

  return false;
}

bool ZFlyEmQualityAnalyzer::isOrphanBody(const ZObject3dScan &obj)
{
  Cuboid_I objBox;
  obj.getBoundBox(&objBox);

  int boxIndex = m_substackRegion.hitInternalTest(
        objBox.cb[0], objBox.cb[1], objBox.cb[2]);
  if (boxIndex >= 0) {
    if (boxIndex == m_substackRegion.hitInternalTest(
          objBox.ce[0], objBox.ce[1], objBox.ce[2])) {
      return true;
    }
  }

  return false;
}

bool ZFlyEmQualityAnalyzer::isStitchedOrphanBody(const ZObject3dScan &obj)
{
  if (obj.isEmpty()) {
    return false;
  }

  Cuboid_I objBox;
  obj.getBoundBox(&objBox);

  FlyEm::ZIntCuboidArray roi = m_substackRegion;
  roi.intersect(objBox);

  if (roi.size() == 1) {
    return false;
  } else {
#if 0

    FlyEm::ZIntCuboidArray innerFace = m_substackRegion.getInnerFace();
    FlyEm::ZIntCuboidArray face = m_substackRegion.getFace();

#ifdef _DEBUG_2
    innerFace.exportSwc(GET_DATA_DIR + "/test.swc");
    face.exportSwc(GET_DATA_DIR + "/test2.swc");
#endif


    if (face.hitTest(objBox.cb[0], objBox.cb[1], objBox.cb[2]) >= 0 &&
        innerFace.hitTest(objBox.cb[0], objBox.cb[1], objBox.cb[2]) < 0) {
#ifdef _DEBUG_
      std::cout << face.hitTest(objBox.cb[0], objBox.cb[1], objBox.cb[2]) << std::endl;
      std::cout << "Boundary: " << objBox.cb[0] << ' ' << objBox.cb[1] << ' '
                << objBox.cb[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.ce[0], objBox.cb[1], objBox.cb[2]) >= 0 &&
        innerFace.hitTest(objBox.ce[0], objBox.cb[1], objBox.cb[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.ce[0] << ' ' << objBox.cb[1] << ' '
                << objBox.cb[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.cb[0], objBox.ce[1], objBox.cb[2]) >= 0 &&
        innerFace.hitTest(objBox.cb[0], objBox.ce[1], objBox.cb[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.cb[0] << ' ' << objBox.ce[1] << ' '
                << objBox.cb[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.ce[0], objBox.ce[1], objBox.cb[2]) >= 0 &&
        innerFace.hitTest(objBox.ce[0], objBox.ce[1], objBox.cb[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.ce[0] << ' ' << objBox.ce[1] << ' '
                << objBox.cb[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.cb[0], objBox.cb[1], objBox.ce[2]) >= 0 &&
        innerFace.hitTest(objBox.cb[0], objBox.cb[1], objBox.ce[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.cb[0] << ' ' << objBox.cb[1] << ' '
                << objBox.ce[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.ce[0], objBox.cb[1], objBox.ce[2]) >= 0 &&
        innerFace.hitTest(objBox.ce[0], objBox.cb[1], objBox.ce[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.ce[0] << ' ' << objBox.cb[1] << ' '
                << objBox.ce[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.cb[0], objBox.ce[1], objBox.ce[2]) >= 0 &&
        innerFace.hitTest(objBox.cb[0], objBox.ce[1], objBox.ce[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.cb[0] << ' ' << objBox.ce[1] << ' '
                << objBox.ce[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.ce[0], objBox.ce[1], objBox.ce[2]) >= 0 &&
        innerFace.hitTest(objBox.ce[0], objBox.ce[1], objBox.ce[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.ce[0] << ' ' << objBox.ce[1] << ' '
                << objBox.ce[2] << std::endl;
#endif
      return false;
    }
#endif

    ZObject3dScan tmpObj = obj;
    tmpObj.dilate();
    for (size_t i = 0; i < tmpObj.getStripeNumber(); ++i) {
      ZObject3dStripe stripe = tmpObj.getStripe(i);

      for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
        if (m_substackRegion.hitTest(stripe.getSegmentStart(j), stripe.getY(),
                                     stripe.getZ()) < 0) {
          return false;
        }

        if (m_substackRegion.hitTest(stripe.getSegmentEnd(j), stripe.getY(),
                                     stripe.getZ()) < 0) {
          return false;
        }
      }
    }
  }

  return true;
}

void ZFlyEmQualityAnalyzer::setSubstackRegion(const FlyEm::ZIntCuboidArray &roi)
{
  m_substackRegion = roi;
}

void ZFlyEmQualityAnalyzer::labelSwcNodeOutOfRange(
    const ZFlyEmNeuron &neuron, const ZFlyEmNeuronRange &range, int label)
{
  ZSwcTree *tree = neuron.getModel();

  if (tree != NULL) {
    ZFlyEmNeuronAxis axis = neuron.getAxis();
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
      ZPoint pt = SwcTreeNode::pos(tn);
      ZPoint axisCenter = axis.getCenter(pt.z());
      pt.setX(pt.x() - axisCenter.x());
      pt.setY(pt.y() - axisCenter.y());
      if (!range.contains(pt)) {
        SwcTreeNode::setLabel(tn, label);
      }
    }
  }
}
