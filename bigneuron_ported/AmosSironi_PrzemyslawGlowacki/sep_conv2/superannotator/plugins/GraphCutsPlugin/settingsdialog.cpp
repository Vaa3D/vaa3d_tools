#include "settingsdialog.h"
#include "ui_settingsdialog.h"

settingsDialog::settingsDialog(QWidget *parent, int _activeOverlay,
                               int _brushSizeX, int _brushSizeY, int _brushSizeZ,
                               int _maxWidth, int _maxHeight, int _maxDepth) :
    QDialog(parent),
    ui(new Ui::settingsDialog)
{
    ui->setupUi(this);
    ui->sbActiveOverlay->setValue(_activeOverlay+1);
    ui->sbBrushSizeX->setValue(_brushSizeX);
    ui->sbBrushSizeY->setValue(_brushSizeY);
    ui->sbBrushSizeZ->setValue(_brushSizeZ);
    ui->sbMaxWidth->setValue(_maxWidth);
    ui->sbMaxHeight->setValue(_maxHeight);
    ui->sbMaxDepth->setValue(_maxDepth);
}

settingsDialog::~settingsDialog()
{
    delete ui;
}

int settingsDialog::getActiveOverlay()
{
    return ui->sbActiveOverlay->value()-1;
}

int settingsDialog::getBrushSizeX()
{
    return ui->sbBrushSizeX->value();
}

int settingsDialog::getBrushSizeY()
{
    return ui->sbBrushSizeY->value();
}

int settingsDialog::getBrushSizeZ()
{
    return ui->sbBrushSizeZ->value();
}

int settingsDialog::getMaxWidth()
{
    return ui->sbMaxWidth->value();
}

int settingsDialog::getMaxHeight()
{
    return ui->sbMaxHeight->value();
}

int settingsDialog::getMaxDepth()
{
    return ui->sbMaxDepth->value();
}

int settingsDialog::getGaussianVariance()
{
    return ui->sbVariance->value();
}

int settingsDialog::getEdgeWeight()
{
    return ui->sbEdgeWeight->value();
}
