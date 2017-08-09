#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QStringList>
#include <QMap>

#define PORT 1234


class Server : QObject {
    Q_OBJECT
public:
    explicit Server(QObject* parent = 0);
    void sendUserList();
    void sendToAll(const QString&);
public slots:
    void onNewConnection();
    void onDisconnect();
    void onReadyRead();
private:
    QTcpServer* server;
    QMap<QTcpSocket*,QString> clients;
};

#endif // SERVER_H
