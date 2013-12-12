#ifndef ZOBJECT3DSCANTEST_H
#define ZOBJECT3DSCANTEST_H

#include "ztestheader.h"
#include "zobject3dscan.h"
#include "neutubeconfig.h"
#include "zgraph.h"

#ifdef _USE_GTEST_

static void createStripe(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(0, 1);
  stripe->addSegment(3, 5);
}

static void createStripe2(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(3, 5, false);
  stripe->addSegment(0, 1, false);
  stripe->addSegment(3, 1, false);
}

static void createStripe3(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(3, 5);
  stripe->addSegment(0, 1);
  stripe->addSegment(3, 1);
}

static void createStripe4(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(3, 5);
  stripe->addSegment(0, 1);
  stripe->addSegment(3, 1, false);
}

TEST(TestObject3dStripe, TestGetProperty) {
  ZObject3dStripe stripe;
  createStripe(&stripe);
  EXPECT_EQ(stripe.getMinX(), 0);
  EXPECT_EQ(stripe.getMaxX(), 5);
  EXPECT_EQ(stripe.getSegmentNumber(), 2);
  EXPECT_EQ((int) stripe.getSize(), 2);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ(stripe.getVoxelNumber(), 5);

  createStripe2(&stripe);
  EXPECT_EQ(stripe.getSegmentNumber(), 2);
  EXPECT_EQ((int) stripe.getSize(), 2);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ(stripe.getVoxelNumber(), 7);

  stripe.canonize();
  EXPECT_EQ(stripe.getMinX(), 0);
  EXPECT_EQ(stripe.getMaxX(), 5);
  EXPECT_EQ(stripe.getSegmentNumber(), 1);
  EXPECT_EQ((int) stripe.getSize(), 1);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ(stripe.getVoxelNumber(), 6);

  createStripe3(&stripe);
  EXPECT_EQ(stripe.getMinX(), 0);
  EXPECT_EQ(stripe.getMaxX(), 5);
  EXPECT_EQ(stripe.getSegmentNumber(), 1);
  EXPECT_EQ((int) stripe.getSize(), 1);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ(stripe.getVoxelNumber(), 6);

  createStripe4(&stripe);
  EXPECT_EQ(stripe.getSegmentNumber(), 3);
  EXPECT_EQ((int) stripe.getSize(), 3);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);

  stripe.canonize();
  EXPECT_EQ(stripe.getMinX(), 0);
  EXPECT_EQ(stripe.getMaxX(), 5);
  EXPECT_EQ(stripe.getSegmentNumber(), 1);
  EXPECT_EQ((int) stripe.getSize(), 1);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ(stripe.getVoxelNumber(), 6);
}

TEST(TestObject3dStripe, TestUnify) {
  ZObject3dStripe stripe;
  stripe.setY(3);
  stripe.setZ(5);
  stripe.addSegment(3, 5);

  ZObject3dStripe stripe2;
  stripe2.setY(3);
  stripe2.setZ(5);
  stripe2.addSegment(6, 7);

  EXPECT_FALSE(stripe.equalsLiterally(stripe2));

  EXPECT_TRUE(stripe.unify(stripe2));
  EXPECT_EQ(1, stripe.getSegmentNumber());
  EXPECT_EQ(3, stripe.getMinX());
  EXPECT_EQ(7, stripe.getMaxX());
  EXPECT_EQ(5, stripe.getVoxelNumber());

  stripe2.setY(4);
  EXPECT_FALSE(stripe.unify(stripe2));
  EXPECT_EQ(1, stripe.getSegmentNumber());
  EXPECT_EQ(3, stripe.getMinX());
  EXPECT_EQ(7, stripe.getMaxX());
  EXPECT_EQ(5, stripe.getVoxelNumber());

  stripe2.setY(3);
  stripe2.setZ(4);
  EXPECT_FALSE(stripe.unify(stripe2));
  EXPECT_EQ(1, stripe.getSegmentNumber());
  EXPECT_EQ(3, stripe.getMinX());
  EXPECT_EQ(7, stripe.getMaxX());
  EXPECT_EQ(5, stripe.getVoxelNumber());

  stripe2.clearSegment();
  stripe2.setY(3);
  stripe2.setZ(5);
  stripe2.addSegment(1, 7);
  EXPECT_TRUE(stripe.unify(stripe2));
  EXPECT_EQ(1, stripe.getSegmentNumber());
  EXPECT_EQ(1, stripe.getMinX());
  EXPECT_EQ(7, stripe.getMaxX());
  EXPECT_EQ(7, stripe.getVoxelNumber());

  stripe2.clearSegment();
  stripe2.setY(3);
  stripe2.setZ(5);
  stripe2.addSegment(9, 10);
  EXPECT_TRUE(stripe.unify(stripe2));
  EXPECT_EQ(2, stripe.getSegmentNumber());
  EXPECT_EQ(1, stripe.getMinX());
  EXPECT_EQ(10, stripe.getMaxX());
  EXPECT_EQ(9, stripe.getVoxelNumber());
}

TEST(TestObject3dStripe, TestIO) {
  FILE *fp = fopen((GET_TEST_DATA_DIR + "/test.sobj").c_str(), "w");
  ZObject3dStripe stripe;
  createStripe2(&stripe);
  stripe.write(fp);

  fclose(fp);

  fp = fopen((GET_TEST_DATA_DIR + "/test.sobj").c_str(), "r");
  ZObject3dStripe stripe2;
  stripe2.read(fp);
  fclose(fp);

  EXPECT_TRUE(stripe.equalsLiterally(stripe));
  EXPECT_TRUE(stripe.equalsLiterally(stripe2));
}

bool isSorted(const ZObject3dStripe &stripe)
{
  if (!stripe.isEmpty()) {
    for (int i = 0; i < stripe.getSegmentNumber() - 1; ++i) {
      if (stripe.getSegmentStart(i) > stripe.getSegmentStart(i + 1)) {
        return false;
      } else if (stripe.getSegmentStart(i) == stripe.getSegmentStart(i + 1)) {
        if (stripe.getSegmentEnd(i) > stripe.getSegmentEnd(i + 1)) {
          return false;
        }
      }
    }
  }

  return true;
}

TEST(TestObject3dStripe, TestSort) {
  ZObject3dStripe stripe;
  createStripe2(&stripe);
  EXPECT_FALSE(isSorted(stripe));

  stripe.sort();
  EXPECT_TRUE(isSorted(stripe));

  stripe.clearSegment();
  stripe.addSegment(1, 2);
  stripe.addSegment(4, 5, false);
  stripe.addSegment(1, 2, false);
  stripe.addSegment(3, 5, false);
  EXPECT_FALSE(isSorted(stripe));

  stripe.sort();
  EXPECT_TRUE(isSorted(stripe));

  stripe.clearSegment();
  stripe.setY(0);
  stripe.setZ(1);
  stripe.addSegment(0, 1, false);
  stripe.addSegment(4, 5, false);
  stripe.addSegment(1, 0, false);
  stripe.addSegment(3, 9, false);
  stripe.addSegment(3, 5, false);

  stripe.sort();

  EXPECT_TRUE(isSorted(stripe));
}

TEST(TestObject3dStripe, TestCanonize) {
  ZObject3dStripe stripe;
  createStripe2(&stripe);
  EXPECT_FALSE(stripe.isCanonized());

  stripe.canonize();
  EXPECT_TRUE(isSorted(stripe));

  stripe.clearSegment();
  stripe.addSegment(1, 2);
  stripe.addSegment(3, 4, false);
  stripe.addSegment(3, 9, false);
  stripe.addSegment(3, 5, false);
  EXPECT_TRUE(stripe.isCanonized());

  stripe.canonize();
  EXPECT_TRUE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(1, 2);
  stripe.addSegment(4, 5, false);
  stripe.addSegment(7, 8, false);
  EXPECT_TRUE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(7, 8, false);
  stripe.addSegment(1, 2, false);
  EXPECT_FALSE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(4, 5);
  stripe.addSegment(1, 7, false);
  stripe.addSegment(7, 8, false);
  EXPECT_FALSE(stripe.isCanonized());

  stripe.canonize();
  EXPECT_TRUE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(4, 5);
  stripe.addSegment(4, 7, false);
  stripe.addSegment(7, 8, false);
  stripe.addSegment(4, 8, false);
  stripe.addSegment(5, 6, false);
  stripe.addSegment(10, 15, false);
  stripe.addSegment(19, 15, false);
  EXPECT_TRUE(stripe.isCanonized());
  EXPECT_EQ(2, stripe.getSegmentNumber());
  EXPECT_EQ(15, stripe.getVoxelNumber());
}

static void createObject(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(4, 5, false);
  obj->addSegment(7, 8, false);
  obj->addStripe(0, 1, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(3, 3, false);
  obj->addSegment(5, 7, false);
}

static void createObject2(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(0, 5, false);
  obj->addSegment(0, 8, false);
  obj->addStripe(0, 1, false);
  obj->addSegment(3, 3, false);
  obj->addSegment(0, 1, false);
  obj->addStripe(0, 1, false);
  obj->addSegment(5, 7, false);
}

static void createObject3(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0);
  obj->addSegment(0, 1);
  obj->addSegment(0, 5);
  obj->addSegment(0, 8);
  obj->addStripe(0, 1);
  obj->addSegment(3, 3);
  obj->addSegment(0, 1);
  obj->addStripe(0, 1);
  obj->addSegment(5, 7);
}

TEST(TestObject3dScan, TestGetProperty) {
  ZObject3dScan obj;
  createObject(&obj);
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  ZCuboid box = obj.getBoundBox();
  ASSERT_EQ(box.firstCorner().x(), 0);
  ASSERT_EQ(box.firstCorner().y(), 0);
  ASSERT_EQ(box.firstCorner().z(), 0);

  ASSERT_EQ(box.lastCorner().x(), 8);
  ASSERT_EQ(box.lastCorner().y(), 1);
  ASSERT_EQ(box.lastCorner().z(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 12);

  obj.canonize();
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.firstCorner().x(), 0);
  ASSERT_EQ(box.firstCorner().y(), 0);
  ASSERT_EQ(box.firstCorner().z(), 0);

  ASSERT_EQ(box.lastCorner().x(), 8);
  ASSERT_EQ(box.lastCorner().y(), 1);
  ASSERT_EQ(box.lastCorner().z(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 12);

  createObject2(&obj);
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.firstCorner().x(), 0);
  ASSERT_EQ(box.firstCorner().y(), 0);
  ASSERT_EQ(box.firstCorner().z(), 0);

  ASSERT_EQ(box.lastCorner().x(), 8);
  ASSERT_EQ(box.lastCorner().y(), 1);
  ASSERT_EQ(box.lastCorner().z(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 15);

  obj.canonize();
  //obj.print();
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.firstCorner().x(), 0);
  ASSERT_EQ(box.firstCorner().y(), 0);
  ASSERT_EQ(box.firstCorner().z(), 0);

  ASSERT_EQ(box.lastCorner().x(), 8);
  ASSERT_EQ(box.lastCorner().y(), 1);
  ASSERT_EQ(box.lastCorner().z(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 15);

  createObject3(&obj);
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.firstCorner().x(), 0);
  ASSERT_EQ(box.firstCorner().y(), 0);
  ASSERT_EQ(box.firstCorner().z(), 0);

  ASSERT_EQ(box.lastCorner().x(), 8);
  ASSERT_EQ(box.lastCorner().y(), 1);
  ASSERT_EQ(box.lastCorner().z(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 15);

}

TEST(TestObject3dScan, TestAddSegment) {
  ZObject3dScan obj;
  obj.addStripe(1, 0);
  obj.addSegment(1, 2, false);
  obj.addSegment(3, 4, false);

  EXPECT_TRUE(obj.isCanonized());

  obj.addStripe(1, 0);
  obj.addSegment(5, 6, false);
  obj.addSegment(7, 8, false);
  EXPECT_TRUE(obj.isCanonized());

  obj.addSegment(5, 6, false);
  EXPECT_TRUE(obj.isCanonized());

  obj.addSegment(3, 6, false);
  EXPECT_TRUE(obj.isCanonized());

  obj.clear();
  obj.addStripe(1, 0);
  obj.addSegment(5, 6, false);
  obj.addSegment(7, 8, false);
  obj.addSegment(3, 6, false);
  EXPECT_FALSE(obj.isCanonized());

  obj.clear();
  obj.addStripe(1, 0);
  obj.addSegment(1, 2, false);
  obj.addSegment(1, 0, 3, 4, false);

  EXPECT_TRUE(obj.isCanonized());

  obj.clear();
  obj.addSegment(1, 0, 5, 6, false);
  obj.addSegment(1, 0, 7, 8, false);
  obj.addSegment(1, 0, 3, 6, false);
  EXPECT_FALSE(obj.isCanonized());

  obj.clear();
  obj.addSegment(0, 0, 0, 1, false);
  obj.addSegment(0, 0, 0, 5, false);
  obj.addSegment(0, 0, 0, 8, false);
  obj.addSegment(0, 1, 3, 3, false);
  obj.addSegment(0, 1, 0, 1, false);
  obj.addSegment(0, 1, 5, 7, false);
  EXPECT_EQ(2, (int) obj.getStripeNumber());
  EXPECT_EQ(15, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 0, 5);
  obj.addSegment(0, 0, 0, 8);
  obj.addSegment(0, 1, 3, 3);
  obj.addSegment(0, 1, 0, 1);
  obj.addSegment(0, 1, 5, 7);
  EXPECT_EQ(2, (int) obj.getStripeNumber());
  EXPECT_EQ(15, (int) obj.getVoxelNumber());
}

TEST(TestObject3dScan, TestDownsample) {
  ZObject3dScan obj;
  createObject(&obj);


  obj.downsample(1, 1, 1);
  EXPECT_EQ(1, (int) obj.getStripeNumber());
  EXPECT_EQ(3, (int) obj.getVoxelNumber());

  createObject(&obj);
  obj.downsampleMax(1, 1, 1);
  //obj.print();
  EXPECT_EQ(1, (int) obj.getStripeNumber());
  EXPECT_EQ(5, (int) obj.getVoxelNumber());
}

TEST(TestObject3dScan, TestObjectSize){
  ZObject3dScan obj;

  std::vector<size_t> sizeArray = obj.getConnectedObjectSize();
  EXPECT_TRUE(sizeArray.empty());

  createObject(&obj);
  sizeArray = obj.getConnectedObjectSize();
  EXPECT_EQ(2, (int) sizeArray.size());
  EXPECT_EQ(8, (int) sizeArray[0]);
  EXPECT_EQ(4, (int) sizeArray[1]);

  obj.clear();
  Stack *stack = C_Stack::readSc(GET_TEST_DATA_DIR +
        "/benchmark/binary/2d/disk_n2.tif");
  obj.loadStack(stack);
  EXPECT_TRUE(obj.isCanonized());
  sizeArray = obj.getConnectedObjectSize();
  EXPECT_EQ(2, (int) sizeArray.size());
  EXPECT_EQ(489, (int) sizeArray[0]);
  EXPECT_EQ(384, (int) sizeArray[1]);

  C_Stack::kill(stack);

  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/2d/ring_n10.tif");
  obj.loadStack(stack);

  EXPECT_TRUE(obj.isCanonized());

  sizeArray = obj.getConnectedObjectSize();

  EXPECT_EQ(10, (int) sizeArray.size());
  EXPECT_EQ(616, (int) sizeArray[0]);
  EXPECT_EQ(572, (int) sizeArray[1]);
  EXPECT_EQ(352, (int) sizeArray[2]);
  EXPECT_EQ(296, (int) sizeArray[3]);
  EXPECT_EQ(293, (int) sizeArray[4]);
  EXPECT_EQ(279, (int) sizeArray[5]);
  EXPECT_EQ(208, (int) sizeArray[6]);
  EXPECT_EQ(125, (int) sizeArray[7]);
  EXPECT_EQ(112, (int) sizeArray[8]);
  EXPECT_EQ(112, (int) sizeArray[9]);

  C_Stack::kill(stack);

  /*
  obj.clear();
  obj.load(GET_TEST_DATA_DIR +
           "/benchmark/432.sobj");
  sizeArray = obj.getConnectedObjectSize();
  EXPECT_EQ(77, (int) sizeArray.size());
  std::cout << sizeArray[0] << std::endl;

  int offset[3];
  stack = obj.toStack(offset);
  offset[0] = -offset[0];
  offset[1] = -offset[1];
  offset[2] = -offset[2];
  obj.labelStack(stack, 2, offset);
  C_Stack::write(GET_TEST_DATA_DIR +
                 "/test.tif", stack);
                 */
  //EXPECT_EQ(616, (int) sizeArray[0]);
}

TEST(TestObject3dScan, TestBuildGraph) {
  ZObject3dScan obj;
  //createObject(&obj);
  obj.addStripe(0, 0, false);
  obj.addSegment(0, 1, false);

  ZGraph *graph = obj.buildConnectionGraph();

  EXPECT_EQ(0, graph->getEdgeNumber());

  delete graph;

  obj.addSegment(3, 4);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(0, graph->getEdgeNumber());
  delete graph;

  obj.addStripe(0, 1, false);
  obj.addSegment(0, 1, false);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(1, graph->getEdgeNumber());
  delete graph;

  obj.addSegment(2, 2, false);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(2, graph->getEdgeNumber());
  delete graph;

  obj.addStripe(1, 0);
  obj.addSegment(2, 2);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(5, graph->getEdgeNumber());

  const std::vector<ZGraph*> &subGraph = graph->getConnectedSubgraph();
  EXPECT_EQ(1, (int) subGraph.size());
  delete graph;

  obj.clear();
  Stack *stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/2d/ring_n10.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph2 = graph->getConnectedSubgraph();
  EXPECT_EQ(10, (int) subGraph2.size());
  delete graph;
  C_Stack::kill(stack);

#if 1
  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/3d/diadem_e1.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph3 = graph->getConnectedSubgraph();
  EXPECT_EQ(4, (int) subGraph3.size());
  delete graph;
#endif

  obj.clear();
  obj.addStripe(1, 2);
  obj.addSegment(2, 2);
  obj.addStripe(2, 1);
  obj.addSegment(1, 1);
  obj.addSegment(3, 3);
  obj.addStripe(2, 3);
  obj.addSegment(1, 1);
  obj.addSegment(3, 3);
  obj.addStripe(3, 0);
  obj.addSegment(0, 0);
  obj.addSegment(2, 2);
  obj.addSegment(4, 4);
  obj.addStripe(3, 2);
  obj.addSegment(0, 0);
  obj.addSegment(4, 4);
  obj.addStripe(3, 4);
  obj.addSegment(0, 0);
  obj.addSegment(2, 2);
  obj.addSegment(4, 4);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph5 = graph->getConnectedSubgraph();
  EXPECT_EQ(16, graph->getEdgeNumber());
  EXPECT_EQ(1, (int) subGraph5.size());
  delete graph;


  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/3d/series.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph4 = graph->getConnectedSubgraph();
  EXPECT_EQ(15, (int) subGraph4.size());
  delete graph;

  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/3d/block/test.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(1, (int) graph->getConnectedSubgraph().size());
  delete graph;
}

/*
static void createObject(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(4, 5, false);
  obj->addSegment(7, 8, false);
  obj->addStripe(0, 1, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(3, 3, false);
  obj->addSegment(5, 7, false);
}
*/

TEST(TestObject3dScan, TestGetSegment) {
  ZObject3dScan obj;
  createObject(&obj);
  int z, y, x1, x2;
  obj.getSegment(0, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(0, y);
  EXPECT_EQ(0, x1);
  EXPECT_EQ(1, x2);

  obj.getSegment(1, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(0, y);
  EXPECT_EQ(4, x1);
  EXPECT_EQ(5, x2);

  obj.getSegment(2, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(0, y);
  EXPECT_EQ(7, x1);
  EXPECT_EQ(8, x2);

  obj.getSegment(3, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(1, y);
  EXPECT_EQ(0, x1);
  EXPECT_EQ(1, x2);

  obj.getSegment(4, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(1, y);
  EXPECT_EQ(3, x1);
  EXPECT_EQ(3, x2);

  obj.getSegment(5, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(1, y);
  EXPECT_EQ(5, x1);
  EXPECT_EQ(7, x2);
}

TEST(TestObject3dScan, TestGetConnectedComponent) {
  ZObject3dScan obj;
  createObject(&obj);

  std::vector<ZObject3dScan> objArray = obj.getConnectedComponent();
  EXPECT_EQ(2, (int) objArray.size());
  EXPECT_EQ(4, (int) objArray[0].getVoxelNumber());
  EXPECT_EQ(8, (int) objArray[1].getVoxelNumber());

  obj.clear();
  Stack *stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/2d/ring_n10.tif");
  obj.loadStack(stack);

  objArray = obj.getConnectedComponent();
  EXPECT_EQ(10, (int) objArray.size());
  EXPECT_EQ(352, (int) objArray[0].getVoxelNumber());
  EXPECT_EQ(279, (int) objArray[1].getVoxelNumber());
  EXPECT_EQ(125, (int) objArray[2].getVoxelNumber());
  EXPECT_EQ(112, (int) objArray[3].getVoxelNumber());
  EXPECT_EQ(616, (int) objArray[4].getVoxelNumber());
  EXPECT_EQ(112, (int) objArray[5].getVoxelNumber());
  EXPECT_EQ(296, (int) objArray[6].getVoxelNumber());
  EXPECT_EQ(293, (int) objArray[7].getVoxelNumber());
  EXPECT_EQ(572, (int) objArray[8].getVoxelNumber());
  EXPECT_EQ(208, (int) objArray[9].getVoxelNumber());

  C_Stack::kill(stack);

  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/3d/diadem_e1.tif");
  obj.loadStack(stack);
  objArray = obj.getConnectedComponent();
  EXPECT_EQ(43, (int) objArray.size());
  EXPECT_EQ(2, (int) objArray[0].getVoxelNumber());
  EXPECT_EQ(68236, (int) objArray[1].getVoxelNumber());
  EXPECT_EQ(2, (int) objArray[2].getVoxelNumber());
  EXPECT_EQ(3, (int) objArray[3].getVoxelNumber());

  /*
  for (size_t i = 0; i < objArray.size(); ++i) {
    std::cout<< objArray[i].getVoxelNumber() << std::endl;
  }
*/
}

TEST(TestObject3dScan, duplicateAcrossZ)
{
  ZObject3dScan obj;
  obj.addSegment(0, 0, 1, 2);
  obj.duplicateAcrossZ(3);

  EXPECT_EQ(3, (int) obj.getStripeNumber());
  EXPECT_EQ(6, (int) obj.getVoxelNumber());

  obj.duplicateAcrossZ(2);
  EXPECT_EQ(2, (int) obj.getStripeNumber());
  EXPECT_EQ(4, (int) obj.getVoxelNumber());

  obj.addSegment(0, 1, 3, 4);
  obj.duplicateAcrossZ(3);

  EXPECT_EQ(6, (int) obj.getStripeNumber());
  EXPECT_EQ(12, (int) obj.getVoxelNumber());

  //obj.print();
}

TEST(TestObject3dScan, TestScanArray) {
  Stack *stack = C_Stack::readSc(GET_TEST_DATA_DIR +
                                 "/benchmark/binary/3d/diadem_e1.tif");

  std::map<int, ZObject3dScan*> *objSet = ZObject3dScan::extractAllObject(
        stack->array, C_Stack::width(stack), C_Stack::height(stack),
        C_Stack::depth(stack), 0, NULL);

  EXPECT_EQ(2, (int) objSet->size());
  EXPECT_TRUE((*objSet)[0]->isCanonizedActually());
  EXPECT_TRUE((*objSet)[1]->isCanonizedActually());

  (*objSet)[1]->save(GET_TEST_DATA_DIR + "/test.sobj");
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/test.sobj");
  EXPECT_TRUE(obj.isCanonizedActually());

  //obj.scanArray(array, )
}

TEST(TestObject3dScan, TestIO) {
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");

  EXPECT_TRUE(obj.isCanonizedActually());
}

TEST(ZObject3dScan, dilate) {
  ZObject3dScan obj;

  obj.addSegment(0, 0, 0, 1);

  obj.dilate();
  EXPECT_EQ(5, (int) obj.getStripeNumber());
  EXPECT_EQ(12, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 3, 4);

  obj.dilate();

  EXPECT_EQ(5, (int) obj.getStripeNumber());
  EXPECT_EQ(23, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 1, 0, 1);
  obj.dilate();
  //obj.print();
  EXPECT_EQ(8, (int) obj.getStripeNumber());
  EXPECT_EQ(20, (int) obj.getVoxelNumber());

  obj.clear();;
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 4, 5);
  obj.dilate();
  EXPECT_EQ(5, (int) obj.getStripeNumber());
  EXPECT_EQ(24, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, -1, 2, 3);
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 4, 5);
  obj.addSegment(0, 1, 2, 3);
  //obj.print();
  obj.dilate();
  //obj.print();
  EXPECT_EQ(11, (int) obj.getStripeNumber());
  EXPECT_EQ(13, (int) obj.getSegmentNumber());
  EXPECT_EQ(40, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(1, 0, 0, 1);
  obj.dilate();
  EXPECT_EQ(8, (int) obj.getStripeNumber());
  EXPECT_EQ(20, (int) obj.getVoxelNumber());
}

TEST(ZObject3dScan, overlap)
{
  Stack *stack = C_Stack::make(GREY, 3, 3, 3);
  Zero_Stack(stack);
  stack->array[0] = 1;

  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);

  EXPECT_EQ(1, (int) obj.countForegroundOverlap(stack));
}

class Object3dScanTestF1 : public ::testing::Test {
protected:
  virtual void SetUp() {
    m_obj.clear();
    m_obj.addSegment(0, 1, 0, 2);
    m_obj.addSegment(1, 0, 1, 1);
    m_obj.addSegment(1, 1, 0, 2);
    m_obj.addSegment(1, 2, 1, 1);
    m_obj.addSegment(2, 0, 1, 1);
    m_obj.addSegment(2, 1, 1, 1);
    m_obj.addSegment(2, 2, 1, 1);
  }

  virtual void TearDown() {}

  ZObject3dScan m_obj;
};

TEST_F(Object3dScanTestF1, Slice)
{
  ZObject3dScan obj = m_obj;
  obj.addSegment(0, 1, 0, 2);
  obj.addSegment(1, 0, 1, 1);
  obj.addSegment(1, 1, 0, 2);
  obj.addSegment(1, 2, 1, 1);
  obj.addSegment(2, 0, 1, 1);
  obj.addSegment(2, 1, 1, 1);
  obj.addSegment(2, 2, 1, 1);

  ZObject3dScan slice = obj.getSlice(0);
  EXPECT_EQ(3, (int) slice.getVoxelNumber());
  slice = obj.getSlice(0, 1);
  EXPECT_EQ(8, (int) slice.getVoxelNumber());

  slice = obj.getSlice(0, 2);
  EXPECT_EQ(11, (int) slice.getVoxelNumber());
}

TEST_F(Object3dScanTestF1, Statistics)
{
  ZObject3dScan obj = m_obj;
  obj.addSegment(0, 1, 0, 2);
  obj.addSegment(1, 0, 1, 1);
  obj.addSegment(1, 1, 0, 2);
  obj.addSegment(1, 2, 1, 1);
  obj.addSegment(2, 0, 1, 1);
  obj.addSegment(2, 1, 1, 1);
  obj.addSegment(2, 2, 1, 1);

  ZPoint center = obj.getCentroid();
  EXPECT_DOUBLE_EQ(1.0, center.x());
  EXPECT_DOUBLE_EQ(1.0, center.y());
  EXPECT_DOUBLE_EQ(1.0, center.z());
}

TEST_F(Object3dScanTestF1, equal)
{
  ZObject3dScan obj = m_obj;
  EXPECT_TRUE(m_obj.equalsLiterally(obj));
  obj.canonize();
  EXPECT_TRUE(m_obj.equalsLiterally(obj));

  obj.addSegment(2, 1, 1, 1, false);
  EXPECT_FALSE(m_obj.equalsLiterally(obj));

  obj.canonize();
  EXPECT_TRUE(m_obj.equalsLiterally(obj));
}

#endif

#endif // ZOBJECT3DSCANTEST_H
