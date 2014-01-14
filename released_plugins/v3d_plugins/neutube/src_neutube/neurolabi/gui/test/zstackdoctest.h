#ifndef ZSTACKDOCTEST_H
#define ZSTACKDOCTEST_H

#include "ztestheader.h"
#include "zstackdoc.h"
#include "neutubeconfig.h"

#ifdef _USE_GTEST_
TEST(ZStackDoc, Basic)
{
  ZStackDoc doc(NULL, NULL);
  EXPECT_TRUE(doc.isEmpty());
  EXPECT_FALSE(doc.hasStackData());
  EXPECT_FALSE(doc.hasStackMask());

  EXPECT_TRUE(doc.stackSourcePath().isEmpty());

}

TEST(ZStackDoc, Swc)
{
  ZStackDoc doc(NULL, NULL);
  doc.readSwc((GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/1.swc").c_str());
  doc.saveSwc(GET_TEST_DATA_DIR + "/test1.swc");
  ASSERT_EQ(1, doc.getSwcList().size());
  ASSERT_EQ(GET_TEST_DATA_DIR + "/test1.swc", doc.getSwcList().front()->source());

  ZSwcTree *tree = new ZSwcTree;
  tree->load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/2.swc");
  doc.addSwcTree(tree);
  ASSERT_EQ(2, doc.getSwcList().size());

  doc.saveSwc(GET_TEST_DATA_DIR + "/test2.swc");
  ASSERT_EQ(1, doc.getSwcList().size());
  ASSERT_EQ(GET_TEST_DATA_DIR + "/test2.swc", doc.getSwcList().front()->source());
}

#endif

#endif // ZSTACKDOCTEST_H
