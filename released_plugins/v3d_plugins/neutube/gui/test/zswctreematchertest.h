#ifndef ZSWCTREEMATCHERTEST_H
#define ZSWCTREEMATCHERTEST_H

#include "ztestheader.h"
#include "zswctreematcher.h"
#include "neutubeconfig.h"

#ifdef _USE_GTEST_

TEST(ZSwcTreeMatcher, dynamicProgramming) {
  ZMatrix simMat(3, 3);
  simMat.set(0, 0, 1);
  simMat.set(1, 1, 1);
  simMat.set(2, 2, 1);

  ZSwcTreeMatcher matcher;
  std::map<int, int> match = matcher.dynamicProgrammingMatch(simMat, 0.5);
  EXPECT_EQ(0, match[0]);
  EXPECT_EQ(1, match[1]);
  EXPECT_EQ(2, match[2]);

  simMat.set(0, 1, 3.0);
  simMat.debugOutput();
  match = matcher.dynamicProgrammingMatch(simMat, 0.5);
  EXPECT_EQ(3, (int) match.size());
  EXPECT_EQ(1, match[0]);
  EXPECT_EQ(1, match[1]);
  EXPECT_EQ(2, match[2]);
}

TEST(ZSwcTreeMatcher, match)
{

}

#endif

#endif // ZSWCTREEMATCHERTEST_H
