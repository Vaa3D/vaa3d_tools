#include "judgebranchdialog.h"


JudgeBranchDialog::JudgeBranchDialog(QWidget* parent, V3DPluginCallback2& callback)
{
    this->callback = &callback;
    rf = 0;
    saveEdit = new QLineEdit(QObject::tr("SaveRandomForest"));
    readEdit = new QLineEdit(QObject::tr("ReadRandomForest"));
    saveEdit->setFixedWidth(400);
    readEdit->setFixedWidth(400);

    QPushButton* saveButton = new QPushButton(QObject::tr("..."));
    QPushButton* readButton = new QPushButton(QObject::tr("..."));

    QPushButton* trainButton = new QPushButton(QObject::tr("Train"));
    QPushButton* judgeBranchButton = new QPushButton(QObject::tr("JudgeBranch"));

    connect(saveButton,SIGNAL(clicked()),this,SLOT(_slots_saveRandomForest()));
    connect(readButton,SIGNAL(clicked()),this,SLOT(_slots_readRandomForest()));
    connect(trainButton,SIGNAL(clicked()),this,SLOT(_slots_train()));
    connect(judgeBranchButton,SIGNAL(clicked()),this,SLOT(_slots_judgeBranch()));

    QGridLayout* layout = new QGridLayout();
    layout->addWidget(saveEdit,1,1);
    layout->addWidget(saveButton,1,2);
    layout->addWidget(readEdit,2,1);
    layout->addWidget(readButton,2,2);
    layout->addWidget(trainButton,3,1);
    layout->addWidget(judgeBranchButton,3,2);

    setLayout(layout);
    setWindowTitle(QObject::tr("Judge Branch"));
}

void JudgeBranchDialog::_slots_readRandomForest(){
    QString dirRF = QFileDialog::getExistingDirectory(this,QObject::tr("Read Random Forest..."));
    readEdit->setText(dirRF);
    if(rf)
        delete rf;
    rf = new RandomForest();
    if(rf->readRandomForest(dirRF)){
        cout<<"tree size: "<<rf->getTrees().size()<<endl;
        cout<<"C: "<<rf->C<<" M: "<<rf->M<<" numTrees: "<<rf->numTrees<<endl;
        QMessageBox::information(0, "", "Read Successfully!");
    }else {
        QMessageBox::information(0, "", "Read failed!");
        if(rf)
            delete rf;
        rf = 0;
    }

}

void JudgeBranchDialog::_slots_saveRandomForest(){
    QString dirRF = QFileDialog::getExistingDirectory(this,QObject::tr("Save Random Forest..."));
    saveEdit->setText(dirRF);
    if(rf->saveRandomForest(dirRF)){
        QMessageBox::information(0, "", "Save Successfully!");
    }else {
        QMessageBox::information(0, "", "Save failed!");
    }

}

void JudgeBranchDialog::_slots_judgeBranch(){
    if(!rf){
        QMessageBox::information(0, "", "Random Forest is null!");
        return;
    }
    judgeBranch(rf,callback);
    QMessageBox::information(0, "", "judeg is ended!");
}

void JudgeBranchDialog::_slots_train(){
    rf = train(rf,callback);
    QMessageBox::information(0, "", "Train is ended!");
}
