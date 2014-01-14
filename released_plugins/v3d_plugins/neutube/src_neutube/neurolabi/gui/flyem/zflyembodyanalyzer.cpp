#include "zflyembodyanalyzer.h"

#include <iostream>

#include "zstackskeletonizer.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "zstackgraph.h"
#include "zgraph.h"
#include "tz_stack_bwmorph.h"
#include "swc/zswcresampler.h"
#include "zgraphcompressor.h"
#include "zobject3d.h"

using namespace std;

ZFlyEmBodyAnalyzer::ZFlyEmBodyAnalyzer()
{
  for (int i = 0; i < 3; ++i) {
    m_downsampleInterval[i] = 0;
  }
  m_minLoopSize = 100;
}

ZPointArray ZFlyEmBodyAnalyzer::computeHoleCenter(const ZObject3dScan &obj)
{
  ZObject3dScan bufferObj = obj;

  bufferObj.downsampleMax(m_downsampleInterval[0], m_downsampleInterval[1],
      m_downsampleInterval[2]);

  ZPointArray centerArray;

  std::vector<ZObject3dScan> objArray = bufferObj.findHoleObjectArray();
  for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
       iter != objArray.end(); ++iter) {
    centerArray.push_back(iter->getCentroid());
  }

  centerArray.scale(m_downsampleInterval[0] + 1,
      m_downsampleInterval[1] + 1, m_downsampleInterval[2] + 1);

  return centerArray;
}

void ZFlyEmBodyAnalyzer::setDownsampleInterval(int ix, int iy, int iz)
{
  m_downsampleInterval[0] = ix;
  m_downsampleInterval[1] = iy;
  m_downsampleInterval[2] = iz;
}

ZPointArray ZFlyEmBodyAnalyzer::computeTerminalPoint(const ZObject3dScan &obj)
{
  ZObject3dScan bufferObj = obj;

  bufferObj.downsampleMax(m_downsampleInterval[0], m_downsampleInterval[1],
      m_downsampleInterval[2]);

  int offset[3] = {0, 0, 0};
  Stack *stack = bufferObj.toStack(offset);

  ZStackSkeletonizer skeletonizer;
  skeletonizer.setRebase(true);

  skeletonizer.setMinObjSize(100);
  skeletonizer.setDistanceThreshold(0);
  skeletonizer.setLengthThreshold(50);
  skeletonizer.setKeepingSingleObject(true);

  ZSwcTree *wholeTree = skeletonizer.makeSkeleton(stack);

  C_Stack::kill(stack);

  if (wholeTree != NULL) {
    ZSwcResampler resampler;
    resampler.optimalDownsample(wholeTree);
  }

  std::vector<Swc_Tree_Node*> terminalArray =
      wholeTree->getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR);

  ZPointArray pts;
  pts.resize(terminalArray.size());

  for (size_t i = 0; i < terminalArray.size(); ++i) {
    pts[i] = SwcTreeNode::pos(terminalArray[i]);
  }
  pts.translate(ZPoint(offset[0], offset[1], offset[2]));
  pts.scale(m_downsampleInterval[0] + 1, m_downsampleInterval[1] + 1,
      m_downsampleInterval[2] + 1);

  return pts;
}

ZPointArray ZFlyEmBodyAnalyzer::computeLoopCenter(const ZObject3dScan &obj)
{
  ZObject3dScan bufferObj = obj;

  bufferObj.downsampleMax(m_downsampleInterval[0], m_downsampleInterval[1],
      m_downsampleInterval[2]);

  int offset[3] = {0, 0, 0};

  cout << "Filling hole ..." << endl;
  bufferObj.fillHole();

  cout << "Creating stack ..." << endl;
  Stack *data = bufferObj.toStack(offset);

  cout << "Thinning ..." << endl;
  Stack *shrinked = Stack_Bwpeel(data, REMOVE_ARC, NULL);

  cout << "Building graph ..." << endl;
  ZStackGraph stackGraph;
  ZGraph *graph = stackGraph.buildForegroundGraph(shrinked);

  cout << "Compressing graph ..." << endl;
  ZGraphCompressor compressor;
  compressor.setGraph(graph);
  compressor.compress();

  ZPointArray pts;

  std::vector<std::vector<int> > cycleArray = graph->getCycle();
  for (size_t i = 0; i < cycleArray.size(); ++i) {
    vector<int> path = cycleArray[i];
    if ((int) path.size() >= m_minLoopSize) {
      ZObject3d *loopObj = new ZObject3d;
      for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
           ++iter) {
        int x, y, z;
        C_Stack::indexToCoord(compressor.uncompress(*iter), C_Stack::width(data),
                              C_Stack::height(data), &x, &y, &z);
        loopObj->append(x, y, z);
      }
      pts.push_back(loopObj->getCenter());
    }
  }
  pts.translate(ZPoint(offset[0], offset[1], offset[2]));
  pts.scale(m_downsampleInterval[0] + 1, m_downsampleInterval[1] + 1,
      m_downsampleInterval[2] + 1);

  delete graph;
  Kill_Stack(shrinked);
  Kill_Stack(data);

  return pts;
}
