#ifndef MESSAGESERVER_H
#define MESSAGESERVER_H

#include <QtNetwork>
#include <ctime>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QThread>
#include <QReadWriteLock>
//#include "basic_surf_objs.h"
#include "messagesocket.h"
//#include "basic_c_fun/neuron_format_converter.h"
#include "neuron_editing/neuron_format_converter.h"
#include <QMap>
#include <QDateTime>
#include <managesocket.h>
#include "send_file.h"
struct RemoveInfo{
    NeuronTree NT;
    int time;
    int id;
};

class MessageServer;
struct ForAUTOSave
{

    QString ip;
    FileServer_send *fileserver_send=0;
    ManageSocket *managesocket=0;
    QMap <QString ,MessageServer* > *Map_File_MessageServer=0;
    QString anofile_name;
    QString messageport;
};

class MessageServer:public QTcpServer
{
    Q_OBJECT
public:
    explicit MessageServer(QString filename,Global_Parameters *parameters=0,QObject *parent=0);
//    inline void emitUserLoad(ForAUTOSave t)
//    {
//        emit userLoadSignal(t);
//    }
signals:
//    void userLoadSignal(ForAUTOSave);
protected:
public:
        Global_Parameters *global_parameters;
private:
    void incomingConnection(int socketDesc);
    QString filename;

    QList<NeuronTree> sketchedNTList;
    QList<RemoveInfo> removedNTList;
    QStringList orderList;
    int sketchNum;



public slots:
    void MessageServerSlotAnswerMessageSocket_sendtoall(const QString &msg);
    void MessageServerSlotAnswerMessageSocket_disconnected();

    void MessageServerSlotAnswerMessageSocket_addseg(QString);
    void MessageServerSlotAnswerMessageSocket_delseg(QString);
    void MessageServerSlotAnswerMessageSocket_addmarker(QString,int flag=0);
    void MessageServerSlotAnswerMessageSocket_delmarker(QString,bool flag=0);
    void MessageServerSlotAnswerMessageSocket_retype(QString);
//    void MessageServerSlotAnswerMessageSocket_insert(QString);
//    void MessageServerSlotAnswerMessageSocket_directconnect(QString);
    QMap<quint32 ,QString> autoSave();
    void userLoad(ForAUTOSave);
signals:
    void MessageServerSignal_sendtoall(const QString &msg);
    void MessageServerDeleted(QString);

};
#endif // MESSAGESERVER_H
