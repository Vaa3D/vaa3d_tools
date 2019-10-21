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

    std::vector<QString> messagelist;
    QReadWriteLock lock_messagelist;

    QString Creator;
    QReadWriteLock lock_Creator;

    NeuronTree wholeNT;
    QReadWriteLock lock_wholeNT;

    QList <CellAPO> wholePoint;
    QReadWriteLock lock_wholePoint;

    QList<NeuronTree> NeuronList;
    QReadWriteLock lock_NeuronList;

    int sketchNum;
    QReadWriteLock lock_sketchNum;

    QString filename;
    QTimer *timer;

    quint32 messageUsedIndex;
    QReadWriteLock lock_messageusedindex;

};

class MessageSocket:public QTcpSocket
{
    Q_OBJECT
public:
    explicit MessageSocket(int socketDesc,Global_Parameters *parameters=0,QObject *parent=0);
protected:
    void SendToUser(const QString &msg);//已定义
    void SendToAll(const QString &msg);//已定义 +已使用
    void SendUserList();//已定义
    void SendColortype();//已定义
    void SendCreaorMsg();//已定义

    bool containsClient(const QString &name);//已定义
    int getUser(const QString &name);//已定义
    void updateUserMessage(QString username);//已定义 + 已使用

    void loginProcess(const QString &name);
    void hmdposProcess(const QString &hmd);
    void askmessageProcess();
    void resindexProcess(const QString &msg);

    void segProcess(const QString &msg);
    void deleteProcess(const QString &delsegpos);
    void markerProcess(const QString &markermsg);
    void delmaekerProcess(const QString &delmarkerpos);
//    void dragnodeProcess(const QString &dragnodepos);
private:
    Global_Parameters *global_parameters;
    int socketId;
//    int nextblocksize;


public slots:
    void MessageSocketSlot_start();//已定义+已绑定
    void MessageSocketSlot_Read();//已绑定
    void MessageSocketSlot_disconnect();//已绑定+已定义
    void MessageSocketSlotAnswerToMessageServer_sendtoall(const QString &msg);//已定义
signals:
    void MessageSocketSignalToMessageServer_disconnected();//已使用
    void MessageSocketSignalToMessageServer_sendtoall(const QString &msg);//已使用
    void signal_addseg(QString);
    void signal_delseg(QString);
    void signal_addmarker(QString);
    void signal_delmarker(QString);
};
#endif // MESSAGESOCKET_H
