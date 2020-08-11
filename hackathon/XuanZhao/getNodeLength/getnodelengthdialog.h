#ifndef GETNODELENGTHDIALOG_H
#define GETNODELENGTHDIALOG_H

#include <QtGui>
#include "v3d_interface.h"
#include <iostream>

using namespace std;

class getNodeLengthDialog : public QDialog
{
    Q_OBJECT
public:
    NeuronTree nt;
//    v3dhandle cur;
    V3dR_MainWindow* cur;

    QLineEdit* swcEdit;
    V3DPluginCallback2* callback;
    QSlider* slider;
    QLineEdit* lineEdit;

    getNodeLengthDialog(QWidget* parent, V3DPluginCallback2& callback);

public slots:
    void setLineEditValue(int value);
    void readSWC();
};

#endif // GETNODELENGTHDIALOG_H
