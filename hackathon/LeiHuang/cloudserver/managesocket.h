#ifndef MANAGESOCKET_H
#define MANAGESOCKET_H
#include <QtNetwork>
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
#endif // MANAGESOCKET_H
