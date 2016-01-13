#include "v3d_message.h"
#include <QWidget>
#include <QDialogButtonBox>
#include <QtGui>
#include <QtNetwork>
#include <stdlib.h>
#include "s2Controller.h"
#include "s2UI.h"


S2UI::S2UI(V3DPluginCallback2 &callback, QWidget *parent):   QDialog(parent)
{

    s2Label = new QLabel(tr("smartScope 2"));
    s2LineEdit = new QLineEdit("01b");
    startS2PushButton = new QPushButton(tr("Start smartScope2"));
    startScanPushButton = new QPushButton(tr("start scan"));
    loadScanPushButton = new QPushButton(tr("load last scan"));
    startPosMonButton = new QPushButton(tr("start monitor"));
    buttonBox1 = new QDialogButtonBox;
    buttonBox1->addButton(startS2PushButton, QDialogButtonBox::ActionRole);
    buttonBox1->addButton(startScanPushButton, QDialogButtonBox::RejectRole);
    buttonBox1->addButton(loadScanPushButton, QDialogButtonBox::RejectRole);




    connect(startS2PushButton, SIGNAL(clicked()), this, SLOT(startS2()));
    connect(startScanPushButton, SIGNAL(clicked()), &myController, SLOT(startScan()));
    connect(loadScanPushButton, SIGNAL(clicked()), this, SLOT(loadScan()));
    connect(startPosMonButton,SIGNAL(clicked()), this, SLOT(posMonButtonClicked()));
    connect(&myController,SIGNAL(newBroadcast(QString)), this, SLOT(updateString(QString)));
    connect(&myPosMon, SIGNAL(newBroadcast(QString)), this, SLOT(updateString(QString)));
    connect(&myPosMon, SIGNAL(pmStatus(bool)), this, SLOT(pmStatusHandler(bool)));
    connect(&myPosMon, SIGNAL(newS2Parameter(QMap<int,S2Parameter>)), this, SLOT(updateS2Data(QMap<int,S2Parameter>)));
    connect(this, SIGNAL(startPM()), &myPosMon, SLOT(startPosMon()));
    connect(this, SIGNAL(stopPM()), &myPosMon, SLOT(stopPosMon()));

    v3dhandle curwin = callback.currentImageWindow();
    /*if (!curwin)
    {
        v3d_msg("Please open an image.");
        return;
    }*/


    mainLayout = new QGridLayout();
    mainLayout->addWidget(s2Label, 0, 0);
    mainLayout->addWidget(s2LineEdit, 0, 1);
    mainLayout->addWidget(buttonBox1,1,0,2,4);
    mainLayout->addWidget(startPosMonButton,3,0);
    // add fields with data...  currently hardcoding the number of parameters...
    for (int jj=0; jj<=7; jj++){
    QLabel * labeli = new QLabel(tr("test"));
    labeli->setText(QString::number(jj));
    labeli->setObjectName(QString::number(jj));
    mainLayout->addWidget(labeli,jj+4,0,1,4);
    }



    setLayout(mainLayout);
    setWindowTitle(tr("smartScope2 Interface"));



}


void S2UI::startS2()
{
     myController.show();
     myPosMon.show();
     myPosMon.setWindowTitle(tr("posMon"));
}

void S2UI::startScan()
{
   /* if (!myController)
    {
        v3d_msg("please start smartScope2");
    return;
    }*/
    myController.cleanAndSend("-mto test");
}

void S2UI::loadScan(){
 myController.getROIData(); // this should really be a signal to myController,
                            // not an explicit call
}

void S2UI::displayScan(){ // this will listen for a signal from myController
    //containing either a filename or  eventually an address

}

void S2UI::pmStatusHandler(bool pmStatus){
    posMonStatus = pmStatus;
    s2LineEdit->setText(tr("pmstatus updated"));
}

void S2UI::posMonButtonClicked(){
    // if it's not running, start it
    // and change button text to 'stop pos mon'
    if (!posMonStatus){
        emit startPM();
        s2LineEdit->setText(tr("pm stop"));
        startPosMonButton->setText(tr("stop position monitor"));
    }
    else{
        emit stopPM();
        startPosMonButton->setText(tr("start position monitor"));
    }
    // if it's running, stop it
    // and change text to start pos mon


}
void S2UI::updateS2Data( QMap<int, S2Parameter> currentParameterMap){
for (int i: currentParameterMap.keys()){
    QString parameterStringi = currentParameterMap[i].getParameterName();
    float parameterValuei = currentParameterMap[i].getCurrentValue();
    QString iString = QString::number(i);
    if (currentParameterMap[i].getExpectedType().contains("string")){
    parameterStringi.append(" = ").append(currentParameterMap[i].getCurrentString());
    }
    if (currentParameterMap[i].getExpectedType().contains("float")){
    parameterStringi.append(" = ").append(QString::number(parameterValuei));
    }
    if (currentParameterMap[i].getExpectedType().contains("list")){
    parameterStringi.append(" = ").append(currentParameterMap[i].getCurrentString().split(".xml").first());
    }
    QLabel* item = this->findChild<QLabel*>( iString);
    if (item){
        item->setText(parameterStringi);
    }
}

}


void S2UI::updateString(QString broadcastedString){
    //s2LineEdit->setText(broadcastedString);
}
