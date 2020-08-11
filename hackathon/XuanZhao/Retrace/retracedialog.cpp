#include "retracedialog.h"

RetraceDialog::RetraceDialog(QWidget *parent, V3DPluginCallback2 &callback)
{
    this->callback = &callback;
    eswcEdit = new QLineEdit(QObject::tr("Read eswc"));
    apoEdit = new QLineEdit(QObject::tr("Read apo"));
    brainEdit = new QLineEdit(QObject::tr("Read brain dir"));
    eswcEdit->setFixedWidth(400);
    apoEdit->setFixedWidth(400);
    brainEdit->setFixedWidth(400);
    resolutionEdit = new QLineEdit("1");

    QPushButton* eswcButton = new QPushButton(QObject::tr("..."));
    QPushButton* apoButton = new QPushButton(QObject::tr("..."));
    QPushButton* brainButton = new QPushButton(QObject::tr("..."));

    connect(eswcButton,SIGNAL(clicked()),this,SLOT(_slots_readEswc()));
    connect(apoButton,SIGNAL(clicked()),this,SLOT(_slots_readApo()));
    connect(brainButton,SIGNAL(clicked()),this,SLOT(_slots_readBrain()));

    QGridLayout* glayout = new QGridLayout;
    glayout->addWidget(eswcEdit,1,1);
    glayout->addWidget(eswcButton,1,2);
    glayout->addWidget(apoEdit,2,1);
    glayout->addWidget(apoButton,2,2);
    glayout->addWidget(brainEdit,3,1);
    glayout->addWidget(brainButton,3,2);
    glayout->addWidget(new QLabel("resolution"),4,1);
    glayout->addWidget(resolutionEdit,4,2);


    QPushButton* start = new QPushButton("Start");
    QPushButton* cancel = new QPushButton("Cancel");

    connect(start, SIGNAL(clicked()), this, SLOT(_slots_start()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

    glayout->addWidget(cancel,5,1);
    glayout->addWidget(start,5,2);

    setLayout(glayout);

    setWindowTitle(QObject::tr("Retrace"));
}

void RetraceDialog::_slots_readEswc(){
    eswcPath =  QFileDialog::getOpenFileName(this, QObject::tr("Read ESWC"), "", QObject::tr("Neuron Structure files (*.swc *.eswc)"));
    eswcEdit->setText(eswcPath);
}

void RetraceDialog::_slots_readApo(){
    apoPath =  QFileDialog::getOpenFileName(this, QObject::tr("Read APO"), "", QObject::tr("Neuron Structure files (*.apo)"));
    apoEdit->setText(apoPath);
}

void RetraceDialog::_slots_readBrain(){
    brainPath = QFileDialog::getExistingDirectory(this,QObject::tr("Read Brain Dir..."));
    brainEdit->setText(brainPath);
}

void RetraceDialog::_slots_start(){
    if(eswcPath == "" || apoPath == "" || brainPath == ""){
        QMessageBox::information(0, "", "Please select path!");
        return;
    }
    resolution = resolutionEdit->text().toInt();
    retrace(apoPath,eswcPath,brainPath,resolution,*callback);

}



