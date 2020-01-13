#ifndef MANAGESERVER_H
#define MANAGESERVER_H
#include <QtNetwork>
#include <QRegExp>
#include <QDir>
#include <QFileInfoList>
#include <QMap>
#include "receive_file.h"
#include "send_file.h"
#include "messageserver.h"
QString currentDir();
class FileServer;

class ManageSocket:public QTcpSocket
{
    Q_OBJECT
public:
    explicit ManageSocket(QObject *parent=0);
public slots:
    void readManage();
    void resetfileserver();
    void ondisconnect();

protected:
signals:
    void makeMessageServer(ManageSocket *managesocket,QString anofile_name);

private:



};

class ManageServer:public QTcpServer
{
    Q_OBJECT
public:
    explicit ManageServer(QObject *parent=0);
private:
    void incomingConnection(int socketDesc);
public slots:
    void makeMessageServer(ManageSocket *managesocket,QString anofile_name);
    void messageserver_ondeltete(QString);
public:
    QMap <QString ,MessageServer* > Map_File_MessageServer;
public:

};



#endif // MANAGESERVER_H
