#include "mydialog.h"

mydialog::mydialog()
{
    this->setWindowTitle("Para get Window");
    //this->resize(800,500);
    centarldialog=new QDialog(this);

    eswcfiles=new QLabel;
    eswcfiles->setText("open eswcfiles");
    brain=new QLabel;
    brain->setText("open brain");
    savepath=new QLabel;
    savepath->setText("save path");
    e_eswcfiles=new QLineEdit;
    e_brain=new QLineEdit;
    e_savepath=new QLineEdit;
    s_eswcfiles=new QPushButton;
    s_eswcfiles->setText("..");
    s_brain=new QPushButton;
    s_brain->setText("..");
    s_savepath=new QPushButton;
    s_savepath->setText("..");

    layout1=new QGridLayout;
    layout1->addWidget(eswcfiles,0,0);
    layout1->addWidget(e_eswcfiles,0,1);
    layout1->addWidget(s_eswcfiles,0,2);
    layout1->addWidget(brain,1,0);
    layout1->addWidget(e_brain,1,1);
    layout1->addWidget(s_brain,1,2);
    layout1->addWidget(savepath,2,0);
    layout1->addWidget(e_savepath,2,1);
    layout1->addWidget(s_savepath,2,2);

    dx=new QLabel;
    dx->setText("dx");
    dy=new QLabel;
    dy->setText("dy");
    dz=new QLabel;
    dz->setText("dz");
    e_dx=new QSpinBox;
    e_dx->setRange(1,65535);
    e_dx->setValue(256);
    e_dy=new QSpinBox;
    e_dy->setRange(1,65535);
    e_dy->setValue(256);
    e_dz=new QSpinBox;
    e_dz->setRange(1,65535);
    e_dz->setValue(128);

    layout2=new QGridLayout;
    layout2->addWidget(dx,0,0);
    layout2->addWidget(e_dx,0,1);
    layout2->addWidget(dy,1,0);
    layout2->addWidget(e_dy,1,1);
    layout2->addWidget(dz,2,0);
    layout2->addWidget(e_dz,2,1);

    singleTree=new QCheckBox;
    singleTree->setText("singleTree");

    button_ok=new QPushButton("ok");
    button_ok->setDefault(true);
    button_ok->raise();
    button_cancel=new QPushButton("cancel");
    button_cancel->raise();

    hbox1=new QHBoxLayout;
    hbox1->addWidget(button_ok);
    hbox1->addWidget(button_cancel);

    layout_central=new QGridLayout(centarldialog);
    layout_central->addLayout(layout1,0,0);
    layout_central->addLayout(layout2,1,0);
    layout_central->addWidget(singleTree,2,3);
    layout_central->addLayout(hbox1,3,0);

    this->setLayout(layout_central);

    connect(s_eswcfiles,SIGNAL(clicked()),this,SLOT(on_pushButton_s_eswcfiles_clicked()));
    connect(s_brain,SIGNAL(clicked()),this,SLOT(on_pushButton_s_brain_clicked()));
    connect(s_savepath,SIGNAL(clicked()),this,SLOT(on_pushButton_s_savepath_clicked()));
    connect(button_ok,SIGNAL(clicked()),this,SLOT(accept()));
    connect(button_cancel,SIGNAL(clicked()),this,SLOT(reject()));


}


mydialog::~mydialog()
{
    delete centarldialog;
    delete layout_central;
    delete layout1;
    delete layout2;
    delete hbox1;
    delete eswcfiles;
    delete brain;
    delete savepath;
    delete e_eswcfiles;
    delete e_brain;
    delete e_savepath;
    delete s_eswcfiles;
    delete s_brain;
    delete s_savepath;
    delete dx;
    delete dy;
    delete dz;
    delete e_dx;
    delete e_dy;
    delete e_dz;
    delete singleTree;
    delete button_ok;
    delete button_cancel;
}

void mydialog::on_pushButton_s_eswcfiles_clicked()
{
    QStringList files;
    files=QFileDialog::getOpenFileNames(this,"eswc select");
    for(int i=0;i<files.size();++i)
    {
        e_eswcfiles->insert(files[i]);
        e_eswcfiles->insert(" ");
    }
}

void mydialog::on_pushButton_s_brain_clicked()
{
    QString tmp=QFileDialog::getExistingDirectory(this,"brain select");
    e_brain->insert(tmp);
}

void mydialog::on_pushButton_s_savepath_clicked()
{
    QString tmp=QFileDialog::getExistingDirectory(this,"save_path select");
    e_savepath->insert(tmp);
}

parameter1 mydialog::getParameter()
{
    parameter1 p;
    p.dx=e_dx->value();
    p.dy=e_dy->value();
    p.dz=e_dz->value();
    p.singleTree=singleTree->isChecked();
    QString tmp=e_eswcfiles->text();
    p.eswcfiles=tmp.split(" ");
    p.brain=e_brain->text();
    p.savepath=e_savepath->text();

    /*if(this->exec()== QDialog::Accepted)
    {
        delete this;
    }*/
    return p;
}
