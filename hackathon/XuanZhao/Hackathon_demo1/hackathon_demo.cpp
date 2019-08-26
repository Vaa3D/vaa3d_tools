#include "hackathon_demo.h"
#include "ui_hackathon_demo.h"

Hackathon_Demo::Hackathon_Demo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Hackathon_Demo)
{
    ui->setupUi(this);
}

Hackathon_Demo::~Hackathon_Demo()
{
    delete ui;
}
