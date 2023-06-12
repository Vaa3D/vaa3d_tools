#ifndef OUTSOURCE_H
#define OUTSOURCE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "SpLog.h"


struct Delivery{
    QString filepath;
    QString name;
    QString IP;
    QString Port;
    QString quality;

    unsigned char* imgdata;
    int datalen;
    int xdim;
    int ydim;
    int zdim;
    int cdim;

    QString method;

    Delivery(){
        filepath=name=method=quality="";
        imgdata=0;
        datalen=xdim=ydim=zdim=cdim=0;
        IP="127.0.0.1";
        Port="6869";
    }
};

class PluginPipeLine;

class OutSource:public QObject{
    Q_OBJECT
public:
    OutSource(PluginPipeLine *pp);
    virtual ~OutSource();

    virtual void asyncPostRequest(QJsonObject &body);
    void AssignPort(QString PORT);
    void AssignIP(QString IP);
    void sendDelivery(Delivery delivery);
signals:
    void CompletedPath(QString);
    void nextOperation(QString,int,PluginPipeLine*);
    void startpoint(int,int,int,PluginPipeLine*);
public slots:
    virtual void ReplyFinished(QNetworkReply* reply);

private:

    QString geturl();

    QString SERVER_IP ;
    QString port;
    QNetworkAccessManager *manager = nullptr;
    QNetworkRequest request;
    PluginPipeLine *pp;
    static bool first;
};

#endif // OUTSOURCE_H
