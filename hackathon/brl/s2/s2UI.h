

#ifndef S2UI_H
#define S2UI_H

#include <QDialog>
#include <QTcpSocket>
#include <v3d_interface.h>
#include "s2Controller.h"
QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

QT_END_NAMESPACE

//! [0]
class S2UI : public QDialog
{
    Q_OBJECT

public:
    S2UI(V3DPluginCallback2 &callback, QWidget *parent = 0 );

private slots:
    void startS2();
    void startScan();
private:
    QLabel *s2Label;
    QLineEdit *s2LineEdit;
    QPushButton *startS2PushButton;
    QPushButton *startScanPushButton;
    QDialogButtonBox *buttonBox1;
    S2Controller *myController;
    //QObject ss2Params;
    /*  this will include:
     * optical zoom
     * pixel size in microns
     * pixels/line
     * rows/image
     * resonant/nonresonant mode
    * Current scan xy location in microns (within field of view)
    * xy stage position (for future use  )
    * piezo z position
    * stepper z position.
    * piezo v. stepper z mode
    * Pockels cell voltage
    *  ???
    * this object may include polling on its own*/

};
//! [0]

#endif
