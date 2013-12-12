#ifndef ZFLYEMNEURONRANGETEST_H
#define ZFLYEMNEURONRANGETEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyemneuronrange.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmNeuronRange, Basic)
{
  ZFlyEmNeuronRange range;
  range.setPlaneRange(0.0, 5.0);
  range.setPlaneRange(10.0, 20.0);
  range.setPlaneRange(100.0, 30.0);

  EXPECT_TRUE(range.contains(0.0, 0.0, 0.0));
  EXPECT_TRUE(range.contains(0.0, 0.0, 50.0));
  EXPECT_FALSE(range.contains(0.0, 0.0, 101.0));
  EXPECT_FALSE(range.contains(0.0, 40.0, 50.0));
  EXPECT_TRUE(range.contains(0.0, 25.0, 70.0));
}

#endif

#endif // ZFLYEMNEURONRANGETEST_H
