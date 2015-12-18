#include "v3d_message.h"
#include <QDebug>
#include <QtGui>
#include <QtNetwork>
#include <stdlib.h>
#include "s2UI.h"



S2UI::S2UI(V3DPluginCallback2 &callback, QWidget *parent):   QDialog(parent)
{
    s2Label = new QLabel(tr("smartScope 2"));
    s2LineEdit = new QLineEdit("01");

    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("Please open an image.");
        return;
    }


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(s2Label, 0, 0);
    mainLayout->addWidget(s2LineEdit, 0, 1);
    setLayout(mainLayout);
    setWindowTitle(tr("smartScope2 Client"));

}

//  thinking this through (again...)
// I want the UI to instantiate an S2Controller class and hook up the relevant UI components to
// the controller.  S2Controller will be defined in its own file s2Controller.cpp and .h
//
