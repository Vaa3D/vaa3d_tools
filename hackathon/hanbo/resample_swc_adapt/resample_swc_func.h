/* resample_swc_func.h
 * This is a plugin to resample neuron swc subject to a fixed step length.
 * 2012-03-02 : by Yinan Wan
 * 2014-10-02 : Hanbo Chen added adaptive downsample
 */
 
#ifndef __RESAMPLE_SWC_FUNC_H__
#define __RESAMPLE_SWC_FUNC_H__

#include <v3d_interface.h>

int resample_swc(V3DPluginCallback2 &callback, QWidget *parent);
bool resample_swc(const V3DPluginArgList & input, V3DPluginArgList & output);
bool resample_swc_toolbox(const V3DPluginArgList & input);

int resample_swc_adaptive(V3DPluginCallback2 &callback, QWidget *parent);

//define a simple dialog for choose adaptive sample parameter
class AdaptiveSampleDialog : public QDialog
{
    Q_OBJECT

public:
    AdaptiveSampleDialog(V3DPluginCallback2 &cb, QWidget *parent)
    {
        //initialization of variables
        gridLayout = 0;

        //create a dialog
        gridLayout = new QGridLayout();

        step_spinBox = new QDoubleSpinBox();
        step_spinBox->setMinimum(0);
        step_spinBox->setDecimals(2);
        step_spinBox->setValue(5);
        step_checkBox = new QCheckBox();
        angleT_spinBox = new QDoubleSpinBox();
        angleT_spinBox->setRange(0,180);
        angleT_spinBox->setDecimals(2);
        angleT_spinBox->setValue(10);
        radiusT_spinBox = new QDoubleSpinBox();
        radiusT_spinBox->setMinimum(1);
        radiusT_spinBox->setDecimals(2);
        radiusT_spinBox->setValue(2);
        radiusT_checkBox = new QCheckBox();

        gridLayout->addWidget(new QLabel("Peform resample before downsample:"),0,0,Qt::AlignRight);
        QHBoxLayout * hbox1 = new QHBoxLayout();
        hbox1->addWidget(step_checkBox);
        hbox1->addWidget(new QLabel("Step length:"));
        hbox1->addWidget(step_spinBox);
        gridLayout->addLayout(hbox1,0,1);
        gridLayout->addWidget(new QLabel("Maximum turning angle allowed:"),1,0,Qt::AlignRight);
        gridLayout->addWidget(angleT_spinBox, 1,1);
        gridLayout->addWidget(new QLabel("Skip when radius changes sharply:"),2,0,Qt::AlignRight);
        QHBoxLayout * hbox2 = new QHBoxLayout();
        hbox2->addWidget(radiusT_checkBox);
        hbox2->addWidget(new QLabel("Change rate:"));
        hbox2->addWidget(radiusT_spinBox);
        gridLayout->addLayout(hbox2,2,1);

        QHBoxLayout * hbox3 = new QHBoxLayout();
        ok     = new QPushButton("OK");
        cancel = new QPushButton("Cancel");
        ok->setMaximumWidth(50);
        cancel->setMaximumWidth(65);
        hbox3->addWidget(cancel);
        hbox3->addWidget(ok);
        gridLayout->addLayout(hbox3,3,1);

        setLayout(gridLayout);
        setWindowTitle(QString("SWC Adaptive Sampling"));

        //slot interface
        connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
        connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
        update();
    }

    ~AdaptiveSampleDialog(){}

    public slots:
    void update()
    {
        angleT = angleT_spinBox->text().toDouble();
        stepCheck=step_checkBox->isChecked();
        step=step_spinBox->text().toDouble();
        if(radiusT_checkBox->isChecked())
            radiusT = 1/radiusT_spinBox->text().toDouble();
        else
            radiusT = 0;

    }

public:
    double angleT,radiusT,step;
    bool stepCheck;
    QGridLayout *gridLayout;
    QPushButton* ok;
    QPushButton* cancel;
    QDoubleSpinBox * angleT_spinBox;
    QDoubleSpinBox * radiusT_spinBox;
    QCheckBox * radiusT_checkBox;
    QDoubleSpinBox * step_spinBox;
    QCheckBox * step_checkBox;
};
#endif

