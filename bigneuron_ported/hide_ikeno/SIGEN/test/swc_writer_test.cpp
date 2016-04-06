#include "sigen/writer/swc_writer.h"
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
void connect(boost::shared_ptr<sigen::NeuronNode> a, boost::shared_ptr<sigen::NeuronNode> b) {
  a->AddConnection(b.get());
  b->AddConnection(a.get());
}
TEST(SwcWriter, write) {
  sigen::NeuronNodePtr a, b, c, d, e;
  a = boost::make_shared<sigen::NeuronNode>();
  a->id_ = 1;
  a->setCoord(1.1, 1.2, 1.3);
  a->radius_ = 1.4;
  a->type_ = sigen::NeuronType::EDGE;
  b = boost::make_shared<sigen::NeuronNode>();
  b->id_ = 2;
  b->setCoord(2.1, 2.2, 2.3);
  b->radius_ = 2.4;
  b->type_ = sigen::NeuronType::BRANCH;
  c = boost::make_shared<sigen::NeuronNode>();
  c->id_ = 3;
  c->setCoord(3.1, 3.2, 3.3);
  c->radius_ = 3.4;
  c->type_ = sigen::NeuronType::CONNECT;
  d = boost::make_shared<sigen::NeuronNode>();
  d->id_ = 4;
  d->setCoord(4.1, 4.2, 4.3);
  d->radius_ = 4.4;
  d->type_ = sigen::NeuronType::EDGE;
  e = boost::make_shared<sigen::NeuronNode>();
  e->id_ = 5;
  e->setCoord(5.1, 5.2, 5.3);
  e->radius_ = 5.4;
  e->type_ = sigen::NeuronType::EDGE;
  connect(a, b);
  connect(b, c);
  connect(c, d);
  connect(b, e);
  sigen::Neuron n;
  n.storage_.push_back(a);
  n.storage_.push_back(b);
  n.storage_.push_back(c);
  n.storage_.push_back(d);
  n.set_root(a.get());

  std::stringstream ss;
  sigen::SwcWriter w;
  w.Write(ss, n);

  std::set<std::string> file_content;
  file_content.insert("1 6 1.1 1.2 1.3 1.4 -1");
  file_content.insert("2 5 2.1 2.2 2.3 2.4 1");
  file_content.insert("3 3 3.1 3.2 3.3 3.4 2");
  file_content.insert("4 6 4.1 4.2 4.3 4.4 3");
  file_content.insert("5 6 5.1 5.2 5.3 5.4 2");

  for (int i = 0; i < 5; ++i) {
    std::string line;
    EXPECT_TRUE((bool)std::getline(ss, line));
    EXPECT_TRUE((bool)file_content.count(line));
    file_content.erase(line);
  }
  EXPECT_TRUE(file_content.empty());
}
