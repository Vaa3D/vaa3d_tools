/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QDebug>
#include <QtGui>
#include <QtNetwork>
#include <stdlib.h>
#include "s2Client.h"



//! [0]
S2Client::S2Client(QWidget *parent):   QDialog(parent), networkSession(0)
{
//! [0]
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));
	cmdLabel = new QLabel(tr("Command:"));

    QString ipAddress;
	ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
	int portnumber= 1236;
    hostLineEdit = new QLineEdit(ipAddress);
    portLineEdit = new QLineEdit("1236");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
	cmdLineEdit = new QLineEdit;

    hostLabel->setBuddy(hostLineEdit);
    portLabel->setBuddy(portLineEdit);

    statusLabel = new QLabel(tr(" - - - "));

    getFortuneButton = new QPushButton(tr("Send Command"));
    getFortuneButton->setDefault(true);
    getFortuneButton->setEnabled(false);
	connectButton = new QPushButton(tr("connect to PrairieView"));
	connectButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
	buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);
//! [1]
    tcpSocket = new QTcpSocket(this);
//! [1]

    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(getFortuneButton, SIGNAL(clicked()),
            this, SLOT(requestNewFortune()));
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectToPV()));
//! [2] //! [3]
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readPV()));
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

    setWindowTitle(tr("smartScope2 Client"));
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

        getFortuneButton->setEnabled(false);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    }
//! [5]
}

void S2Client::connectToPV()
{
    tcpSocket->connectToHost(hostLineEdit->text(),
	portLineEdit->text().toInt());
	}

void S2Client::requestNewFortune()
{
    getFortuneButton->setEnabled(false);
 	cleanAndSend(cmdLineEdit->text());
}

void S2Client::cleanAndSend(QString inputString)
{	
	//QString outputString = cmdLineEdit->text();
	//outputString.replace(' ', (char)1);
	//outputString.append((char)13).append((char)10).toLatin1();
	//tcpSocket->write(outputString);
	inputString.replace(' ', (char)1).append((char)13).append((char)10);
	tcpSocket->write(inputString.toLatin1());
	//tcpSocket->write(cmdLineEdit->text().replace(' ', (char)1).append((char)13).append((char)10).toLatin1());
	getFortuneButton->setEnabled(true);
}

void S2Client::sendX()
{
	const QString xQ = QString("-x");
	cmdLineEdit->setText(xQ);
	cleanAndSend(xQ);
	QTimer::singleShot(100, this, SLOT(cleanUp()));
}

void S2Client::cleanUp()
{
	tcpSocket->close();
	close();
}

void S2Client::readPV()
{   QString pvResponse;
//! [9]
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
	if (tcpSocket->bytesAvailable()==0){
    return;
	}
	in >> pvResponse;

//! [9]
    statusLabel->setText(pvResponse);
    getFortuneButton->setEnabled(true);
}


void S2Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    getFortuneButton->setEnabled(true);
}
//! [13]

void S2Client::enableGetFortuneButton()
{
    getFortuneButton->setEnabled((!networkSession || networkSession->isOpen()) &&
                                 !hostLineEdit->text().isEmpty() &&
                                 !portLineEdit->text().isEmpty());
    connectButton->setEnabled((!networkSession || networkSession->isOpen()) &&
                                 !hostLineEdit->text().isEmpty() &&
                                 !portLineEdit->text().isEmpty());
}

void S2Client::sessionOpened()
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

    statusLabel->setText(tr("This examples requires that you run the "
                            "Fortune Server example as well."));

    enableGetFortuneButton();
}

