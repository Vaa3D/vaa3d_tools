#include "sigen/writer/fileutils.h"
#include <gtest/gtest.h>
std::string add(const std::string &a, const std::string &b) {
  return sigen::FileUtils::AddExtension(a, b);
}
TEST(FileUtils, AddExtension) {
  EXPECT_EQ("hello.swc", add("hello", "swc"));
  EXPECT_EQ("hello.out.swc", add("hello.out", "swc"));
  EXPECT_EQ("hello.swc", add("hello.swc", "swc"));
  EXPECT_EQ("hello.swc", add("hello", ".swc"));
  EXPECT_EQ("hello.out.swc", add("hello.out", ".swc"));
  EXPECT_EQ("hello.swc", add("hello.swc", ".swc"));
}
