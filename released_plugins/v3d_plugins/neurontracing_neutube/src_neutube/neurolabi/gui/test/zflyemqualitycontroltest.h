#ifndef ZFLYEMQUALITYCONTROLTEST_H
#define ZFLYEMQUALITYCONTROLTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyemqualityanalyzer.h"
#include "flyem/zintcuboidarray.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmQualityAnalyzer, isStitchedOrphanBody)
{
  ZObject3dScan obj;
  obj.addSegment(0, 1, 3, 3);
  obj.addSegment(0, 2, 2, 2);
  obj.addSegment(0, 3, 1, 1);

  FlyEm::ZIntCuboidArray roi;
  roi.append(0, 0, 0, 5, 5, 1);
  roi.append(5, 0, 0, 5, 5, 1);

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(roi);
  EXPECT_FALSE(analyzer.isStitchedOrphanBody(obj));

  roi.clear();
  roi.append(0, 0, -1, 5, 5, 3);
  roi.append(5, 0, -1, 5, 5, 3);
  analyzer.setSubstackRegion(roi);

  obj.addSegment(0, 1, 5, 5);
  obj.addSegment(0, 2, 6, 6);
  EXPECT_TRUE(analyzer.isStitchedOrphanBody(obj));

  obj.addSegment(0, 3, 0, 0);
  EXPECT_FALSE(analyzer.isStitchedOrphanBody(obj));

  obj.clear();
  obj.addSegment(0, 2, 1, 3);
  obj.addSegment(0, 3, 3, 3);
  obj.addSegment(0, 3, 3, 3);
  obj.addSegment(0, 5, 4, 4);
  obj.addSegment(0, 6, 4, 4);
  obj.addSegment(0, 7, 4, 6);

  roi.clear();
  roi.append(0, 0, -1, 5, 5, 3);
  roi.append(3, 5, -1, 5, 5, 3);
  analyzer.setSubstackRegion(roi);
  EXPECT_TRUE(analyzer.isStitchedOrphanBody(obj));

  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/benchmark/block.txt");
  EXPECT_EQ(216, (int) blockArray.size());

  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 2999;
  blockArray.intersect(boundBox);
  EXPECT_EQ(54, (int) blockArray.size());

  boundBox = blockArray.getBoundBox();

  EXPECT_EQ(2469, boundBox.cb[0]);
  EXPECT_EQ(2232, boundBox.cb[1]);
  EXPECT_EQ(1500, boundBox.cb[2]);

  //blockArray.print();

  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);

  //blockArray.print();

  boundBox = blockArray.getBoundBox();
  EXPECT_EQ(0, boundBox.cb[0]);
  EXPECT_EQ(0, boundBox.cb[1]);
  EXPECT_EQ(0, boundBox.cb[2]);

  blockArray.translate(0, 0, 10);


  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");
  analyzer.setSubstackRegion(blockArray);
  //EXPECT_FALSE(analyzer.isStitchedOrphanBody(obj));


  /*
  blockArray.translate(0, 0, -10);
  blockArray.rescale(0.5);
  blockArray.exportSwc(GET_TEST_DATA_DIR + "/flyem/FIB/skeletonization/session9/block.swc");
  */

  roi.clear();
  roi.append(0, 0, 0, 5, 5, 5);
  roi.append(0, 0, 5, 3, 3, 3);

  obj.clear();
  obj.addSegment(2, 1, 1, 1);
  obj.addSegment(3, 1, 1, 1);
  obj.addSegment(4, 1, 1, 1);
  obj.addSegment(5, 1, 1, 1);
  obj.addSegment(6, 1, 1, 1);

  analyzer.setSubstackRegion(roi);
  EXPECT_TRUE(analyzer.isStitchedOrphanBody(obj));
  EXPECT_FALSE(analyzer.isOrphanBody(obj));

  obj.clear();
  obj.addSegment(2, 1, 1, 2);
  obj.addSegment(3, 1, 1, 2);
  obj.addSegment(4, 1, 1, 2);
  obj.addSegment(5, 1, 1, 2);
  obj.addSegment(6, 1, 1, 2);
  EXPECT_FALSE(analyzer.isStitchedOrphanBody(obj));
}



#endif

#endif // ZFLYEMQUALITYCONTROLTEST_H
