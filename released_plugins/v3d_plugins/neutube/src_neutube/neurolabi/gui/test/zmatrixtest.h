#ifndef ZMATRIXTEST_H
#define ZMATRIXTEST_H

#include "ztestheader.h"
#include "zmatrix.h"

#ifdef _USE_GTEST_
TEST(ZMatrix, copyValue)
{
  ZMatrix mat(5, 3);
  std::vector<double> array(10, 0);

  for (int i = 0; i < 15; ++i) {
    mat.set(i, i);
  }

  for (int i = 0; i < 15; ++i) {
    ASSERT_EQ(i, mat.getValue(i));
  }

  ASSERT_EQ(1, mat.copyRowValue(1, 2, 4, &(array[1])));
  ASSERT_EQ(0.0, array[0]);
  ASSERT_EQ(5.0, array[1]);
  ASSERT_EQ(0.0, array[2]);
  ASSERT_EQ(0.0, array[3]);
  ASSERT_EQ(0.0, array[4]);

  ASSERT_EQ(3, mat.copyRowValue(1, -1, 2, &(array[1])));
  ASSERT_EQ(0.0, array[0]);
  ASSERT_EQ(5.0, array[1]);
  ASSERT_EQ(3.0, array[2]);
  ASSERT_EQ(4.0, array[3]);
  ASSERT_EQ(5.0, array[4]);
  ASSERT_EQ(0.0, array[5]);
}

#endif

#endif // ZMATRIXTEST_H
