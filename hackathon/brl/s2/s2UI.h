

#ifndef S2UI_H
#define S2UI_H

#include <QDialog>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTcpSocket>
#include <v3d_interface.h>
#include "s2Controller.h"
QT_BEGIN_NAMESPACE
class QWidget;
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
    QLineEdit *s2LineEdit;
public slots:
    void pmStatusHandler(bool pmStatus);
signals:
    void startPM();
    void stopPM();
private slots:
    void startS2();
    void startScan();
    void loadScan();
    void displayScan();
    void posMonButtonClicked();
    void updateS2Data(  QMap<int, S2Parameter> currentParameterMap);
    void updateString(QString broadcastedString);
private:
    QLabel *s2Label;
    //QLabel *labeli;
    QGridLayout *mainLayout;
    QPushButton *startS2PushButton;
    QPushButton *startScanPushButton;
    QPushButton *loadScanPushButton;
    QPushButton *startPosMonButton;
    QDialogButtonBox *buttonBox1;
    QGroupBox *parameterBox;
    QVBoxLayout *vbox;
    S2Controller myController;
    S2Controller myPosMon;
    bool posMonStatus = false;
};
//! [0]

#endif
