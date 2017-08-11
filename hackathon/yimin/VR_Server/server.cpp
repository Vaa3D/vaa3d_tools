#include "server.h"
#include <QString>
#include <QRegExp>


std::vector<clientproperty> clientsproperty;
Server::Server(QObject* parent) : QObject(parent) {
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()),
            this,   SLOT(onNewConnection()));

    if (!server->listen(QHostAddress::Any, PORT)) {
        qDebug() << "Server is not started.";
    } else {
        qDebug() << "Server is started.";
    }
	clientNum=0;
}


void Server::sendUserList() {
    QString line = "/users:" + ((QStringList)clients.values()).join(",") + "\n";
    sendToAll(line);
}


void Server::sendToAll(const QString& msg) {
    foreach (QTcpSocket* socket, clients.keys()) {
        socket->write(msg.toUtf8());
    }
}
void Server::sendColorMsg() {
	foreach (QTcpSocket* socket, clients.keys()) {
		QString username = clients.value(socket);
		for(int i=0;i<clientsproperty.size();i++)
		{
			if(clientsproperty.at(i).name!=username) continue;
			QString msg= QString::number(clientsproperty.at(i).colortype, 10);
			sendToAll(QString("/color:"+username+" "+msg+" \n"));
		}
	}

}

void Server::onNewConnection() {
    QTcpSocket* socket = server->nextPendingConnection();
    qDebug() << "Client connected: " << socket->peerAddress().toString();

    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));

    clients.insert(socket, "");
	clientNum=(clientNum+1)%17;

	qDebug()<<"the num of this client is " <<clientNum;

}

void Server::onDisconnect() {
    QTcpSocket* socket = (QTcpSocket*)sender();
    qDebug() << "Client disconnected: " << socket->peerAddress().toString();

    QString username = clients.value(socket);
    sendToAll(QString("/system:" + username + " left .\n"));
    clients.remove(socket);
	for(int i=0;i<clientsproperty.size();i++)
	{
		if(clientsproperty.at(i).name!=username) continue;
		clientsproperty.erase(clientsproperty.begin()+i);
		i--;
	}
    sendUserList();
}


void Server::onReadyRead() {
    QRegExp loginRex("^/login:(.*)$");
    QRegExp messageRex("^/say:(.*)$");
	QRegExp hmdposRex("^/hmdpos:(.*)$");
    QTcpSocket* socket = (QTcpSocket*)sender();
    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();
        if (loginRex.indexIn(line) != -1) {
            QString user = loginRex.cap(1);
            clients[socket] = user;
            sendToAll(QString("/system:" + user + " joined .\n"));
            sendUserList();
            qDebug() << user << "logged in.";
			clientproperty client00={
									clientNum,
									user,
									21
			};
			client00.colortype=clientNum+2;//21+(clientNum*20)%(275-21);
			clientsproperty.push_back(client00);
			sendColorMsg();
        }
        else if (messageRex.indexIn(line) != -1) {
            QString user = clients.value(socket);
            QString msg = messageRex.cap(1);
            sendToAll(QString(user + ":" + msg + "\n"));
            qDebug() << "User:" << user;
            qDebug() << "Message:" << msg;
        }
        else if (hmdposRex.indexIn(line) != -1) {
			//qDebug()<<"run here";
            QString user = clients.value(socket);
            QString hmd = hmdposRex.cap(1);
            sendToAll(QString("/hmdpos:" +user+" "+hmd + " \n"));
            qDebug() << "User:" << user;
            qDebug() << "HMD Position:" << hmd;
        }
        else {
            qDebug() << "Bad message from " << socket->peerAddress().toString();
        }
    }
}

