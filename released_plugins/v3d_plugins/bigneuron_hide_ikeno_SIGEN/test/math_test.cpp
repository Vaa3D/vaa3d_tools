#include <gtest/gtest.h>
#include <sigen/common/math.h>
TEST(Math, Mean) {
  std::vector<double> xs;
  xs.push_back(1.0);
  xs.push_back(2.0);
  xs.push_back(3.0);
  EXPECT_EQ(2, sigen::Mean(xs));
}
