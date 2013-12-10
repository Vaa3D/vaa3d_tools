#ifndef ZRESCALESWCDIALOG_H
#define ZRESCALESWCDIALOG_H

#include <QDialog>

namespace Ui {
class ZRescaleSwcDialog;
}

struct ZRescaleSwcSetting
{
  bool bTranslateSoma;
  double translateDstX;
  double translateDstY;
  double translateDstZ;
  double scaleX;
  double scaleY;
  double scaleZ;
  bool bRescaleBranchRadius;
  bool bRescaleSomaRadius;
  double rescaleBranchRadiusScale;
  double rescaleSomaRadiusScale;
  int somaCutLevel;
  bool bReduceSwcNodes;
  double reduceSwcNodesLengthThre;
};

class ZRescaleSwcDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit ZRescaleSwcDialog(QWidget *parent = 0);
  ~ZRescaleSwcDialog();

  ZRescaleSwcSetting getRescaleSetting();

private slots:
  void translateSomaCheckBoxChanged(int state);
  void setScaleMethod();
  void rescaleBranchRadiusCheckBoxChanged(int state);
  void rescaleSomaRadiusCheckBoxChanged(int state);
  void reduceNSwcNodesCheckBoxChanged(int state);
  
private:
  Ui::ZRescaleSwcDialog *ui;
};

#endif // ZRESCALESWCDIALOG_H
