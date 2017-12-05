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
