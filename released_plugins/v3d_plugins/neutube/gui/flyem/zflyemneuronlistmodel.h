#ifndef ZFLYEMNEURONLISTMODEL_H
#define ZFLYEMNEURONLISTMODEL_H

#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QVector>
#include <QtCore>
#include <QString>

class ZFlyEmNeuron;
class ZStackDoc;
class ZFlyEmNeuronPresenter;

class ZFlyEmNeuronListModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit ZFlyEmNeuronListModel(QObject *parent = 0);

  int rowCount( const QModelIndex & parent = QModelIndex() ) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data( const QModelIndex & index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation,
                     int role = Qt::DisplayRole ) const;

  void clear();
  void append(const ZFlyEmNeuron* neuron);

  const ZFlyEmNeuron *getNeuron(int index) const;
  const ZFlyEmNeuron *getNeuron(int row, int col) const;
  const ZFlyEmNeuron *getNeuron(const QModelIndex &index) const;
  ZFlyEmNeuron *getNeuron(const QModelIndex &index);

  QVector<const ZFlyEmNeuron*> getNeuronArray(const QModelIndex &index) const;

  void retrieveModel(const QModelIndexList &indexList, ZStackDoc *doc) const;
  bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
  bool insertColumns(int col, int count, const QModelIndex &parent = QModelIndex());
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
  bool removeColumns(int col, int count, const QModelIndex &parent = QModelIndex());

  void exportCsv(const QString &path);
  void exportSwc(const QString &swc);

  void setPresenter(ZFlyEmNeuronPresenter *presenter);

  void notifyDataChanged (const QModelIndex & topLeft,
                          const QModelIndex & bottomRight);

  void notifyRowDataChanged(int row);

  /*!
   * \brief Test if the current index if the key of the neuron.
   *
   * \return true iff \a index is the key.
   */
  static bool isNeuronKey(const QModelIndex &index);

signals:

public slots:

private:
  QVector<const ZFlyEmNeuron*> m_neuronList;
  ZFlyEmNeuronPresenter *m_presenter;
  ZFlyEmNeuronPresenter *m_defaultPresenter;
};

#endif // ZFLYEMNEURONLISTMODEL_H
