#ifndef ZFLYEMNEURONFILTER_H
#define ZFLYEMNEURONFILTER_H

#include <vector>
#include "zflyemneuron.h"
#include "zjsonobject.h"
#include "swc/zswcdeepanglemetric.h"

/*!
 * \brief The class of selecting a subset of flyem neurons.
 *
 * It is a composite patten class.
 */
class ZFlyEmNeuronFilter
{
public:
  ZFlyEmNeuronFilter();
  virtual ~ZFlyEmNeuronFilter();

  std::vector<ZFlyEmNeuron*> filter(
      const std::vector<ZFlyEmNeuron*> input) const;

  virtual bool isPassed(const ZFlyEmNeuron &neuron) const = 0;

  /*!
   * \brief Configure the filter from a json object.
   *
   * \return true if the configuration is successful.
   */
  virtual bool configure(ZJsonValue &config) = 0;

  /*!
   * \brief Print the filter.
   */
  virtual void print(int indent = 0) const = 0;

  virtual void appendFilter(ZFlyEmNeuronFilter *filter);

  /*!
   * \brief Set the reference neuron.
   * \param neuron The reference neuron is used to calculate the relationship
   */
  virtual void setReference(const ZFlyEmNeuron *neuron);

protected:
  const ZFlyEmNeuron *m_refNeuron;
};

class ZFlyEmNeuronCompositeFilter : public ZFlyEmNeuronFilter
{
public:
  ZFlyEmNeuronCompositeFilter();

  std::vector<ZFlyEmNeuron*> filter(
      const std::vector<ZFlyEmNeuron*> input) const;

  void appendFilter(ZFlyEmNeuronFilter *filter);

  bool isPassed(const ZFlyEmNeuron &neuron) const;

  /*!
   * \brief Configure the filter from a json object.
   */
  bool configure(ZJsonValue &config);

  /*!
   * \brief Print the filter.
   */
  void print(int indent = 0) const;

  /*!
   * \brief Set reference neuron
   * The reference \a neuron is passed to all components.
   */
  void setReference(const ZFlyEmNeuron *neuron);

private:
  std::vector<ZFlyEmNeuronFilter*> m_filterArray;
};

class ZFlyEmNeuronLayerFilter : public ZFlyEmNeuronFilter
{
public:
  ZFlyEmNeuronLayerFilter();
  bool isPassed(const ZFlyEmNeuron &neuron) const;

  double getLayerStart(int layer) const;
  double getLayerEnd(int layer) const;

  bool configure(ZJsonValue &config);

  void print(int indent = 0) const;

private:
  const static int m_layerNumber;
  static double m_layerRatio[];
  double m_layerStart;
  double m_layerLength;
  int m_top;
  int m_bottom;
  bool m_isExclusive;
};

class ZFlyEmNeuronTipDistanceFilter : public ZFlyEmNeuronFilter
{
public:
  ZFlyEmNeuronTipDistanceFilter();

public:
  bool isPassed(const ZFlyEmNeuron &neuron) const;
  bool configure(ZJsonValue &config);
  void print(int indent = 0) const;

private:
  double m_minDist;
};

class ZFlyEmNeuronTipAngleFilter : public ZFlyEmNeuronFilter
{
public:
  ZFlyEmNeuronTipAngleFilter();

public:
  bool isPassed(const ZFlyEmNeuron &neuron) const;
  bool configure(ZJsonValue &config);
  void print(int indent = 0) const;

private:
  double m_minDist;
};

class ZFlyEmNeuronDeepAngleFilter : public ZFlyEmNeuronFilter
{
public:
  ZFlyEmNeuronDeepAngleFilter();

public:
  bool isPassed(const ZFlyEmNeuron &neuron) const;
  bool configure(ZJsonValue &config);
  void print(int indent = 0) const;

private:
  double m_minDist;
  mutable ZSwcDeepAngleMetric m_metric;
};

#endif // ZFLYEMNEURONFILTER_H
