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
struct clientproperty{
    int number;
    QString name;
    int colortype;
        bool Creator;
        bool online;
        int messageindex;
};

class Server : QObject {
    Q_OBJECT
public:
    explicit Server(QObject* parent = 0);
    void sendUserList();
    void sendToAll(const QString&);
    void sendToUser(const QString& msg,const QString& name);
	void sendColorMsg();
        void sendCreatorMsg();
	void AddToNTList(QString &msg, int type);
	bool DeleteSegment(QString segName);
        bool containsClient(std::vector<clientproperty> clients,QString name);
        int getuser(std::vector<clientproperty> clients,QString name);
        void UpdateUsermessage(QString user);

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


#endif // SERVER_H
