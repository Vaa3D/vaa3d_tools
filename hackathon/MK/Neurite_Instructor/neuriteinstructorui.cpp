#include <iostream>

#include <QString>
#include <QStringList>

#include "ui_Neurite_Instructor.h"
#include "neuriteinstructorui.h"
#include "Neurite_Instructor_plugin.h"

using namespace std;

neuriteInstructorUI::neuriteInstructorUI(QWidget* parent, V3DPluginCallback2* callback) : QDialog(parent), ui(new Ui::neuriteInstructorUI)
{
    ui->setupUi(this);
    this->mainCallBack = callback;
    this->pluginParent = parent;

    this->show();
}

neuriteInstructorUI::~neuriteInstructorUI()
{
    delete ui;
}

void neuriteInstructorUI::okClicked()
{
    if (ui->checkBox->isChecked())
    {
        v3dhandle curwin = this->mainCallBack->currentImageWindow();
        if (!curwin)
        {
            cout << "no window opened" << endl;
            return;
        }

        this->markerList = mainCallBack->getLandmark(curwin);
        this->imageName = mainCallBack->getImageName(curwin);
    }
}

void neuriteInstructorUI::filePath()
{
    QObject* emitter = sender();
    QString emitterName = emitter->objectName();

    QString fileName = QFileDialog::getOpenFileName(0, QObject::tr("open Network Model Files"), "");

    if(fileName.isEmpty()) return;

    if (emitterName == "pushButton")
    {
        ui->lineEdit->setText(fileName);
        this->deployName = fileName;
    }
    else if (emitterName == "pushButton_2")
    {
        ui->lineEdit_2->setText(fileName);
        this->modelName = fileName;
    }
    else if (emitterName == "pushButton_3")
    {
        ui->lineEdit_3->setText(fileName);
        this->meanName = fileName;
    }

    //delete fileDlg;
}
