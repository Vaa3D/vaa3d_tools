

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
    void loadScan();
    void displayScan();
private:
    QLabel *s2Label;
    QLineEdit *s2LineEdit;
    QPushButton *startS2PushButton;
    QPushButton *startScanPushButton;
    QPushButton *loadScanPushButton;
    QDialogButtonBox *buttonBox1;
    S2Controller myController;

};
//! [0]

#endif
