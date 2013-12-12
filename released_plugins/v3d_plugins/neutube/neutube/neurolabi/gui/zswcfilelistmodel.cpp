#include "zswcfilelistmodel.h"

#include <QDir>

ZSwcFileListModel::ZSwcFileListModel(QObject *parent) :
  QAbstractListModel(parent)
{
}

void ZSwcFileListModel::loadDir(const QString &dirPath)
{
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.swc" << "*.SWC";
  m_fileList = dir.entryInfoList(filter, QDir::NoFilter, QDir::Time);
}

int ZSwcFileListModel::rowCount(const QModelIndex &/*parent*/) const
{
  return m_fileList.size();
}

QVariant ZSwcFileListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= m_fileList.size() || index.row() < 0) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return m_fileList.at(index.row()).completeBaseName();
  }

  return QVariant();
}

bool ZSwcFileListModel::removeRows(int row, int count,
                                   const QModelIndex &/*parent*/)
{
  bool removed = false;

  for (int i = 0; i < count; ++i) {
    if (row < m_fileList.size()) {
      m_fileList.removeAt(row);
      removed = true;
    }
  }

  return removed;
}

QString ZSwcFileListModel::getFilePath(int index)
{
  return m_fileList[index].absoluteFilePath();
}

void ZSwcFileListModel::deleteAll()
{
  for (int i = 0; i < rowCount(); ++i) {
    QFile::remove(getFilePath(i));
  }
  removeRows(0, rowCount());
}
