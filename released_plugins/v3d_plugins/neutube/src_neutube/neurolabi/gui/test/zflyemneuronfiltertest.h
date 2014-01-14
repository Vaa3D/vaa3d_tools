#ifndef ZFLYEMNEURONFILTERTEST_H
#define ZFLYEMNEURONFILTERTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyemneuronfilter.h"
#include "flyem/zflyemneuronfilterfactory.h"
#include "flyem/zflyemdatabundle.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmNeuronFilter, Factory)
{
  ZFlyEmNeuronFilterFactory factory;
  ZFlyEmNeuronFilter *filter = factory.createFilter(
        ZFlyEmNeuronFilterFactory::COMPOSITE);

  ZFlyEmNeuronFilter *subfilter = factory.createFilter(
        ZFlyEmNeuronFilterFactory::LAYER);

  ZJsonObject config;
  config.setEntry("start", 100.0);
  config.setEntry("length", 10000.0);
  config.setEntry("top", 1);
  config.setEntry("bottom", 5);
  config.setEntry("exclusive", true);
  config.print();

  ASSERT_TRUE(subfilter->configure(config));

  filter->appendFilter(subfilter);

  subfilter = factory.createFilter(
          ZFlyEmNeuronFilterFactory::COMPOSITE);
  subfilter->appendFilter(factory.createFilter(
                            ZFlyEmNeuronFilterFactory::LAYER));

  filter->appendFilter(subfilter);

  subfilter = factory.createFilter(ZFlyEmNeuronFilterFactory::TIP_DISTANCE);
  ASSERT_TRUE(subfilter);

  config.setEntry("min_dist", 10.0);
  ASSERT_TRUE(subfilter->configure(config));
  filter->appendFilter(subfilter);

  filter->print();  
}

TEST(ZFlyEmNeuronFilter, filter)
{
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(GET_TEST_DATA_DIR + "/benchmark/bundle1/data_bundle.json");
  ZFlyEmNeuron *neuron = bundle.getNeuron(1);

  ZFlyEmNeuronFilterFactory factory;

  //Test layer filter
  ZFlyEmNeuronFilter *filter = factory.createFilter(
        ZFlyEmNeuronFilterFactory::LAYER);
  ZJsonObject config;
  config.setEntry("top", 1);
  config.setEntry("bottom", 5);
  config.setEntry("exclusive", true);
  config.print();

  filter->configure(config);

  ASSERT_TRUE(filter->isPassed(*neuron));
  ASSERT_FALSE(filter->isPassed(*bundle.getNeuron(2)));

  config.setEntry("bottom", 4);
  filter->configure(config);
  ASSERT_FALSE(filter->isPassed(*neuron));

  ZFlyEmNeuronFilter *filter2 = factory.createFilter(
        ZFlyEmNeuronFilterFactory::TIP_DISTANCE);
  filter2->setReference(neuron);

  ASSERT_FALSE(filter2->isPassed(*bundle.getNeuron(2)));
  ASSERT_TRUE(filter2->isPassed(*bundle.getNeuron(4)));
  ASSERT_FALSE(filter2->isPassed(*bundle.getNeuron(3)));

  ZFlyEmNeuronFilter *filter3 = factory.createFilter(
        ZFlyEmNeuronFilterFactory::TIP_ANGLE);
  filter3->setReference(neuron);

  ASSERT_TRUE(filter3->isPassed(*bundle.getNeuron(2)));
  ASSERT_FALSE(filter3->isPassed(*bundle.getNeuron(4)));
  ASSERT_FALSE(filter3->isPassed(*bundle.getNeuron(3)));

  ZFlyEmNeuronFilter *filter4 = factory.createFilter(
        ZFlyEmNeuronFilterFactory::COMPOSITE);
  filter4->appendFilter(filter2);
  filter4->appendFilter(filter3);
  filter4->setReference(neuron);

  ASSERT_FALSE(filter4->isPassed(*bundle.getNeuron(2)));
  ASSERT_FALSE(filter4->isPassed(*bundle.getNeuron(4)));
  ASSERT_FALSE(filter4->isPassed(*bundle.getNeuron(3)));

  ZFlyEmNeuronFilter *filter5 = factory.createFilter(
        ZFlyEmNeuronFilterFactory::COMPOSITE);
  filter5->appendFilter(filter);
  filter5->appendFilter(filter2);
  filter5->setReference(neuron);

  config.setEntry("bottom", 5);
  filter->configure(config);
  filter5->print();
  ASSERT_TRUE(filter5->isPassed(*bundle.getNeuron(4)));

  ZFlyEmNeuronFilter *filter6 =
      factory.createFilter(ZFlyEmNeuronFilterFactory::DEEP_ANGLE);
  filter6->setReference(neuron);
  {
    ZJsonObject config;
    config.setEntry("deep_level", 3);
    config.setEntry("deep_dist", 10000.0);
    filter6->configure(config);
  }

  filter6->print();

  ASSERT_TRUE(filter6->isPassed(*bundle.getNeuron(2)));
  ASSERT_FALSE(filter6->isPassed(*bundle.getNeuron(4)));
  ASSERT_FALSE(filter6->isPassed(*bundle.getNeuron(3)));
}


#endif


#endif // ZFLYEMNEURONFILTERTEST_H
