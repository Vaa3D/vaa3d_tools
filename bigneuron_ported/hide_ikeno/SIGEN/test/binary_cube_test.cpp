#include "sigen/common/binary_cube.h"
#include <gtest/gtest.h>
TEST(BinaryCube, init) {
  sigen::BinaryCube cube(2, 3, 4);
  EXPECT_EQ(2, cube.x_);
  EXPECT_EQ(3, cube.y_);
  EXPECT_EQ(4, cube.z_);
}
TEST(BinaryCube, read_write) {
  sigen::BinaryCube cube(2, 3, 4);
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 4; ++k) {
        EXPECT_FALSE(cube[i][j][k]);
      }
    }
  }
  cube[1][2][3] = true;
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 4; ++k) {
        if (i == 1 && j == 2 && k == 3)
          EXPECT_TRUE(cube[i][j][k]);
        else
          EXPECT_FALSE(cube[i][j][k]);
      }
    }
  }
}
