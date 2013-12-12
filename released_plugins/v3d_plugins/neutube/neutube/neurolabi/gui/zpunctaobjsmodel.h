#ifndef ZPUNCTAOBJSMODEL_H
#define ZPUNCTAOBJSMODEL_H

#include <map>
#include <vector>
#include "zobjsmodel.h"

class ZStackDoc;
class ZPunctum;

class ZPunctaObjsModel : public ZObjsModel
{
  Q_OBJECT
public:
  explicit ZPunctaObjsModel(ZStackDoc *doc, QObject *parent = 0);
  virtual ~ZPunctaObjsModel();

  QModelIndex getIndex(ZPunctum* punctum, int col = 0) const;
  // if index is single punctum, return it, otherwise return NULL
  ZPunctum* getPunctum(const QModelIndex &index) const;
  // if index contains many puncta, return them, otherwise return NULL
  const std::vector<ZPunctum *> *getPuncta(const QModelIndex &index) const;
  void updateData(ZPunctum* punctum);

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;

  // used to separate puncta by file (source)
  std::map<QString, ZObjsItem*> m_punctaSourceToParent;
  std::map<ZObjsItem*, int> m_punctaSourceParentToRow;
  std::map<QString, int> m_punctaSourceToCount;
  std::map<ZPunctum*, int> m_punctaToRow;
  std::vector<std::vector<ZPunctum*> > m_punctaSeparatedByFile;
  
};

#endif // ZPUNCTAOBJSMODEL_H
