#ifndef SEND_FILE_H
#define SEND_FILE_H
#include <QtNetwork>
class FileSocket_send:public QTcpSocket
{
    Q_OBJECT
public:
    explicit FileSocket_send::FileSocket_send
    (QString ip,QString port,QString anofile_path,QObject *parent=0);

    void sendFile(QString filepath,QString filename);
public slots:
    void readMSG();
private:
    QString anopath;
    QString anoname;
};
#endif // SEND_FILE_H
