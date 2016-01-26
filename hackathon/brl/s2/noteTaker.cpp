#include "noteTaker.h"


NoteTaker::NoteTaker(QWidget *parent) :
    QWidget(parent)
{
    myTime = new QDateTime;
    mainLayout = new QGridLayout;
    notes = new QTextEdit;
    saveButton = new QPushButton(tr("Save..."));
    updateButton = new QPushButton(tr("update"));
    mainLayout->addWidget(notes, 0,0, 5, 4);
    mainLayout->addWidget(saveButton, 5,3);
    mainLayout->addWidget(updateButton,5,0);
    setLayout(mainLayout);
    connectStuff();
}

void NoteTaker::connectStuff(){
connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
connect(updateButton, SIGNAL(clicked()),this, SLOT(updateStuff()));

}

void NoteTaker::updateStuff(){

    notes->append(myTime->currentDateTime().toString());

}

void NoteTaker::status(QString statString){
    statString.append(myTime->currentDateTime().toString());
    notes->append(statString);
}

void NoteTaker::setSaveDir(QFileInfo saveDir){

}

void NoteTaker::save(){

}
