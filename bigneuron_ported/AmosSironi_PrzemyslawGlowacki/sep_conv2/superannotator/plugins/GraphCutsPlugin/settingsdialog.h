#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class settingsDialog;
}

class settingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit settingsDialog(QWidget *parent = 0, int _activeOverlay = 0, int _brushSizeX = 1, int _brushSizeY = 1, int _brushSizeZ = 1,
                            int maxWidth = 100, int maxHeight = 100, int maxDepth = 100);
    ~settingsDialog();
    
    int getActiveOverlay();
    int getBrushSizeX();
    int getBrushSizeY();
    int getBrushSizeZ();
    int getMaxWidth();
    int getMaxHeight();
    int getMaxDepth();
    int getGaussianVariance();
    int getEdgeWeight();

private:
    Ui::settingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
