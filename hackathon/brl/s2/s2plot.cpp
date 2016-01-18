#include "s2plot.h"
#include "ui_s2plot.h"

S2Plot::S2Plot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::S2Plot)
{
    ui->setupUi(this);
    plotGS = new QGraphicsScene();
    plotGS->addText("test");
    myRect = plotGS->addRect(10,15, 12, 55);
    ui->mainGV->setScene(plotGS);

}

S2Plot::~S2Plot()
{
    delete ui;
}
