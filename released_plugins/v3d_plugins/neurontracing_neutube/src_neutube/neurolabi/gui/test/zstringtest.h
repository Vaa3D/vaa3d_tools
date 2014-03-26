#ifndef ZSTRINGTEST_H
#define ZSTRINGTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstring.h"
#include "biocytin/zbiocytinfilenameparser.h"

#ifdef _USE_GTEST_

TEST(ZString, extractWord) {
  ZString str = "\"This\" is a test";
  EXPECT_EQ("This", str.firstQuotedWord());

  str = "\"This is a test";
  EXPECT_EQ("", str.firstQuotedWord());

  str = "This\" is a test";
  EXPECT_EQ("", str.firstQuotedWord());

  str = "\"This\" \"is\" a test";
  EXPECT_EQ("This", str.firstQuotedWord());

  str = "This \"is\" a test";
  EXPECT_EQ("is", str.firstQuotedWord());

  str = "This \"\"is a test";
  EXPECT_EQ("", str.firstQuotedWord());
}

TEST(ZString, ParseFileName)
{
  std::string str = "/Users/foo/test/DH070613-1-2.tif";

  EXPECT_EQ("DH070613-1-2.tif", ZString::getBaseName(str));
  EXPECT_EQ("DH070613-1-2",
            ZString::removeFileExt(ZString::getBaseName(str)));
}

TEST(ZBiocytinFileNameParser, Basic)
{
  std::string str = "/Users/foo/test/DH070613-1-2.tif";

  EXPECT_EQ("DH070613-1-2", ZBiocytinFileNameParser::getCoreName(str));

  EXPECT_EQ("DH070613-1-2",
            ZBiocytinFileNameParser::getCoreName("DH070613-1-2.tif"));
  EXPECT_EQ("DH070613-1-2",
            ZBiocytinFileNameParser::getCoreName("DH070613-1-2.Edit.tif"));
  EXPECT_EQ("DH070613-1-2",
            ZBiocytinFileNameParser::getCoreName("DH070613-1-2.edit.tif"));
  EXPECT_EQ("DH070613-1-2",
            ZBiocytinFileNameParser::getCoreName("DH070613-1-2.proj.tif"));
  EXPECT_EQ("DH070613-1-2",
            ZBiocytinFileNameParser::getCoreName("DH070613-1-2.roi.tif"));
  EXPECT_EQ("DH070613-1-2",
            ZBiocytinFileNameParser::getCoreName("DH070613-1-2.ROI.tif"));
  EXPECT_EQ("DH070613-1-2",
            ZBiocytinFileNameParser::getCoreName("DH070613-1-2.mask.tif"));
  EXPECT_EQ("DH070613-1-2",
            ZBiocytinFileNameParser::getCoreName("DH070613-1-2.Mask.tif"));


  EXPECT_EQ(ZBiocytinFileNameParser::ORIGINAL,
            ZBiocytinFileNameParser::getRole("DH070613-1-2.tif"));
  EXPECT_EQ(ZBiocytinFileNameParser::EDIT,
            ZBiocytinFileNameParser::getRole("DH070613-1-2.Edit.tif"));
  EXPECT_EQ(ZBiocytinFileNameParser::EDIT,
            ZBiocytinFileNameParser::getRole("DH070613-1-2.edit.tif"));
  EXPECT_EQ(ZBiocytinFileNameParser::PROJECTION,
            ZBiocytinFileNameParser::getRole("DH070613-1-2.proj.tif"));
  EXPECT_EQ(ZBiocytinFileNameParser::ROI,
            ZBiocytinFileNameParser::getRole("DH070613-1-2.roi.tif"));
  EXPECT_EQ(ZBiocytinFileNameParser::ROI,
            ZBiocytinFileNameParser::getRole("DH070613-1-2.ROI.tif"));
  EXPECT_EQ(ZBiocytinFileNameParser::MASK,
            ZBiocytinFileNameParser::getRole("DH070613-1-2.mask.tif"));
  EXPECT_EQ(ZBiocytinFileNameParser::MASK,
            ZBiocytinFileNameParser::getRole("DH070613-1-2.Mask.tif"));
}

#endif

#endif // ZSTRINGTEST_H
