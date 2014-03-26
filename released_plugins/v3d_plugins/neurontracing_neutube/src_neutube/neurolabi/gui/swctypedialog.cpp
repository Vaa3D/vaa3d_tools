#include "swctypedialog.h"

#include <iostream>

#include "ui_swctypedialog.h"

SwcTypeDialog::SwcTypeDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SwcTypeDialog)
{
  ui->setupUi(this);
}

SwcTypeDialog::SwcTypeDialog(ZSwcTree::ESelectionMode selected, QWidget *parent)
  : QDialog(parent), ui(new Ui::SwcTypeDialog)
{
  ui->setupUi(this);

  switch (selected) {
  case ZSwcTree::WHOLE_TREE:
    ui->mainTrunkRadioButton->show();
    ui->connectionRadioButton->hide();
    ui->downRadioButton->hide();
    ui->trafficRadioButton->show();
    ui->trunkLevelRadioButton->show();
    ui->rootRadioButton->show();
    ui->subtreeRadioButton->show();
    break;

  case ZSwcTree::SWC_NODE:
    ui->mainTrunkRadioButton->hide();
    ui->connectionRadioButton->show();
    ui->downRadioButton->show();
    ui->trafficRadioButton->hide();
    ui->trunkLevelRadioButton->hide();
    ui->rootRadioButton->hide();
    ui->subtreeRadioButton->hide();
    break;

  default:
    break;
  }
}

SwcTypeDialog::~SwcTypeDialog()
{
  delete ui;
}

int SwcTypeDialog::type()
{
  return ui->typeSpinBox->value();
}

SwcTypeDialog::EPickingMode SwcTypeDialog::pickingMode()
{
  SwcTypeDialog::EPickingMode mode = INDIVIDUAL;

  if (ui->individualRadioButton->isChecked()) {
    mode = INDIVIDUAL;
  }

  if (ui->connectionRadioButton->isChecked()) {
    mode = CONNECTION;
  }

  if (ui->downRadioButton->isChecked()) {
    mode = DOWNSTREAM;
  }

  if (ui->mainTrunkRadioButton->isChecked()) {
    mode = MAIN_TRUNK;
  }

  if (ui->trafficRadioButton->isChecked()) {
    mode = TRAFFIC;
  }

  if (ui->longestLeafRadioButton->isChecked()) {
    mode = LONGEST_LEAF;
  }

  if (ui->furthestRadioButton->isChecked()) {
    mode = FURTHEST_LEAF;
  }

  if (ui->trunkLevelRadioButton->isChecked()) {
    mode = TRUNK_LEVEL;
  }

  if (ui->rootRadioButton->isChecked()) {
    mode = ROOT;
  }

  if (ui->branchLevelRadioButton->isChecked()) {
    mode = BRANCH_LEVEL;
  }

  if (ui->subtreeRadioButton->isChecked()) {
    mode = SUBTREE;
  }

#ifdef _DEBUG_
  std::cout << "selection mode " << mode << std::endl;
#endif

  return mode;
}
