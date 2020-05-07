#ifndef MANAGESERVER_H
#define MANAGESERVER_H
#include <QtNetwork>
#include <QRegExp>
#include <QDir>
#include <QFileInfoList>
#include <QMap>
#include "receive_file.h"

#include "messageserver.h"
//class ManageSocket;
QString currentDir();
QString currentDirImg();

class FileServer;

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
    QMap <QString ,MessageServer* > Map_File_MessageServer;//ANO ->MessageServer
    QMap <QString,int> userList;//username->userid ,userid was in eswc
public:
signals:
    void userload(ForAUTOSave);

};
#endif // MANAGESERVER_H
