#ifndef DOWNSAMPLE_WIDGET_H
#define DOWNSAMPLE_WIDGET_H

#include <QtGui/QDialog>
#include <QFileDialog>
#include <v3d_interface.h>
#include <iostream>

using namespace std;

class DownsampleDialog : public QDialog
{
    Q_OBJECT
public:
    QString file = "";
    QString files = "";
    QString fileSave = "";
    QString folderSave = "";
    QSpinBox * res_output;
    QHBoxLayout * hbox1;
    QHBoxLayout * hbox2;
    QPushButton * input_file_Btn;
    QPushButton * input_folder_Btn;
    QLineEdit * file_edit;
    QLineEdit * folder_edit;
    QHBoxLayout * hbox_save;
    QLineEdit * files_save;
    QPushButton * saveBtn;
    QHBoxLayout * hbox3;
    QPushButton * ok;
    QPushButton * cancel;
    QGridLayout * gridLayout;
    static void downsampleEswc(const QString file, QString saveName, double down);
    static void downsampleApo(const QString file, QString saveName, double down);
    static QString nameSaveFile(const QString file);

public:
    DownsampleDialog(QWidget *parent);

    ~DownsampleDialog(){}

public slots:

    void selectFiles();

    void saveFiles();
};



#endif // DOWNSAMPLE_WIDGET_H
