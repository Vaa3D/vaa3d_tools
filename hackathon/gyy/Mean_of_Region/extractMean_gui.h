#ifndef EXTRACTMEAN_GUI_H
#define EXTRACTMEAN_GUI_H

#include <QtGui/QDialog>
#include <QFileDialog>
#include <v3d_interface.h>
#include <iostream>

using namespace std;

class ExtractMeanCenterDialog: public QDialog
{
    Q_OBJECT

public:
    QString openFolderFiles = "";
    QString saveFolderFiles = "";
    QSpinBox * rangeBox;
    QHBoxLayout * openHbox;
    QHBoxLayout * saveHbox;
    QPushButton * openButton;
    QPushButton * saveButton;
    QLineEdit * openEdit;
    QLineEdit * saveEdit;
    QHBoxLayout * hbox_ok;
    QPushButton * ok;
    QPushButton * cancel;
    QGridLayout * gridLayout;

public:
    ExtractMeanCenterDialog(QWidget *parent);

    ~ExtractMeanCenterDialog(){}

public slots:

    void saveFiles();
    void selectFolder();

};

#endif // EXTRACTMEAN_GUI_H
