#ifndef MYSOCKET_H
#define MYSOCKET_H
#include <QtNetwork>
#include <QString>
#include <QStringList>
#include <QMap>
#include <vector>
#include <QRegExp>
#include <QReadWriteLock>

class  MySocket;

struct clientproperty{
    int number;
    QString name;
    int colortype;
    bool Creator;
    bool online;
   int messageindex;
   int Creator_res;
};


struct Global_Parameters
{
    QMap <MySocket* ,QString> clients;
    QReadWriteLock lock_clients;

    int clientNum;
    QReadWriteLock lock_clientNum;

    std::vector<clientproperty> clientsproperty;
    QReadWriteLock lock_clientsproperty;

//    std::vector<QString> messagelist;
//    QReadWriteLock lock_messagelist;

    //修改成2个messagelist，方便后期写文件
    //begin
    std::vector<QString> *currentmessagelist;
    QReadWriteLock *lock_currentmessagelist;

    std::vector<QString> *backupmessagelist;
    QReadWriteLock *lock_backupmessagelist;

    std::vector<QString> messagelist1;
    QReadWriteLock lock_messagelist1;

    std::vector<QString> messagelist2;
    QReadWriteLock lock_messagelist2;

    //end


    QString Creator;
    QReadWriteLock lock_Creator;

};

class MySocket:public QTcpSocket
{
    Q_OBJECT
public:
    explicit MySocket(int socketDesc, Global_Parameters *parameters=0, QObject *parent=0);

public slots:
    void MySocketSlot_Read();
    void MySocketSlot_disconnect();
    void MySocketSlotAnswerMyThread_sendtouser(const QString &msg);
signals:
    void MySocketSignalToMyThread_disconnected();
    void MySocketSignalToMyThread_sendtoall(const QString &msg);
protected:
    void SendToUser(const QString &msg);
    void SendToAll(const QString &msg);
    void SendUserList();
    void SendColortype();
    void SendCreatorMsg();

    void updateUserMessage(QString username);
    bool containsClient(const QString &name);
    int getUser(const QString &name);

private:
    void loginProcess(const QString &user);
    void messageProcess(const QString msg);
    void deleteProcess(const QString &delID);
    void markerProcess(const QString &markermsg);
    void delmaekerProcess(const QString &delmarkerpos);
    void dragnodeProcess(const QString &dragnodepos);
    void hmdposProcess(const QString &hmd);
    void askmessageProcess();
    void resindexProcess(const QString &msg);
//    void leaveProcess();

private:
    Global_Parameters *global_parameters;
    int socketId;
    int receivemessageindex;//for debug



};



#endif // MYSOCKET_H
