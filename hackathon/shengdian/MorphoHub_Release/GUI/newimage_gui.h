#ifndef NEWIMAGE_GUI_H
#define NEWIMAGE_GUI_H
#include <QDialog>
#include <QtGui>
#include <QWidget>
#include "morphohub_dbms_basic.h"
using namespace std;
class NewImage_gui: public QDialog
{
    Q_OBJECT
private:
    mImage *newimage;
    QGridLayout *gridlayout;
    QLineEdit * iSampleID;
    QLineEdit * iSourceID;
    QLineEdit * iFormat;
    QLineEdit * iObject;
    QLineEdit * iSizeX;
    QLineEdit * iSizeY;
    QLineEdit * iSizeZ;
    QLineEdit * iResolutionX;
    QLineEdit * iResolutionY;
    QLineEdit * iResolutionZ;
    QLineEdit * iBit;
    QLineEdit * iComments;
    QPushButton * ok;
    QPushButton * cancel;

public:
    explicit NewImage_gui(mImage &newI,QWidget *parent = 0);
    ~NewImage_gui();
    NewImage_gui();
    void initialization();

public slots:
    void reject() { return QDialog::reject();}
    void okButton_slot();
};

#endif // NEWIMAGE_GUI_H
