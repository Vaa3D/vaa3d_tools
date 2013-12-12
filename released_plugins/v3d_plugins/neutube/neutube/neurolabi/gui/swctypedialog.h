#ifndef SWCTYPEDIALOG_H
#define SWCTYPEDIALOG_H

#include <QDialog>

namespace Ui {
class SwcTypeDialog;
}

#include "zswctree.h"

class SwcTypeDialog : public QDialog
{
  Q_OBJECT
  
public:
  enum EPickingMode{
    INDIVIDUAL, DOWNSTREAM, CONNECTION, MAIN_TRUNK, TRUNK_LEVEL, TRAFFIC,
    BRANCH_LEVEL, LONGEST_LEAF, FURTHEST_LEAF, ROOT
  };

public:
  explicit SwcTypeDialog(QWidget *parent = 0);
  explicit SwcTypeDialog(ZSwcTree::ESelectionMode selected,
                         QWidget *parent);
  ~SwcTypeDialog();
  
  int type();
  EPickingMode pickingMode();

private:
  Ui::SwcTypeDialog *ui;
};

#endif // SWCTYPEDIALOG_H
