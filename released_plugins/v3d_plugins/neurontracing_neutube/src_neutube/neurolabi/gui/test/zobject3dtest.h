#ifndef ZOBJECT3DTEST_H
#define ZOBJECT3DTEST_H

#include "ztestheader.h"
#include "zobject3d.h"
#include "neutubeconfig.h"

#ifdef _USE_GTEST_

TEST(ZObject3d, duplicateAcrossZ) {
  ZObject3d obj;
  obj.append(1, 1, 0);
  obj.duplicateAcrossZ(5);
  EXPECT_EQ(5, (int) obj.size());
  EXPECT_EQ(1, obj.x(0));
  EXPECT_EQ(1, obj.y(0));
  EXPECT_EQ(0, obj.z(0));
  EXPECT_EQ(1, obj.x(1));
  EXPECT_EQ(1, obj.y(1));
  EXPECT_EQ(1, obj.z(1));


}

#endif


#endif // ZOBJECT3DTEST_H
