#include "sigen/common/variant.h"
#include <gtest/gtest.h>
#include <iostream>
TEST(variant, bool) {
  sigen::Variant v;
  v = true;
  ASSERT_EQ(typeid(bool), v.type());
  EXPECT_TRUE(boost::get<bool>(v));
}
TEST(variant, int) {
  sigen::Variant v;
  v = 123;
  ASSERT_EQ(typeid(int), v.type());
  EXPECT_EQ(123, boost::get<int>(v));
}
TEST(variant, double) {
  sigen::Variant v;
  v = 12.3;
  ASSERT_EQ(typeid(double), v.type());
  EXPECT_DOUBLE_EQ(12.3, boost::get<double>(v));
}
TEST(variant, string) {
  sigen::Variant v;
  // BAD NEWS
  // `v = "let's boost";` cause v.type() == typeid(bool)
  v = std::string("let's boost");
  ASSERT_NE(typeid(bool), v.type());
  ASSERT_EQ(typeid(std::string), v.type());
  EXPECT_EQ("let's boost", boost::get<std::string>(v));
}
