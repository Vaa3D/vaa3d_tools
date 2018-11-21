#ifndef BIGSCREENCONTROLDIALOG_H
#define BIGSCREENCONTROLDIALOG_H
#include<QtGui>;
#include <string>;
#include "v3d_message.h"
#include <iostream>;
using namespace std;
struct DisplayPARA
{
    QString openfolder;
    QString openanchorfile;
    double spositionX;
    double spositionY;
    int displaynumber;
    int updateinterval;
    bool isneuronCombined;
    bool isDownSample;
    bool isDownSampleCombined;

    int winsizeX;
    int winsizeY;
    int displaycolumn;
    int displayrow;

    int sampleRate;
    int downsamplerate;
};

class BigscreenControlDialog : public QDialog
{
    Q_OBJECT
public:
    BigscreenControlDialog(QWidget * parent) : QDialog(parent)
    {
        displaypath_label=new QLabel(tr("Choose the display directory : "));
        editor_displayfolder = new QLineEdit(tr(""));
        button_displaypath = new QPushButton(tr("..."));

        anchorpath_label=new QLabel(tr("Choose the anchor file : "));
        editor_anchorfile = new QLineEdit(tr(""));
        button_anchorfile = new QPushButton(tr("..."));

        startPointX=new QLineEdit(QString("").setNum(0));
        startPointY=new QLineEdit(QString("").setNum(0));

        winSizeX=new QSpinBox();
        winSizeX->setRange(700,10000);
        winSizeX->setValue(1920);

        winSizeY=new QSpinBox();
        winSizeY->setRange(700,10000);
        winSizeY->setValue(1080);

        displaycolumn=new QSpinBox();
        displaycolumn->setRange(1,5);
        displaycolumn->setValue(3);

        displayrow=new QSpinBox();
        displayrow->setRange(1,3);
        displayrow->setValue(3);

        displayNum=new QSpinBox();
        displayNum->setRange(1,15);
        displayNum->setValue(9);

        updateInterval=new QSpinBox();
        updateInterval->setRange(0,100);
        updateInterval->setValue(30);

        downSample=new QCheckBox();
        downSample->setText(tr("Down Sample"));
        downSample->setChecked(false);

        combineNeuron=new QCheckBox();
        combineNeuron->setText(tr("Combine"));
        combineNeuron->setChecked(true);

        downSampleRate=new QSpinBox();
        downSampleRate->setRange(1,1000);
        downSampleRate->setValue(20);

        zmovieSamplingrate=new QSpinBox();
        zmovieSamplingrate->setRange(1,1000);
        zmovieSamplingrate->setValue(100);

        downSampleCombined=new QCheckBox();
        downSampleCombined->setText(tr("Down Sample (combined)"));
        downSampleCombined->setChecked("true");

        ok = new QPushButton(tr("ok"));
        cancel = new QPushButton(tr("cancel"));

        gridlayout=new QGridLayout();
        gridlayout->addWidget(displaypath_label,0,0,1,5);
        gridlayout->addWidget(editor_displayfolder, 0,5,1,4);
        gridlayout->addWidget(button_displaypath,0,9,1,1);

        gridlayout->addWidget(new QLabel("start point X : "),1,0,1,8);
        gridlayout->addWidget(startPointX,1,8,1,1);
        gridlayout->addWidget(new QLabel("start point Y : "),2,0,1,8);
        gridlayout->addWidget(startPointY,2,8,1,1);


        gridlayout->addWidget(new QLabel("display number : "),3,0,1,8);
        gridlayout->addWidget(displayNum,3,8,1,1);

        gridlayout->addWidget(combineNeuron,4,0,1,4);
        //gridlayout->addWidget(downSample,4,4,1,4);
        gridlayout->addWidget(downSampleCombined,4,8,1,5);

        gridlayout->addWidget(new QLabel("Window Size X : "),5,0,1,8);
        gridlayout->addWidget(winSizeX,5,8,1,2);
        gridlayout->addWidget(new QLabel("Window Size Y: "),6,0,1,8);
        gridlayout->addWidget(winSizeY,6,8,1,2);
        gridlayout->addWidget(new QLabel("Display Column : "),7,0,1,8);
        gridlayout->addWidget(displaycolumn,7,8,1,2);
        gridlayout->addWidget(new QLabel("Display Row : "),8,0,1,8);
        gridlayout->addWidget(displayrow,8,8,1,2);

        gridlayout->addWidget(new QLabel("update interval : "),9,0,1,8);
        gridlayout->addWidget(updateInterval,9,8,1,1);

        gridlayout->addWidget(new QLabel("down sample rate : "),10,0,1,8);
        gridlayout->addWidget(downSampleRate,10,8,1,4);

        gridlayout->addWidget(anchorpath_label,11,0,1,5);
        gridlayout->addWidget(editor_anchorfile, 11,5,1,4);
        gridlayout->addWidget(button_anchorfile,11,9,1,1);

        gridlayout->addWidget(new QLabel("zmoive sampling rate : "),12,0,1,8);
        gridlayout->addWidget(zmovieSamplingrate,12,8,1,4);

        gridlayout->addWidget(cancel, 13, 0, 1, 5, Qt::AlignRight);
        gridlayout->addWidget(ok, 13, 5, 1, 5, Qt::AlignRight);

        connect(button_displaypath,SIGNAL(clicked()),this,SLOT(setFolderPath()));
        connect(button_anchorfile,SIGNAL(clicked()),this,SLOT(setAnchorFile()));
        connect(ok,SIGNAL(clicked()),this,SLOT(accept()));
        connect(cancel,SIGNAL(clicked()),this,SLOT(reject()));

//        connect(ok, SIGNAL(clicked()), this, SLOT(update()));

        this->setLayout(gridlayout);
        this->setWindowTitle("Display Control Center");
    }
    ~BigscreenControlDialog(){}
public slots:

    void accept()
    {
        return QDialog::accept();
    }

    void reject()
    {
        return QDialog::reject();
    }
    void setFolderPath()
    {
        QString title="open display folder : ";
        QString current_path = editor_displayfolder->text();
        QString dir = QFileDialog::getExistingDirectory(this, title,
                                                         "~/",
                                                         QFileDialog::ShowDirsOnly
                                                         | QFileDialog::DontResolveSymlinks);
        if(dir.isEmpty())
            editor_displayfolder->setText(current_path);
        else
            editor_displayfolder->setText(dir);
    }
    void setAnchorFile()
    {
        QString title="choose anchor file for displaying : ";
        QString current_file= editor_anchorfile->text();
        QString anchorfile=QFileDialog::getOpenFileName(this, title,
                                                        "",
                                                        QObject::tr("Anchor Point File (*.apftxt *.txt *.apf)"
                                                                    ));
        if(anchorfile.isEmpty())
        {
            cout<<"Anchor file is not existed or empty!";
            editor_anchorfile->setText(current_file);
        }
        else
        {
            editor_anchorfile->setText(anchorfile);
        }

    }

    void update()
    {
        if(editor_displayfolder->text().isEmpty())
        {
            v3d_msg("Please select the correct path!");
            return QDialog::reject();
        }
        controlPara.openfolder=editor_displayfolder->text()/*.toStdString()*/;
        controlPara.openanchorfile=editor_anchorfile->text();
        controlPara.spositionX=atof(startPointX->text().toStdString().c_str());
        controlPara.spositionY=atof(startPointY->text().toStdString().c_str());

        controlPara.displaynumber=displayNum->text().toInt();
        controlPara.updateinterval=updateInterval->text().toInt();
        controlPara.winsizeX=winSizeX->text().toInt();
        controlPara.winsizeY=winSizeY->text().toInt();
        controlPara.displaycolumn=displaycolumn->text().toInt();
        controlPara.displayrow=displayrow->text().toInt();
        controlPara.isneuronCombined=combineNeuron->isChecked();
        controlPara.isDownSample=downSample->isChecked();
        controlPara.isDownSampleCombined=downSampleCombined->isChecked();
        controlPara.downsamplerate=downSampleRate->text().toInt();
        controlPara.sampleRate=zmovieSamplingrate->text().toInt();
    }

public:
    DisplayPARA controlPara;

    QGridLayout *gridlayout;

    QLabel * displaypath_label;
    QLineEdit * editor_displayfolder;
    QPushButton * button_displaypath;

    QLabel * anchorpath_label;
    QLineEdit * editor_anchorfile;
    QPushButton * button_anchorfile;

    QLineEdit * startPointX;
    QLineEdit * startPointY;

    QSpinBox * winSizeX;
    QSpinBox * winSizeY;

    QSpinBox * displayNum;
    QSpinBox * updateInterval;

    QSpinBox * displaycolumn;
    QSpinBox * displayrow;

    QSpinBox * downSampleRate;
    QSpinBox * zmovieSamplingrate;

    QCheckBox * downSample;
    QCheckBox * combineNeuron;
    QCheckBox * downSampleCombined;

    QPushButton * ok;
    QPushButton * cancel;

};
#endif // BIGSCREENCONTROLDIALOG_H
