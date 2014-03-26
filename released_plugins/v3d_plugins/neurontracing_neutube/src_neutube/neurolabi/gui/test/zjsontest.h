#ifndef ZJSONTEST_H
#define ZJSONTEST_H

#include "ztestheader.h"
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "c_json.h"

#ifdef _USE_GTEST_

TEST(Json, basic)
{
  ZJsonObject obj;
  ASSERT_TRUE(obj.isEmpty());

  obj.setEntry("test1", 10);
  ASSERT_EQ(10, ZJsonParser::integerValue(obj["test1"]));
  obj.setEntry("test2", true);
  ASSERT_EQ(true, ZJsonParser::booleanValue(obj["test2"]));

  obj.print();
}

#endif

#endif // ZJSONTEST_H
