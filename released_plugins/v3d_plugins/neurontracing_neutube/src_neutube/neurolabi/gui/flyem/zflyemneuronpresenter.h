#ifndef ZFLYEMNEURONPRESENTER_H
#define ZFLYEMNEURONPRESENTER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include "zflyemneuron.h"

class ZFlyEmDataFrame;

class ZFlyEmNeuronPresenter : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmNeuronPresenter(QObject *parent = 0);

  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole ) const;
  virtual QVariant data(const ZFlyEmNeuron &neuron, int index,
                        int role = Qt::DisplayRole) const = 0;
  virtual int columnCount() const;
  virtual const ZFlyEmNeuron *getNeuron(
      const ZFlyEmNeuron &neuron, int index) const;
  virtual QVector<const ZFlyEmNeuron*> getNeuronArray(
      const ZFlyEmNeuron &neuron, int index) const;

protected:
  static QString getIdString(const ZFlyEmNeuron &neuron);

signals:

public slots:

protected:
  QVector<QString> m_fieldList;
};

class ZFlyEmNeuronInfoPresenter : public ZFlyEmNeuronPresenter
{
  Q_OBJECT
public:
  explicit ZFlyEmNeuronInfoPresenter(QObject *parent = 0);
  QVariant data(const ZFlyEmNeuron &neuron, int index,
                int role = Qt::DisplayRole) const;
};

class ZFlyEmNeuronFeaturePresenter : public ZFlyEmNeuronPresenter
{
  Q_OBJECT
public:
  explicit ZFlyEmNeuronFeaturePresenter(QObject *parent = 0);
  QVariant data(const ZFlyEmNeuron &neuron, int index,
                int role = Qt::DisplayRole) const;
};

class ZFlyEmNeuronConnectionPresenter : public ZFlyEmNeuronPresenter
{
  Q_OBJECT
public:
  explicit ZFlyEmNeuronConnectionPresenter(QObject *parent = 0);
  QVariant data(const ZFlyEmNeuron &neuron, int index,
                int role = Qt::DisplayRole) const;
  const ZFlyEmNeuron *getNeuron(
        const ZFlyEmNeuron &neuron, int index) const;
  QVector<const ZFlyEmNeuron*> getNeuronArray(
        const ZFlyEmNeuron &neuron, int index) const;
};

class ZFlyEmNeuronVolumePresenter : public ZFlyEmNeuronPresenter
{
  Q_OBJECT
public:
  explicit ZFlyEmNeuronVolumePresenter(QObject *parent = 0);
  QVariant data(const ZFlyEmNeuron &neuron, int index,
                int role = Qt::DisplayRole) const;
};

class ZFlyEmNeuronTopMatchPresenter : public ZFlyEmNeuronPresenter
{
  Q_OBJECT
public:
  explicit ZFlyEmNeuronTopMatchPresenter(QObject *parent = 0);
  QVariant data(const ZFlyEmNeuron &neuron, int index,
                int role = Qt::DisplayRole) const;

  const ZFlyEmNeuron *getNeuron(const ZFlyEmNeuron &neuron, int index) const;

  inline void setDataFrame(ZFlyEmDataFrame *frame) {
    m_frame = frame;
  }

private:
  ZFlyEmDataFrame *m_frame;
};


#endif // ZFLYEMNEURONPRESENTER_H
