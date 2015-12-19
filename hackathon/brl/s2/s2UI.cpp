#include "v3d_message.h"
#include <QDebug>
#include <QtGui>
#include <QtNetwork>
#include <stdlib.h>
#include "s2UI.h"
#include "s2Controller.h"



S2UI::S2UI(V3DPluginCallback2 &callback, QWidget *parent):   QDialog(parent)
{
    s2Label = new QLabel(tr("smartScope 2"));
    s2LineEdit = new QLineEdit("01a");
    startS2PushButton = new QPushButton(tr("Start smartScope2"));
    startScanPushButton = new QPushButton(tr("start scan"));

    buttonBox1 = new QDialogButtonBox;
    buttonBox1->addButton(startS2PushButton, QDialogButtonBox::ActionRole);
    buttonBox1->addButton(startScanPushButton, QDialogButtonBox::RejectRole);

    connect(startS2PushButton, SIGNAL(clicked()), this, SLOT(startS2()));
    connect(startScanPushButton, SIGNAL(clicked()), this, SLOT(startScan()));

    v3dhandle curwin = callback.currentImageWindow();
    /*if (!curwin)
    {
        v3d_msg("Please open an image.");
        return;
    }*/


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(s2Label, 0, 0);
    mainLayout->addWidget(s2LineEdit, 0, 1);
    mainLayout->addWidget(buttonBox1,1,0,2,4);
    setLayout(mainLayout);
    setWindowTitle(tr("smartScope2 Interface"));



}


void S2UI::startS2()
{
     myController.show();
     myController.exec();
}

void S2UI::startScan()
{
 /*   if (!myController)
    {
        v3d_msg("please start smartScope2");
    return;
    }*/
    myController.cleanAndSend("-mto test");
}
