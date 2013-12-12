#ifndef ZSTITCHGRIDTEST_H
#define ZSTITCHGRIDTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zstitchgrid.h"

#ifdef _USE_GTEST_
TEST(ZStitchGrid, Basic) {
  FlyEm::ZStitchGrid grid;

  grid.setGridNumber(3, 3, 3);
  grid.setInterval(5, 5, 5);
  grid.setStart(0, 0, 0);

  EXPECT_EQ(8, grid.getBlockNumber());

  Cuboid_I cuboid;
  grid.getBlock(0, &cuboid);
  EXPECT_EQ(0, cuboid.cb[0]);
  EXPECT_EQ(0, cuboid.cb[1]);
  EXPECT_EQ(0, cuboid.cb[2]);
  EXPECT_EQ(4, cuboid.ce[0]);
  EXPECT_EQ(4, cuboid.ce[1]);
  EXPECT_EQ(4, cuboid.ce[2]);

  grid.getBlock(1, &cuboid);
  EXPECT_EQ(5, cuboid.cb[0]);
  EXPECT_EQ(0, cuboid.cb[1]);
  EXPECT_EQ(0, cuboid.cb[2]);
  EXPECT_EQ(9, cuboid.ce[0]);
  EXPECT_EQ(4, cuboid.ce[1]);
  EXPECT_EQ(4, cuboid.ce[2]);

  grid.getBlock(2, &cuboid);
  EXPECT_EQ(0, cuboid.cb[0]);
  EXPECT_EQ(5, cuboid.cb[1]);
  EXPECT_EQ(0, cuboid.cb[2]);
  EXPECT_EQ(4, cuboid.ce[0]);
  EXPECT_EQ(9, cuboid.ce[1]);
  EXPECT_EQ(4, cuboid.ce[2]);

  grid.getBlock(3, &cuboid);
  EXPECT_EQ(5, cuboid.cb[0]);
  EXPECT_EQ(5, cuboid.cb[1]);
  EXPECT_EQ(0, cuboid.cb[2]);
  EXPECT_EQ(9, cuboid.ce[0]);
  EXPECT_EQ(9, cuboid.ce[1]);
  EXPECT_EQ(4, cuboid.ce[2]);

  grid.getBlock(4, &cuboid);
  EXPECT_EQ(0, cuboid.cb[0]);
  EXPECT_EQ(0, cuboid.cb[1]);
  EXPECT_EQ(5, cuboid.cb[2]);
  EXPECT_EQ(4, cuboid.ce[0]);
  EXPECT_EQ(4, cuboid.ce[1]);
  EXPECT_EQ(9, cuboid.ce[2]);

  int index = grid.hitTest(7, 7, 1);
  EXPECT_EQ(3, index);
  EXPECT_EQ(0, grid.hitTest(0, 0, 0));
  EXPECT_EQ(-1, grid.hitTest(-0.1, 0, 0));
  EXPECT_EQ(0, grid.hitTest(4.9, 4.9, 4.9));
  EXPECT_EQ(3, grid.hitTest(5.0, 5.0, 3));
  EXPECT_EQ(1, grid.hitTest(5.0, 3, 3));


  grid.setStart(10, 10, 10);
  grid.getBlock(0, &cuboid);
  EXPECT_EQ(10, cuboid.cb[0]);
  EXPECT_EQ(10, cuboid.cb[1]);
  EXPECT_EQ(10, cuboid.cb[2]);
  EXPECT_EQ(14, cuboid.ce[0]);
  EXPECT_EQ(14, cuboid.ce[1]);
  EXPECT_EQ(14, cuboid.ce[2]);

}

#endif

#endif // ZSTITCHGRIDTEST_H
