#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <gtest/gtest.h>
#include <utility>
#include <vector>
TEST(boost_shared_ptr, null) {
  boost::shared_ptr<int> p;
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE((bool)p);
}
TEST(boost_shared_ptr, equal) {
  boost::shared_ptr<int> p = boost::make_shared<int>(334);
  boost::shared_ptr<int> q = p;
  EXPECT_EQ(p, q);
}
TEST(boost_shared_ptr, ref_count) {
  boost::shared_ptr<int> a;
  {
    boost::shared_ptr<int> b = boost::make_shared<int>(334);
    a = b;
  }
  EXPECT_TRUE((bool)a);
}
TEST(weak_ptr, ref_count) {
  boost::weak_ptr<int> a;
  {
    a = boost::make_shared<int>(334);
  }
  EXPECT_TRUE(a.expired());
}
struct S {
  int value_;
  boost::weak_ptr<S> next_;
  S(int value, boost::weak_ptr<S> next) : value_(value), next_(next) {}
  S(int value) : value_(value) {}
};
TEST(cyclic_ptr, release) {
  boost::weak_ptr<S> p, q;
  {
    std::vector<boost::shared_ptr<S> > owner;
    owner.push_back(boost::make_shared<S>(123));
    owner.push_back(boost::make_shared<S>(456));
    owner[0]->next_ = owner[1];
    owner[1]->next_ = owner[0];
    p = owner[0];
    q = owner[1];
    EXPECT_EQ(456, p.lock()->next_.lock()->value_);
    EXPECT_EQ(123, q.lock()->next_.lock()->value_);
  }
  EXPECT_TRUE(p.expired());
  EXPECT_TRUE(q.expired());
}
