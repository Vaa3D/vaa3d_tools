#ifndef SYNAPSERESAMPLERDIALOG_H
#define SYNAPSERESAMPLERDIALOG_H

#include <QDialog>
#include <QPoint>
#include <Matrix3D.h>
#include "CommonTypes.h"

namespace Ui {
    class SynapseResamplerDialog;
}

class SynapseResamplerDialog : public QDialog
{
    Q_OBJECT
private:
    const Matrix3D<PixelType> *mRawImg;
    Matrix3D<LabelType> *mLabelImg;

public:
    explicit SynapseResamplerDialog(QWidget *parent = 0);
    ~SynapseResamplerDialog();

    void mainWindowClicked(unsigned int px, unsigned int py, unsigned int pz);

    void setAuxData( const Matrix3D<PixelType> *rawImg, Matrix3D<LabelType> *labelImg );

private:
    Ui::SynapseResamplerDialog *ui;
};

#endif // SYNAPSERESAMPLERDIALOG_H
