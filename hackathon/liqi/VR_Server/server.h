#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QStringList>
#include <QMap>

#include "basic_surf_objs.h"

#define PORT 1234


class Server : QObject {
    Q_OBJECT
public:
    explicit Server(QObject* parent = 0);
    void sendUserList();
    void sendToAll(const QString&);
	void sendColorMsg();
	void AddToNTList(QString &msg, int type);
	bool DeleteSegment(QString segName);
public slots:
    void onNewConnection();
    void onDisconnect();
    void onReadyRead();
private:
    QTcpServer* server;
    QMap<QTcpSocket*,QString> clients;
	QMap<QString,int> historyclients;
	int clientNum;
	NeuronTree wholeNT;
	QList<NeuronTree> NeuronList;
	int sketchNum;
};

struct clientproperty{
	int number;
	QString name;
	int colortype;
};
#endif // SERVER_H
