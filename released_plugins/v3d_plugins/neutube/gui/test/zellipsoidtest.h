#ifndef ZELLIPSOIDTEST_H
#define ZELLIPSOIDTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zellipsoid.h"

#ifdef _USE_GTEST_

TEST(ZEllipsoid, containsPoint)
{
  ZEllipsoid ellipsoid;
  ellipsoid.setCenter(0, 0, 0);
  ellipsoid.setSize(1, 1, 1);
  EXPECT_TRUE(ellipsoid.containsPoint(0, 0, 0));
  EXPECT_TRUE(ellipsoid.containsPoint(0.5, 0.5, 0.0));
  EXPECT_FALSE(ellipsoid.containsPoint(1.0, 1.0, 1.0));

  ellipsoid.setCenter(1, 2, 3);
  EXPECT_TRUE(ellipsoid.containsPoint(1, 2, 3));
  EXPECT_TRUE(ellipsoid.containsPoint(1.5, 2.5, 3.0));
  EXPECT_FALSE(ellipsoid.containsPoint(2.0, 3.0, 4.0));
}

#endif

#endif // ZELLIPSOIDTEST_H
