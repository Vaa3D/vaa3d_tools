#ifndef ZSWCFILELISTMODEL_H
#define ZSWCFILELISTMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QFileInfoList>

class ZSwcFileListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  explicit ZSwcFileListModel(QObject *parent = 0);

  void loadDir(const QString &dirPath);

  int rowCount( const QModelIndex & parent = QModelIndex() ) const;
  QVariant data( const QModelIndex & index, int role = Qt::DisplayRole) const;

  void deleteAll();

  bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

  QString getFilePath(int index);

signals:

public slots:

private:
  QFileInfoList m_fileList;
};

#endif // ZSWCFILELISTMODEL_H
