#ifndef SEND_FILE_H
#define SEND_FILE_H
#include <QtNetwork>
#include <QFile>S
class FileSocket_send;
class FileServer_send:public QTcpServer
{
    Q_OBJECT
public:

    explicit FileServer_send(QObject *parent=0);
    void sendFile(QString ip,QString filename);
    void sendV3draw(QString ip,QString filename);
public slots:
    void onSocketDisconnected();
private:
    void incomingConnection(int socketDesc);
private:
    QList<FileSocket_send *> list;


};

class FileSocket_send:public QTcpSocket
{
    Q_OBJECT
public:
    FileSocket_send(QObject *parent=0);

    void sendFile(QString filepath,QString filename);
public slots:
    void readMSG();
public:
//    QString anopath;
    QString anoname;
};
#endif // SEND_FILE_H
