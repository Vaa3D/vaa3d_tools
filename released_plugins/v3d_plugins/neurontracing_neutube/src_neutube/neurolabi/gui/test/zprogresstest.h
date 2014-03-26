#ifndef ZPROGRESSTEST_H
#define ZPROGRESSTEST_H

#include "ztestheader.h"
#include "zprogressreporter.h"
#include "neutubeconfig.h"

#ifdef _USE_GTEST_

TEST(ZProgressReporter, Advance)
{
  ZProgressReporter reporter;
  reporter.start();

  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.0);

  reporter.advance(0.1);

  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.1);

  reporter.startSubprogress(0.5);
  reporter.start();
  for (int i = 0; i < 10; ++i) {
    reporter.advance(0.1);
  }
  reporter.end();
  reporter.endSubprogress(0.5);

  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.6);

  reporter.advance(0.1);

  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.7);

  reporter.end();


  reporter.start();
  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.0);

  reporter.advance(0.1);
  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.1);

  reporter.start(0.5);
  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.1);
  reporter.start();
  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.1);
  for (int i = 0; i < 10; ++i) {
    reporter.advance(0.1);
  }
  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.6);
  reporter.end();
  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.6);
  reporter.end(0.5);
  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.6);

  reporter.advance(0.1);
  ASSERT_DOUBLE_EQ(reporter.getProgress(), 0.7);

  reporter.end();
}

#endif

#endif // ZPROGRESSTEST_H
