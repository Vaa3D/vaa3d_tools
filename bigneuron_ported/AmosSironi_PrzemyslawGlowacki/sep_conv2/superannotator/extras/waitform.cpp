#include "waitform.h"
#include "ui_waitform.h"

#include <QMovie>

WaitForm::WaitForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitForm)
{
    ui->setupUi(this);

    mMovie = new QMovie(":/icons/animated/waitIcon.gif", QByteArray(), this);

    ui->label->setMovie( mMovie );
    mMovie->start();
}

WaitForm::~WaitForm()
{
    delete ui;
}
