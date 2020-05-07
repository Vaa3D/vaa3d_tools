#ifndef MESSAGESOCKET_H
#define MESSAGESOCKET_H

#include <QtNetwork>

#include <QString>
#include <QStringList>
#include <QMap>
#include <QThread>
#include <QReadWriteLock>
#include "./basic_c_fun/basic_surf_objs.h"

class MessageSocket;

struct clientproperty{
    int number;
    QString name;
    int colortype;
    bool Creator;
    bool online;
    int messageindex;
    int Creator_res;
};

struct Global_Parameters{
    QMap<MessageSocket *,QString> clients;
    QReadWriteLock lock_clients;

    int clientNum;
    QReadWriteLock lock_clientNum;

    std::vector<clientproperty> clientsproperty;
    QReadWriteLock lock_clientsproperty;
//    std::vector<QString> messagelist;
    QStringList messagelist;
    QReadWriteLock lock_messagelist;

    QString Creator;
    QReadWriteLock lock_Creator;

    NeuronTree wholeNT;
    QReadWriteLock lock_wholeNT;

    QList <CellAPO> wholePoint;
    QReadWriteLock lock_wholePoint;

    QMap <QString,quint64> Map_Ip_NumMessage;

    QString filename;
    QTimer *timer;//auto save

    quint32 messageUsedIndex;
    QReadWriteLock lock_messageusedindex;

    float global_scale;

    QMap <QString,int> *userInfo;
    QReadWriteLock lock_userInfo;

};

class MessageSocket:public QTcpSocket
{
    Q_OBJECT
public:
    explicit MessageSocket(int socketDesc,Global_Parameters *parameters=0,QObject *parent=0);
protected:
    void SendToUser(const QString &msg);
    void SendToAll(const QString &msg);
    void SendUserList();
    void SendColortype();
    void SendCreaorMsg();

    bool containsClient(const QString &name);
    int getUser(const QString &name);
    void updateUserMessage(QString username);

    void loginProcess(const QString &name);
    void hmdposProcess(const QString &hmd);
    void askmessageProcess();
    void resindexProcess(const QString &msg);

    void segProcess(const QString &msg,int undoP=0);
    void deleteProcess(const QString &delsegpos,int undoP=0);
    void markerProcess(const QString &markermsg,int undoP=0);
    void delmarkerProcess(const QString &delmarkerpos);
    void retypeProcess(const QString &retypeMSG);

    void creatorProcess(const QString msg);
//    void dragnodeProcess(const QString &dragnodepos);
private:
    Global_Parameters *global_parameters;
    int socketId;
    quint16 nextblocksize;
    QString id;


public slots:
    void MessageSocketSlot_start();
    void MessageSocketSlot_Read();
    void MessageSocketSlot_disconnect();
    void MessageSocketSlotAnswerToMessageServer_sendtoall(const QString &msg);
signals:
    void MessageSocketSignalToMessageServer_disconnected();
    void MessageSocketSignalToMessageServer_sendtoall(const QString &msg);
    void signal_addseg(QString);
    void signal_delseg(QString);
    void signal_addmarker(QString);
    void signal_delmarker(QString);
    void signal_retype(QString);
};
#endif // MESSAGESOCKET_H
