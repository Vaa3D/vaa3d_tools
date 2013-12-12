#include "zflyemneuronpresenter.h"
#include "zswctree.h"
#include "zflyemdataframe.h"

ZFlyEmNeuronPresenter::ZFlyEmNeuronPresenter(QObject *parent) :
  QObject(parent)
{
}

QVariant ZFlyEmNeuronPresenter::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      return m_fieldList[section];
    }
  }

  return QVariant();
}

int ZFlyEmNeuronPresenter::columnCount() const
{
  return m_fieldList.size();
}

const ZFlyEmNeuron* ZFlyEmNeuronPresenter::getNeuron(
    const ZFlyEmNeuron &neuron, int index) const
{
  switch (index) {
  case 0:
    return &neuron;
  default:
    return NULL;
  }

  return NULL;
}

QVector<const ZFlyEmNeuron*> ZFlyEmNeuronPresenter::getNeuronArray(
      const ZFlyEmNeuron &neuron, int index) const
{
  QVector<const ZFlyEmNeuron*> neuronArray;
  const ZFlyEmNeuron *buddyNeuron = getNeuron(neuron, index);
  if (buddyNeuron != NULL) {
    neuronArray.push_back(buddyNeuron);
  }

  return neuronArray;
}

QString ZFlyEmNeuronPresenter::getIdString(const ZFlyEmNeuron &neuron)
{
  return QString("%1 (#%2)").arg(neuron.getId()).arg(neuron.getSourceId() + 1);
}

ZFlyEmNeuronInfoPresenter::ZFlyEmNeuronInfoPresenter(QObject *parent) :
  ZFlyEmNeuronPresenter(parent)
{
  m_fieldList << "ID" << "Name" << "Class" << "Model Path" << "Volume Path";
}

QVariant ZFlyEmNeuronInfoPresenter::data(
    const ZFlyEmNeuron &neuron, int index, int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    switch (index) {
    case 0:
      return getIdString(neuron);
    case 1:
      return neuron.getName().c_str();
    case 2:
      return neuron.getClass().c_str();
    case 3:
      return neuron.getModelPath().c_str();
    case 4:
      return neuron.getVolumePath().c_str();
    }
    break;
  case Qt::ToolTipRole:
    switch (index) {
    case 0:
      return QString("Double click to view model");
    case 3:
      return QString("%1").arg(getIdString(neuron)) +
          " (" + neuron.getModelPath().c_str() + ")";
    case 4:
      return QString("%1").arg(getIdString(neuron)) +
          " (" + neuron.getVolumePath().c_str() + ")";
    default:
      break;
    }
  case Qt::StatusTipRole:
    switch(index) {
    case 0:
      return neuron.toString().c_str();
    case 3:
      return QString("%1").arg(getIdString(neuron)) +
          " (" + neuron.getModelPath().c_str() + ")";
    case 4:
      return QString("%1").arg(getIdString(neuron)) +
          " (" + neuron.getVolumePath().c_str() + ")";
    default:
      break;
    }
    break;
  default:
    break;
  }

  return QVariant();
}

////////ZFlyEmNeuronFeaturePresenter////////////

ZFlyEmNeuronFeaturePresenter::ZFlyEmNeuronFeaturePresenter(QObject *parent) :
  ZFlyEmNeuronPresenter(parent)
{
  m_fieldList << "ID" << "Class" << "Length" << "#Branch" << "#TBar" << "#PSD"
              << "X" << "Y";
}

QVariant ZFlyEmNeuronFeaturePresenter::data(
    const ZFlyEmNeuron &neuron, int index, int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    switch (index) {
    case 0:
      return getIdString(neuron);
    case 1:
      return neuron.getClass().c_str();
    case 2:
      return neuron.getModel()->length();
    case 3:
      return Swc_Tree_Branch_Number(neuron.getModel()->data());
    case 4:
      return neuron.getTBarNumber();
    case 5:
      return neuron.getPsdNumber();
    case 6:
    {
      double x, y, z;
      Swc_Tree_Centroid(neuron.getModel()->data(), &x, &y, &z);
      return x;
    }
      break;
    case 7:
    {
      double x, y, z;
      Swc_Tree_Centroid(neuron.getModel()->data(), &x, &y, &z);
      return y;
    }
      break;
    default:
      break;
    }
    break;
  case Qt::ToolTipRole:
    switch (index) {
    case 0:
      return QString("Double click to view model");
    default:
      break;
    }
    break;
  case Qt::StatusTipRole:
  {
    const ZFlyEmNeuron *buddyNeuron = getNeuron(neuron, index);
    if (buddyNeuron != NULL) {
      return buddyNeuron->toString().c_str();
    }
  }
    break;
  default:
    break;
  }

  return QVariant();
}

////////ZFlyEmNeuronConnectionPresenter////////////

ZFlyEmNeuronConnectionPresenter::ZFlyEmNeuronConnectionPresenter(QObject *parent) :
  ZFlyEmNeuronPresenter(parent)
{
  m_fieldList << "ID" << "#Input" << "#Output" << "Strongest input"
             << "Strongest output";
}

QVariant ZFlyEmNeuronConnectionPresenter::data(
    const ZFlyEmNeuron &neuron, int index, int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    switch (index) {
    case 0:
      return getIdString(neuron);
    case 1:
      return neuron.getInputNeuronNumber();
    case 2:
      return neuron.getOutputNeuronNumber();
    case 3:
    {
      const ZFlyEmNeuron *input = neuron.getStrongestInput();
      if (input != NULL) {
        return input->getId();
      } else {
        return QVariant();
      }
    }
    case 4:
    {
      const ZFlyEmNeuron *output = neuron.getStrongestOutput();
      if (output != NULL) {
        return output->getId();
      } else {
        return QVariant();
      }
    }
    default:
      break;
    }
    break;
  case Qt::ToolTipRole:
    switch (index) {
    case 0:
      return QString("Double click to view model");
    default:
      break;
    }
    break;
  case Qt::StatusTipRole:
  {
    const ZFlyEmNeuron *buddyNeuron = getNeuron(neuron, index);
    if (buddyNeuron != NULL) {
      return buddyNeuron->toString().c_str();
    }
  }
    break;
  default:
    break;
  }

  return QVariant();
}

const ZFlyEmNeuron *ZFlyEmNeuronConnectionPresenter::getNeuron(
      const ZFlyEmNeuron &neuron, int index) const
{
  switch (index) {
  case 0:
    return &neuron;
  case 3:
    return neuron.getStrongestInput();
  case 4:
    return neuron.getStrongestOutput();
  default:
    return NULL;
  }

  return NULL;
}

QVector<const ZFlyEmNeuron*> ZFlyEmNeuronConnectionPresenter::getNeuronArray(
      const ZFlyEmNeuron &neuron, int index) const
{
  QVector<const ZFlyEmNeuron*> neuronArray;
  switch (index) {
  case 0:
    neuronArray.append(&neuron);
    break;
  case 1:
    for (int i = 0; i < neuron.getInputNeuronNumber(); ++i) {
      neuronArray.append(neuron.getInputNeuron(i));
    }
    break;
  case 2:
    for (int i = 0; i < neuron.getOutputNeuronNumber(); ++i) {
      neuronArray.append(neuron.getOutputNeuron(i));
    }
    break;
  case 3:
    neuronArray.append(neuron.getStrongestInput());
    break;
  case 4:
    neuronArray.append(neuron.getStrongestOutput());
    break;
  default:
    break;
  }

  return neuronArray;
}

////////ZFlyEmNeuronVolumePresenter////////////

ZFlyEmNeuronVolumePresenter::ZFlyEmNeuronVolumePresenter(QObject *parent) :
  ZFlyEmNeuronPresenter(parent)
{
  m_fieldList << "ID" << "Volume";
}

QVariant ZFlyEmNeuronVolumePresenter::data(
    const ZFlyEmNeuron &neuron, int index, int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    switch (index) {
    case 0:
      return getIdString(neuron);
    case 1:
      return neuron.getBodyVolume();
    default:
      break;
    }
    break;
  case Qt::ToolTipRole:
    switch (index) {
    case 0:
      return QString("Double click to view model");
    default:
      break;
    }
    break;
  case Qt::StatusTipRole:
  {
    const ZFlyEmNeuron *buddyNeuron = getNeuron(neuron, index);
    if (buddyNeuron != NULL) {
      return buddyNeuron->toString().c_str();
    }
  }
    break;
  default:
    break;
  }

  return QVariant();
}

////////ZFlyEmNeuronTopMatchPresenter////////////

ZFlyEmNeuronTopMatchPresenter::ZFlyEmNeuronTopMatchPresenter(QObject *parent) :
  ZFlyEmNeuronPresenter(parent)
{
  m_fieldList << "ID";

  for (int i = 1; i <= ZFlyEmNeuron::TopMatchCapacity; ++i) {
    m_fieldList << QString("Match #%1").arg(i);
  }
}

QVariant ZFlyEmNeuronTopMatchPresenter::data(
    const ZFlyEmNeuron &neuron, int index, int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    if (index == 0) {
      if (neuron.getClass().empty()) {
        return getIdString(neuron);
      } else {
        return QString("%1 (%2)").arg(getIdString(neuron)).arg(neuron.getClass().c_str());
      }
    } else {
      const std::vector<const ZFlyEmNeuron*> &topMatch = neuron.getTopMatch();
      if (index <= (int) topMatch.size()) {
        return QString("%1 (%2)").arg(getIdString(*(topMatch[index - 1]))).
            arg(topMatch[index - 1]->getClass().c_str());
      } else {
        return "N/A";
      }
    }
    break;
  case Qt::ToolTipRole:
    switch (index) {
    case 0:
      return QString("Double click to assign class");
    default:
      break;
    }
    break;
  case Qt::StatusTipRole:
  {
    const ZFlyEmNeuron *buddyNeuron = getNeuron(neuron, index);
    if (buddyNeuron != NULL) {
      return buddyNeuron->toString().c_str();
    }
  }
    break;
  default:
    break;
  }

  return QVariant();
}

const ZFlyEmNeuron* ZFlyEmNeuronTopMatchPresenter::getNeuron(
    const ZFlyEmNeuron &neuron, int index) const
{
  if (index < 0) {
    return NULL;
  }

  if (index == 0) {
    return &neuron;
  } else {
    const std::vector<const ZFlyEmNeuron*> &topMatch = neuron.getTopMatch();
    if (index <= (int) topMatch.size()) {
      return topMatch[index - 1];
    }
  }

  return NULL;
}
