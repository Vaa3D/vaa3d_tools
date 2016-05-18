#include "v3d_message.h"
#include <QDebug>
#include <QtGui>
#include <QtNetwork>
#include <QRegExp>
#include <stdlib.h>
#include "s2Controller.h"
#include "v3d_message.h"
#include <s2UI.h>


S2Parameter::S2Parameter():
    parameterName(""),
    currentValue(0.0),
    currentString(""),
    expectedType("float"){}

S2Parameter::S2Parameter(QString parameterN,
                         QString sendS,
                         float currentV,
                         QString currentS,
                         QString expectedT){
    sendString = sendS;
    parameterName = parameterN;
    currentValue=currentV;
    currentString = currentS;
    expectedType = expectedT;
}

QString S2Parameter::getSendString(){
    return sendString;
}
QString S2Parameter::getCurrentString(){
    return currentString;
}
void S2Parameter::setCurrentString(QString inputString){
    currentString = inputString;
}
float S2Parameter::getCurrentValue(){
    return currentValue;
}
void S2Parameter::setCurrentValue(float value){
    currentValue = value;
}
QString S2Parameter::getExpectedType(){
    return expectedType;
}
QString S2Parameter::getParameterName(){
    return parameterName;
}

//! [0]
S2Controller::S2Controller(QWidget *parent):   QWidget(parent), networkSession(0)
{

    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));
    cmdLabel = new QLabel(tr("Command:"));

    QString ipAddress;
    ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    int portnumber= 1236;
    hostLineEdit = new QLineEdit("10.128.48.53");
    portLineEdit = new QLineEdit("1236");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    cmdLineEdit = new QLineEdit;

    hostLabel->setBuddy(hostLineEdit);
    portLabel->setBuddy(portLineEdit);

    statusLabel = new QLabel(tr(" - - - "));

    sendCommandButton = new QPushButton(tr("Send Command"));
    sendCommandButton->setEnabled(true);
    connectButton = new QPushButton(tr("connect to PrairieView"));
    connectButton->setEnabled(true);

    quitButton = new QPushButton(tr("Quit"));
    getReplyButton = new QPushButton(tr("get reply"));

    buttonBox = new QGroupBox;
    QVBoxLayout *vb = new QVBoxLayout;

    vb->addWidget(sendCommandButton);
    vb->addWidget(quitButton);
    vb->addWidget(connectButton);
    vb->addWidget(getReplyButton);
    buttonBox->setLayout(vb);

    tcpSocket = new QTcpSocket(this);

    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enablesendCommandButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enablesendCommandButton()));
    connect(sendCommandButton, SIGNAL(clicked()),
            this, SLOT(sendCommand()));
    connect(connectButton, SIGNAL(clicked()), this, SLOT(initializeS2()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(checkForMessage()));
    connect(this, SIGNAL(messageIsComplete()), this, SLOT(processMessage()));
    connect(this, SIGNAL(newMessage(QString)),
            this, SLOT(messageHandler(QString)));
    connect(this, SIGNAL(newMessage(QString)),
            this, SLOT(posMonListener(QString)));


    connect(quitButton, SIGNAL(clicked()), this, SLOT(sendX()));

    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(cmdLabel, 2,0);
    mainLayout->addWidget(cmdLineEdit,2,1);
    mainLayout->addWidget(statusLabel, 3, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 4, 0, 1, 2);
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
    //enablesendCommandButton();
    //! [5]
    ii =-1;
    okToSend= true;
    cancelQueue = false;
    cancelPosMon = false;
    inPosMonMode = false;
    commandQueue = QStringList();
    newCommandToSend = false;
}


void S2Controller::initializeS2(){
    initializeParameters();
    connectToS2();
    commandQueueMonitor();
}

void S2Controller::initConnection(){
}

void S2Controller::status(QString statusM){
    emit statusSig(statusM);
}
void S2Controller::initializeParameters(){
    S2Parameter tPara =S2Parameter("currentMode","-gts activeMode", 0.0, "","string");
    s2ParameterMap.insert(0,tPara);
    s2ParameterMap.insert(1, S2Parameter("galvoXVolts", "-gts currentScanCenter XAxis")) ;
    s2ParameterMap.insert(2, S2Parameter("galvoYVolts", "-gts currentScanCenter YAxis")) ;
    s2ParameterMap.insert(3, S2Parameter("piezoZ", "-gmp Z 1")) ;
    s2ParameterMap.insert(4, S2Parameter("stepperZ", "-gmp Z 0")) ;
    s2ParameterMap.insert(5, S2Parameter("stageX", "-gmp X 0")) ;
    s2ParameterMap.insert(6, S2Parameter("stageY", "-gmp Y 0")) ;
    s2ParameterMap.insert(7, S2Parameter("last image", "-gts recentAcquisitions",0.0, "", "list"));
    s2ParameterMap.insert(8, S2Parameter("micronsPerPixelX", "-gts micronsPerPixel XAxis"));
    s2ParameterMap.insert(9, S2Parameter("micronsPerPixelY", "-gts micronsPerPixel YAxis"));
    s2ParameterMap.insert(10,S2Parameter("pixelsPerLine", "-gts pixelsPerLine"));
    s2ParameterMap.insert(11, S2Parameter("linesPerFrame","-gts linesPerFrame"));
    s2ParameterMap.insert(12,S2Parameter("opticalZoom", "-gts opticalZoom"));
    s2ParameterMap.insert(13,S2Parameter("micronROISizeX", "", 0.0, "", "floatderived"));
    s2ParameterMap.insert(14, S2Parameter("micronROISizeY", "", 0, "", "floatderived"));
    s2ParameterMap.insert(15,S2Parameter("maxVoltsX", "-gts maxVoltage XAxis ", 0.0, "float"));
    s2ParameterMap.insert(16,S2Parameter("minVoltsX", "-gts minVoltage XAxis ", 0.0, "float"));

    s2ParameterMap.insert(17,S2Parameter("micronsPerVolt","", 0.0, "", "floatderived"));
    s2ParameterMap.insert(18,S2Parameter("galvoXmicrons", "", 0.0, "", "floatderived"));
    s2ParameterMap.insert(19, S2Parameter("galvoYmicrons", "", 0.0, "", "floatderived"));
    s2ParameterMap.insert(20,S2Parameter("resonantX","-gts currentPanLocationX", 0.0, "float"));
    s2ParameterMap.insert(21,S2Parameter("maxVoltsY", "-gts maxVoltage YAxis ", 0.0, "float"));
    s2ParameterMap.insert(22,S2Parameter("minVoltsY", "-gts minVoltage YAxis ", 0.0, "float"));
    s2ParameterMap.insert(23,S2Parameter("micronsPerVoltY","", 0.0, "", "floatderived"));

    maxParams = s2ParameterMap.keys().last()+1;
    emit newMessage(QString("initialized"));

}

void S2Controller::startScan(){
    sendCommandButton->setEnabled(false);
    sendAndReceive(QString("-ss"));
}

void S2Controller::centerGalvos(){
    sendCommandButton->setEnabled(false);
    QString toSend = QString("-sts currentScanCenter ");
    toSend.append(QString::number(0.0));
    toSend.append(" XAxis");
    cleanAndSend(toSend);
    sendAndReceive(QString("-png center"));
}


void S2Controller::overviewSetup(){
    // idea is to just send a bunch of commands and then the main UI will wait until the parameters are correct.
    // the return data from PV will likely be lost to processMessage(), but that's OK.
    // a more robust? version could query here instead of in S2UI
    centerGalvos();
    cleanAndSend("-zsz 20");
    cleanAndSend("-sts pixelsPerLine 512"); // set pixels per line to 512
    cleanAndSend("-sts linesPerFrame 512"); // set lines per frame to 512
    cleanAndSend("-oz 1"); // set mag to 1x

    //   cleanAndSend("-sts activeMode Galvo");// set to galvo mode

    QTimer::singleShot(10, this, SLOT(overviewHandler()));

}

void S2Controller::stackSetup(){
    // idea is to just send a bunch of commands and then the main UI will wait until the parameters are correct.
    // the return data from PV will likely be lost to processMessage(), but that's OK.
    // a more robust? version could query here instead of in S2UI
    centerGalvos();
    cleanAndSend("-zsz 1.0");
    cleanAndSend("-oz 13"); // set mag to 16x
    cleanAndSend("-sts pixelsPerLine 256"); // set pixels per line to  256
    cleanAndSend("-sts linesPerFrame 256"); // set lines per frame to 256
    //    cleanAndSend("-sts activeMode Galvo");// set to galvo mode


}


void S2Controller::stackSetup(float zsize, float zoom, int pixelsPerLine, int linesPerFrame){
    centerGalvos();
    addToQueue("-zsz "+QString::number(zsize,'f', 4));
    addToQueue("-oz "+QString::number(zoom, 'f',4));
    addToQueue("-sts pixelsPerLine "+QString::number(pixelsPerLine));
    // set mag to 16x
    addToQueue("-sts linesPerFrame "+QString::number(linesPerFrame));

//    cleanAndSend("-zsz "+QString::number(zsize,'f', 4));
//    cleanAndSend("-oz "+QString::number(zoom, 'f',4));
//    cleanAndSend("-sts pixelsPerLine "+QString::number(pixelsPerLine));
//    // set mag to 16x
//    cleanAndSend("-sts linesPerFrame "+QString::number(linesPerFrame));

}

void S2Controller::overviewHandler(){
    // this is now being done in s2UI
}

void S2Controller::connectToS2()
{
    if ( hostLineEdit->text().contains("local")){
        tcpSocket->connectToHost(QHostAddress::LocalHost, portLineEdit->text().toInt());
    }else{
        tcpSocket->connectToHost(hostLineEdit->text(),
                                 portLineEdit->text().toInt());
    }
    status(QString("tcpSocket isopen: ").append(QString::number(tcpSocket->isOpen())));
    status(QString("tcpSocket isvalid: ").append(QString::number(tcpSocket->isValid())));
}

void S2Controller::sendCommand()
{
    sendCommandButton->setEnabled(false);
    sendAndReceive(cmdLineEdit->text());
}
bool S2Controller::sendAndReceive(QString inputString){
    if (!okToSend){return false;}
    okToSend = false;
    if (cleanAndSend(inputString)){return true;
    }

}

void S2Controller::addToQueue(QString cString){
    commandQueue.append(cString);
}

bool S2Controller::cleanAndSend(QString inputString)
{
    inputString.replace(' ', (char)1).append((char)13).append((char)10);
    tcpSocket->write(inputString.toLatin1());
    return true;
}

void S2Controller::sendX()
{
    const QString xQ = QString("-x");
    cmdLineEdit->setText(xQ);
    sendAndReceive(xQ);
    QTimer::singleShot(1000, this, SLOT(cleanUp()));
}

void S2Controller::cleanUp()
{
    tcpSocket->close();
    cmdLineEdit->setText("");
    close();
}

void S2Controller::closeEvent(QCloseEvent *event){
    tcpSocket->close();
    cmdLineEdit->setText("");
}


void S2Controller::checkForMessage(){
    // this will get called by readyRead signal from tcpsocket
    // append any incoming data to a totalString attribute
    stringMessage.append(QString(tcpSocket->readAll()).toLatin1());
    if (stringMessage.contains("DONE\r\n")){
        // check for DONE.  if not, don't do anything!
        // [this will still be called when new data is available]

        // if DONE, emit a signal to processMessage
        emit messageIsComplete();
    }
    // DO NOT unblock commands yet!

}

void S2Controller::processMessage(){
    // once a fullMessage is made, this method should only be called as a slot
    // because it unblocks the ability to send tcp commands
    // - parse the message here, extracting the returned message
    message = QString("");
    QStringList mList;
    mList = stringMessage.split("ACK\r\n");
    message = QString(mList.last().split("DONE\r\n").first());//  still has carriage return
    message.remove("\r\n");
    // and emitting the message, including updating text fields, etc.
    emit newMessage(message);

    // clear the fullMessage buffer  [this should be OK- only this method
    // and checkForMessages should ever access it]  note this is not scalable-
    // there's only one fullMessage at a time.

    stringMessage.clear();
    // unblock commands
    okToSend = true;
    sendCommandButton->setEnabled(true);

}


void S2Controller::messageHandler(QString messageH){
    // slot for handling messages.   first round will
    // just be updating text in this object and the calling UI
    myS2Data.messageString = messageH;
    emit newBroadcast(messageH);
    if (messageH.length() <20){
        statusLabel->setText(messageH);
    }else{
        statusLabel->setText(QString("long message!"));
    }

}

//  new scheme to queue commands:


void S2Controller::commandQueueMonitor(){

    // this loop monitors a
    // queue of commands to send
    // and takes the top one off if the system is ready to send a command
if (cancelQueue){return;}


// pull off the top message and assign it to an attribute of s2Controller : commandToSend
// initiate another loop/handler that will send try to sendandrecieve stringToSend every 10ms or so
if ((commandQueue.length()>0)&&(!newCommandToSend)){
        commandToSend = commandQueue.at(0);
        commandQueue.removeAt(0);
        newCommandToSend = true;
        tryToSend();

    }
QTimer::singleShot(2, this, SLOT(commandQueueMonitor()));
}


void S2Controller::tryToSend(){

    if (!sendAndReceive(commandToSend)){
    QTimer::singleShot(10, this, SLOT(tryToSend()));
    }else{
        newCommandToSend = false;
    }
}


void S2Controller::posMonListener(QString messageL){
    messageL.remove("\r\n");
    float newValue = messageL.toFloat();
    if (s2ParameterMap[ii].getExpectedType().contains("derived")){
        if (ii==13){
            newValue = s2ParameterMap[10].getCurrentValue()*s2ParameterMap[8].getCurrentValue();
        }else if (ii==14){
            newValue =  s2ParameterMap[11].getCurrentValue()*s2ParameterMap[9].getCurrentValue();
        }else if (ii == 17){
            newValue = s2ParameterMap[13].getCurrentValue()/(s2ParameterMap[15].getCurrentValue()-s2ParameterMap[16].getCurrentValue());
            if (newValue<0){
                newValue = -newValue;}

        }else if (ii == 18){
            newValue = s2ParameterMap[17].getCurrentValue()*s2ParameterMap[1].getCurrentValue();
        }else if (ii == 19){
            newValue = s2ParameterMap[17].getCurrentValue()*s2ParameterMap[2].getCurrentValue();
        }else if (ii == 23){
        newValue = s2ParameterMap[13].getCurrentValue()/(s2ParameterMap[21].getCurrentValue()-s2ParameterMap[22].getCurrentValue());
        if (newValue<0){
            newValue = -newValue;}
        }


        messageL = QString::number(newValue);
    }

    s2ParameterMap[ii].setCurrentString( messageL);
    if (ii==1){
        s2ParameterMap[ii].setCurrentValue(-newValue); // x galvo is flipped
    }else{
        s2ParameterMap[ii].setCurrentValue(newValue);
    }
    // use s2ParameterMap to keep track of parameter values.
    if (inPosMonMode){
        ii = (ii+1) % maxParams;
        emit newS2Parameter(s2ParameterMap);
        QTimer::singleShot(2, this, SLOT(posMon()));
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


void S2Controller::initROI(LocationSimple nextLoc){
    //    set up the microscope with appropriate parameters for small 3D ROI.  This could be done with a single .xml file from a saved configuration or through setting parameters from Vaa3D.
    // for now, just move galvos to appropriate location:
    float x = nextLoc.x;
    float y = nextLoc.y;
    QString toSend;
    status(QString("caught nextLoc x= ").append(QString::number(x)).append(QString("  y = ")).append(QString::number(y)));
    if ((x<7.6)&(x>-7.6)){
        if (false){//(s2ParameterMap[0].getCurrentString().contains("Resonant")){//this controller may not know about the s2parametermap!
            toSend = QString("-sts currentPanLocationX ");
            toSend.append(QString::number(x));
        }else{
            toSend = QString("-sts currentScanCenter ");
            toSend.append(QString::number(x));
            toSend.append(" XAxis");
        }
        addToQueue(toSend);
        //cleanAndSend(toSend);
    }else{
        status(QString("X out of bounds!"));}
    if ((y<7.6)&(y>-7.6)){
        QString toSend = QString("-sts currentScanCenter ") ;
        toSend.append(QString::number(y));
        toSend.append(" YAxis");
        addToQueue(toSend);
//        cleanAndSend(toSend);

    }else{
        status(QString("Y out of bounds!"));
    }

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

void S2Controller::startPosMon(){
    status(QString(" start in myPosMon"));
    if (inPosMonMode){
        status("already in Position Monitor mode");
        return;
    }
    inPosMonMode = true;
    cancelPosMon = false;
    emit pmStatus(inPosMonMode);
    ii = 0;
    posMon();

    //

}

void S2Controller::stopPosMon(){
    status(" stop in myPosMon");
    ii = -1;
    cancelPosMon = true;
    inPosMonMode = false;
    emit pmStatus(inPosMonMode);

}

bool S2Controller::getPosMon(){
    return inPosMonMode;
}


void S2Controller::posMon(){
    // send current query string
    if (!s2ParameterMap[ii].getExpectedType().contains("derived")){
        sendAndReceive(s2ParameterMap[ii].getSendString());
    }else{
        posMonListener("");
    }
    //after delay, emit signal and return

}

void S2Controller::startZStack(){
    qDebug()<<"starting z stack in s2Controller";
    //sendAndReceive(QString("-zs"));
    addToQueue(QString("-zs"));
}


// next:
// add optional 'cancel' to flush read and write buffers and release block
// of sending commands.

void S2Controller::cancelQueueSlot(){
    cancelQueue = true;
}


//

// start bulking up the UI to have some useful parameters


// extract file destination, run this on the scope machine.
