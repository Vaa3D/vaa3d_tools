#ifndef ZCUBOIDTEST_H
#define ZCUBOIDTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zintcuboidarray.h"
#include "flyem/zintcuboidcomposition.h"
#ifdef __GLIBCXX__
#include <tr1/memory>
#else
#include <memory>
#endif

#ifdef _USE_GTEST_

TEST(ZIntCuboidArray, basic)
{
  FlyEm::ZIntCuboidArray blockArray;

  blockArray.append(0, 0, 0, 500, 500, 500);
  blockArray.append(500, 500, 500, 500, 500, 500);

  EXPECT_EQ(2, (int) blockArray.size());

  int index = blockArray.hitTest(1, 1, 1);
  EXPECT_EQ(0, index);

  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/benchmark/flyem/block.txt");
  EXPECT_EQ(216, (int) blockArray.size());

  for (size_t i = 0; i < blockArray.size(); ++i) {
    int width, height, depth;
    Cuboid_I_Size(&(blockArray[i]), &width, &height, &depth);
    EXPECT_EQ(500, width);
    EXPECT_EQ(500, height);
    EXPECT_EQ(500, depth);
  }

  blockArray.translate(100, 200, 300);
  for (size_t i = 0; i < blockArray.size(); ++i) {
    int width, height, depth;
    Cuboid_I_Size(&(blockArray[i]), &width, &height, &depth);
    EXPECT_EQ(500, width);
    EXPECT_EQ(500, height);
    EXPECT_EQ(500, depth);
  }
}

TEST(ZIntCuboidArray, boundBox)
{
  FlyEm::ZIntCuboidArray blockArray;

  blockArray.append(10, 20, 30, 10, 10, 10);
  Cuboid_I boundBox = blockArray.getBoundBox();
  EXPECT_EQ(10, boundBox.cb[0]);
  EXPECT_EQ(20, boundBox.cb[1]);
  EXPECT_EQ(30, boundBox.cb[2]);
  EXPECT_EQ(19, boundBox.ce[0]);
  EXPECT_EQ(29, boundBox.ce[1]);
  EXPECT_EQ(39, boundBox.ce[2]);

  blockArray.append(20, 30, 40, 10, 10, 10);
  boundBox = blockArray.getBoundBox();
  EXPECT_EQ(10, boundBox.cb[0]);
  EXPECT_EQ(20, boundBox.cb[1]);
  EXPECT_EQ(30, boundBox.cb[2]);
  EXPECT_EQ(29, boundBox.ce[0]);
  EXPECT_EQ(39, boundBox.ce[1]);
  EXPECT_EQ(49, boundBox.ce[2]);
}

TEST(ZIntCuboidArray, exportSwc)
{
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/benchmark/flyem/block.txt");

  //blockArray.exportSwc(GET_TEST_DATA_DIR + "/test.swc");
}

TEST(ZIntCuboidArray, removeInvalidCublid)
{
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.append(1, 1, 1, 0, 0, 0);
  blockArray.removeInvalidCuboid();
  EXPECT_EQ(0, (int) blockArray.size());

  blockArray.append(0, 0, 0, 1, 1, 1);
  blockArray.append(0, 0, 0, 1, 1, 1);
  blockArray.append(1, 1, 1, 0, 0, 0);
  blockArray.append(1, 1, 1, 0, 0, 0);
  blockArray.append(0, 0, 0, 1, 1, 1);
  blockArray.append(0, 0, 0, 1, 1, 1);
  blockArray.removeInvalidCuboid();
  EXPECT_EQ(4, (int) blockArray.size());
}

TEST(ZIntCuboidArray, range)
{
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/benchmark/flyem/block.txt");

  Cuboid_I boundBox = blockArray.getBoundBox();
  //std::cout << blockArray.size() << std::endl;

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);
  //blockArray.print();
  EXPECT_EQ(108, (int) blockArray.size());

  boundBox.ce[2] = 2999;
  blockArray.intersect(boundBox);
  EXPECT_EQ(54, (int) blockArray.size());

  boundBox = blockArray.getBoundBox();
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);

  boundBox = blockArray.getBoundBox();
  EXPECT_EQ(0, boundBox.cb[0]);
  EXPECT_EQ(0, boundBox.cb[1]);
  EXPECT_EQ(0, boundBox.cb[2]);
}

TEST(ZIntCuboidArray, face)
{
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.append(0, 0, 0, 3, 3, 3);
  FlyEm::ZIntCuboidArray face = blockArray.getFace();

  EXPECT_EQ(6, (int) face.size());

  FlyEm::ZIntCuboidArray face2 = blockArray.getInnerFace();
  EXPECT_TRUE(face2.empty());

  blockArray.append(3, 0, 0, 3, 3, 3);
  face = blockArray.getFace();
  EXPECT_EQ(12, (int) face.size());

  face2 = blockArray.getInnerFace();
  //face2.print();
  EXPECT_EQ(2, (int) face2.size());
  Cuboid_I cuboid = face2[0];
  EXPECT_EQ(3, cuboid.cb[0]);
  EXPECT_EQ(3, cuboid.ce[0]);
  EXPECT_EQ(0, cuboid.cb[1]);
  EXPECT_EQ(2, cuboid.ce[1]);
  EXPECT_EQ(0, cuboid.cb[2]);
  EXPECT_EQ(2, cuboid.ce[2]);

  blockArray.append(1, 3, 0, 4, 3, 3);
  face = blockArray.getFace();
  EXPECT_EQ(18, (int) face.size());

  face2 = blockArray.getInnerFace();
  EXPECT_EQ(6, (int) face2.size());
}

TEST(ZIntCuboidComposition, hitTest)
{
  FlyEm::ZIntCuboidComposition cuboid;
  cuboid.setSingular(0, 0, 0, 3, 3, 3);

  EXPECT_TRUE(cuboid.hitTest(0, 0, 0));
  EXPECT_TRUE(cuboid.hitTest(1, 1, 1));
  EXPECT_TRUE(cuboid.hitTest(2, 2, 2));
  EXPECT_FALSE(cuboid.hitTest(3, 3, 3));

  std::tr1::shared_ptr<FlyEm::ZIntCuboidComposition> comp1(
        new FlyEm::ZIntCuboidComposition);
  comp1->setSingular(0, 0, 0, 3, 3, 3);

  std::tr1::shared_ptr<FlyEm::ZIntCuboidComposition> comp2(
        new FlyEm::ZIntCuboidComposition);
  comp2->setSingular(0, 0, 0, 3, 3, 3);

  std::tr1::shared_ptr<FlyEm::ZIntCuboidComposition> comp3(
        new FlyEm::ZIntCuboidComposition);
  comp3->setComposition(comp1, comp2, FlyEm::ZIntCuboidComposition::OR);
  EXPECT_TRUE(comp3->hitTest(0, 0, 0));
  EXPECT_TRUE(comp3->hitTest(1, 1, 1));
  EXPECT_TRUE(comp3->hitTest(2, 2, 2));
  EXPECT_FALSE(comp3->hitTest(3, 3, 3));

  comp3->setComposition(comp1, comp2, FlyEm::ZIntCuboidComposition::AND);
  EXPECT_TRUE(comp3->hitTest(0, 0, 0));
  EXPECT_TRUE(comp3->hitTest(1, 1, 1));
  EXPECT_TRUE(comp3->hitTest(2, 2, 2));
  EXPECT_FALSE(comp3->hitTest(3, 3, 3));

  comp3->setComposition(comp1, comp2, FlyEm::ZIntCuboidComposition::XOR);
  EXPECT_FALSE(comp3->hitTest(0, 0, 0));
  EXPECT_FALSE(comp3->hitTest(1, 1, 1));
  EXPECT_FALSE(comp3->hitTest(2, 2, 2));
  EXPECT_FALSE(comp3->hitTest(3, 3, 3));


}

#endif

#endif // ZCUBOIDTEST_H
