#include "OutSource.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "PluginPipeline.h"

bool OutSource::first = false;
OutSource::OutSource(PluginPipeLine *parent)
{
    manager = new QNetworkAccessManager();
    SERVER_IP = "127.0.0.1";
    port="6869";
    pp=parent;
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(ReplyFinished(QNetworkReply*)));
}

OutSource::~OutSource()
{


}

void OutSource::asyncPostRequest(QJsonObject &body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setUrl(QUrl(geturl()));
    manager->post(request, dataArray);
//    qDebug()<<"666";
//    qDebug()<<geturl();
}

void OutSource::AssignPort(QString PORT)
{
    port=PORT;
}

void OutSource::AssignIP(QString IP)
{
    SERVER_IP=IP;
}

void OutSource::sendDelivery(Delivery delivery)
{
    QJsonObject dJson;
    dJson["filepath"]=delivery.filepath;
    dJson["name"]=delivery.name;
    dJson["imgdata"]=QString::fromLatin1(reinterpret_cast<const char*>(delivery.imgdata), delivery.datalen);
    dJson["datalen"]=delivery.datalen;
    dJson["xdim"]=delivery.xdim;
    dJson["ydim"]=delivery.ydim;
    dJson["zdim"]=delivery.zdim;
    dJson["cdim"]=delivery.cdim;
    dJson["method"]=delivery.method;
    dJson["quality"]=delivery.quality;
    AssignIP(delivery.IP);
    AssignPort(delivery.Port);
    asyncPostRequest(dJson);
}

void OutSource::ReplyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(status == 200) {
        QByteArray data = reply->readAll().trimmed();
        qDebug() << data;
        // pharse Json, need "loc" to download
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if(error.error == QJsonParseError::NoError) {
            QJsonObject rootObject = doc.object();
            if(rootObject.contains("startpoint")){

                QString point=rootObject.take("startpoint").toString();
                qDebug()<<point;
                int x=point.split(",")[0].toInt();
                int y=point.split(",")[1].toInt();
                int z=point.split(",")[2].toInt();
                emit startpoint(x,y,z,pp);
            }else{
                QString qres=rootObject.take("qres").toString();
                bool qok=false;
                int qresult=qres.toInt(&qok);
                QString sres=rootObject.take("sres").toString();
                bool sok=false;
                int sresult=sres.toInt(&sok);
                if(!first){
                    first=true;
                    emit nextOperation(rootObject.take("imgpath").toString(),1,pp);
                    return;
                }
                if(qresult!=sresult){
//                    emit nextOperation(rootObject.take("imgpath").toString(),qresult,pp);
//                    emit nextOperation(rootObject.take("imgpath").toString(),sresult,pp);
                    emit nextOperation(rootObject.take("imgpath").toString(),2,pp);
                }else{
//                    emit nextOperation(rootObject.take("imgpath").toString(),sresult,pp);
                    emit nextOperation(rootObject.take("imgpath").toString(),1,pp);
                }
            }

        }
    }else{
        qDebug()<<"OutSource failed, status "<<status;
    }
    reply->deleteLater();
}

QString OutSource::geturl()
{
    return "http://"+SERVER_IP+":"+port;
}
