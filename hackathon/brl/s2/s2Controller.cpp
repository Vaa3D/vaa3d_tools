#include "v3d_message.h"
#include <QDebug>
#include <QtGui>
#include <QtNetwork>
#include <QRegExp>
#include <stdlib.h>
#include "s2Controller.h"


//! [0]
S2Controller::S2Controller(QWidget *parent):   QWidget(parent), networkSession(0)
{

    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));
    cmdLabel = new QLabel(tr("Command:"));

    QString ipAddress;
    ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    int portnumber= 1236;
    hostLineEdit = new QLineEdit("10.128.50.139");
    portLineEdit = new QLineEdit("1236");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    cmdLineEdit = new QLineEdit;

    hostLabel->setBuddy(hostLineEdit);
    portLabel->setBuddy(portLineEdit);

    statusLabel = new QLabel(tr(" - - - "));

    sendCommandButton = new QPushButton(tr("Send Command"));
    sendCommandButton->setDefault(true);
    sendCommandButton->setEnabled(false);
    connectButton = new QPushButton(tr("connect to PrairieView"));
    connectButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"));
    getReplyButton = new QPushButton(tr("get reply"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(sendCommandButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
    buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(getReplyButton, QDialogButtonBox::ActionRole);




//! [1]
    tcpSocket = new QTcpSocket(this);
//! [1]

    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enablesendCommandButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enablesendCommandButton()));
    connect(sendCommandButton, SIGNAL(clicked()),
            this, SLOT(sendCommand()));
    connect(connectButton, SIGNAL(clicked()), this, SLOT(initializeS2()));
//! [2] //! [3]
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readPV()));
    connect(getReplyButton, SIGNAL(clicked()), this, SLOT(readPV()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(sendX()));
//! [2] //! [4]
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
//! [3]
            this, SLOT(displayError(QAbstractSocket::SocketError)));
//! [4]

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(cmdLabel, 2,0);
    mainLayout->addWidget(cmdLineEdit,2,1);
    mainLayout->addWidget(statusLabel, 3, 0, 1, 3);
    mainLayout->addWidget(buttonBox, 4, 0, 1, 3);
    setLayout(mainLayout);

    setWindowTitle(tr("smartScope2 Controller"));
    portLineEdit->setFocus();

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        sendCommandButton->setEnabled(false);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    }
//! [5]
}


void S2Controller::initializeS2(){
    initializeParameters();
    connectToS2();
}

void S2Controller::initConnection(){
}

void S2Controller::initializeParameters(){
    S2Data myS2Data;

}

void S2Controller::connectToS2()
{
    tcpSocket->connectToHost(hostLineEdit->text(),
    portLineEdit->text().toInt());
    }

void S2Controller::sendCommand()
{
    sendCommandButton->setEnabled(false);
    cleanAndSend(cmdLineEdit->text());
}

void S2Controller::cleanAndSend(QString inputString)
{
    inputString.replace(' ', (char)1).append((char)13).append((char)10);
    tcpSocket->write(inputString.toLatin1());
    sendCommandButton->setEnabled(true);
}

void S2Controller::sendX()
{
    const QString xQ = QString("-x");
    cmdLineEdit->setText(xQ);
    cleanAndSend(xQ);
    QTimer::singleShot(100, this, SLOT(cleanUp()));
}

void S2Controller::cleanUp()
{
    tcpSocket->close();
    close();
}

void S2Controller::readPV()
{
//! [9]
    QTextStream in(tcpSocket);
    /*in.setVersion(QDataStream::Qt_4_0);
    if (blockSize == 0) {
        if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
            return;
        in >> blockSize;
    }
    if (tcpSocket->bytesAvailable() < blockSize)
        return;*/
    QString pvResponse;
    pvResponse = tcpSocket->readAll( );
    totalMessage.append(pvResponse.toLatin1());

    // check for ACK and DONE?
    // if totalMessage contains ACK and DONE,
    // remove them and intervening characters from totalMessage
    // place what's between them into currentMessage and emit
    // currentMessage to a new field.

    // actually need to break this out some more-  check for ACK and emit signal to
    // update sent/waiting/done status (new attribute!)
    // then if the same string has ACK-*-DONE, process the string.
    // if not, this method (readPV) will change behavior to wait for DONE.

    const QRegExp rx("DONE");
    if (totalMessage.contains("DONE")) {
        QStringList mList;
        mList = totalMessage.split("ACK\r\n");
        currentMessage = QString(mList.last().split("\r\nDONE\r\n").first());
        QString displayMessage;
        QString stringToRemove;
        stringToRemove = currentMessage;
        displayMessage = currentMessage;
        stringToRemove.append("\r\nDONE\r\n").prepend("ACK\r\n");
        statusLabel->setText(displayMessage);
        totalMessage.remove(stringToRemove);
        sendCommandButton->setEnabled(true);
        qDebug()<<totalMessage;
    }

}


void S2Controller::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("smartScope2 Controller"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("smartScope2 Controller"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure PrairieView is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("smartScope2 Controller"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    sendCommandButton->setEnabled(true);
}
//! [13]

void S2Controller::enablesendCommandButton()
{
    sendCommandButton->setEnabled((!networkSession || networkSession->isOpen()) &&
                                 !hostLineEdit->text().isEmpty() &&
                                 !portLineEdit->text().isEmpty());
    connectButton->setEnabled((!networkSession || networkSession->isOpen()) &&
                                 !hostLineEdit->text().isEmpty() &&
                                 !portLineEdit->text().isEmpty());
}

void S2Controller::sessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

    statusLabel->setText(tr("Prototype Controller requires "
                            "PrairieView to run at the same time."));

    enablesendCommandButton();
}

void S2Controller::initROI(){//    set up the microscope with appropriate parameters for small 3D ROI.  This could be done with a single .xml file from a saved configuration or through setting parameters from Vaa3D.
}

void S2Controller::startROI(){ //    set a target file location and trigger the 3D ROI.
}

void S2Controller::getROIData(){ //    FILE VERSION: Wait for PV to signal ROI completion (?), wait for arbitrary delay or poll filesystem for available file
       //                        //SHARED MEMORY VERSION: during ROI initiation, Vaa3D will allocate a new 1d byte array and send the address and length to PV. It might be a bit tricky to know when this data is valid.
}

void S2Controller::processROIData(){ //Process image data and return 1 or more next locations.  Many alternative approaches could be used here, including: Run APP2 and locate ends of structure on boundary.  Identify foreground blobs in 1-D max or sum projections of ROI faces. Identify total intensity and variance in the entire ROI. Identify total tubularity in the ROI or near the edges, etc etc.  In any case, the resulting image coordinates will be transformed into coordinates that PV understands for (e.g.) "PanXY"  commands.
}
void S2Controller::startNextROI(){//   Move to the next ROI location and start the scan.  With the new 'PanXY' command, this should be trivial.
}





