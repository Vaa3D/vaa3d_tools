#include <iostream>

#include <QString>
#include <QStringList>
#include <QTimer>

#include "ui_Neurite_Instructor.h"
#include "neuriteinstructorui.h"
#include "Neurite_Instructor_plugin.h"
#include "funcs.h"

using namespace std;

neuriteInstructorUI::neuriteInstructorUI(QWidget* parent, V3DPluginCallback2* callback) : QDialog(parent), ui(new Ui::neuriteInstructorUI)
{
    ui->setupUi(this);
    this->mainCallBack = callback;
    this->pluginParent = parent;

    this->deployDisplay = "";
    this->modelDisplay = "";
    this->meanDisplay = "";
}

neuriteInstructorUI::~neuriteInstructorUI()
{
    delete ui;
}

void neuriteInstructorUI::uiCall()
{
    this->exec();
}

void neuriteInstructorUI::okClicked()
{
    this->setWindowModality(Qt::ApplicationModal);

    v3dhandle curwin = this->mainCallBack->currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "No image window found.");
        return;
    }
    else
    {
        this->p4DImage = mainCallBack->getImage(curwin);
        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        this->ImgPtr = this->p4DImage->getRawData();
        this->imgX = this->p4DImage->getXDim();
        this->imgY = this->p4DImage->getYDim();
        this->imgZ = this->p4DImage->getZDim();
        this->channel = this->p4DImage->getCDim();
    }

    if (this->deployDisplay == "")
    {
        QMessageBox::information(0, "", "Deploying file is not chosen.");
        return;
    }
    if (this->modelDisplay == "")
    {
        QMessageBox::information(0, "", "Trained model file is not chosen.");
        return;
    }
    if (this->meanDisplay == "")
    {
        QMessageBox::information(0, "", "Image mean file is not chosen.");
        return;
    }

    this->markerList = mainCallBack->getLandmark(curwin);
    predictSWCstroke(this, mainCallBack, curwin);

    this->setWindowModality(Qt::NonModal);
}

void neuriteInstructorUI::filePath()
{
    QObject* emitter = sender();
    QString emitterName = emitter->objectName();

    //if(fileName.isEmpty()) return;

    if (emitterName == "pushButton")
    {
        QString fileName = QFileDialog::getOpenFileName(0, QObject::tr("open Network Model Files"), "",
                                                        QObject::tr("Deploying file (*.prototxt)"));

        ui->lineEdit->setText(fileName);
        this->deployDisplay = fileName;
        this->deployName = fileName.toStdString();
    }
    else if (emitterName == "pushButton_2")
    {
        QString fileName = QFileDialog::getOpenFileName(0, QObject::tr("open Network Model Files"), "",
                                                        QObject::tr("Trained model file (*.caffemodel)"));

        ui->lineEdit_2->setText(fileName);
        this->modelDisplay = fileName;
        this->modelName = fileName.toStdString();
    }
    else if (emitterName == "pushButton_3")
    {
        QString fileName = QFileDialog::getOpenFileName(0, QObject::tr("open Network Model Files"), "",
                                                        QObject::tr("Image mean file (*.binaryproto)"));
        ui->lineEdit_3->setText(fileName);
        this->meanDisplay = fileName;
        this->meanName = fileName.toStdString();
    }
}
