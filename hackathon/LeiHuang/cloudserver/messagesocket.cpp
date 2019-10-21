#include "messagesocket.h"
MessageSocket::MessageSocket(int socketDesc,Global_Parameters *parameters,QObject *parent)
    :socketId(socketDesc),global_parameters(parameters),QTcpSocket (parent)
{
    qDebug()<<"make a messagesocket, and don't set it's socketId ";

}

void MessageSocket::MessageSocketSlot_Read()
{

//      qDebug()<<"in MessageSocketSlot_Read";

    while(this->canReadLine())

    {
        QString msg=QString::fromUtf8(this->readLine()).trimmed();

        QRegExp loginRex("^/login:(.*)$");
        QRegExp askmessageRex("^/ask:(.*)$");
        QRegExp hmdposRex("^/hmdpos:(.*)$");
        QRegExp ResIndexRex("^/ResIndex:(.*)$");

        QRegExp segmentRex("^/seg:(.*)$");
        QRegExp deleteRex("^/del_curve:(.*)$");
        QRegExp markerRex("^/marker:(.*)$");
        QRegExp delmarkerRex("^/del_marker:(.*)$");


        if(loginRex.indexIn(msg)!=-1)
        {
            QString user=loginRex.cap(1);
            qDebug()<<user;
            loginProcess(user);
        }else if(askmessageRex.indexIn(msg)!=-1)
        {
            askmessageProcess();
        }else if(hmdposRex.indexIn(msg)!=-1)
        {
            QString hmd=hmdposRex.cap(1);
            hmdposProcess(hmd);
        }else if(ResIndexRex.indexIn(msg)!=-1)
        {
            QString ResMsg=ResIndexRex.cap(1);
            resindexProcess(ResMsg);
        }else if(segmentRex.indexIn(msg)!=-1)
        {
             qDebug()<<msg;
            QString seg=segmentRex.cap(1);
            segProcess(seg);
        }else if(deleteRex.indexIn(msg)!=-1)
        {
            QString delcurvepos=deleteRex.cap(1);
            deleteProcess(delcurvepos);
        }else if(markerRex.indexIn(msg)!=-1)
        {
            QString markerpos=markerRex.cap(1);
            markerProcess(markerpos);
        }else if(delmarkerRex.indexIn(msg)!=-1)
        {
            QString delmarkerpos=delmarkerRex.cap(1);
            delmaekerProcess(delmarkerpos);
        }
    }
}

void MessageSocket::loginProcess(const QString &name)
{
    global_parameters->lock_clients.lockForWrite();
    global_parameters->clients[this]=name;
    global_parameters->lock_clients.unlock();

    clientproperty client00={global_parameters->clientNum,name,21,false,true, 0};


    if(!containsClient(name))
    {
        global_parameters->lock_clientNum.lockForWrite();
        global_parameters->clientNum++;
        client00.colortype=global_parameters->clientNum+2;
        global_parameters->lock_clientNum.unlock();

        global_parameters->lock_clientsproperty.lockForWrite();
        global_parameters->clientsproperty.push_back(client00);
        global_parameters->lock_clientsproperty.unlock();

    }else {
        int i=getUser(name);
        global_parameters->lock_clientsproperty.lockForWrite();
        global_parameters->clientsproperty[i].online=true;
        global_parameters->lock_clientsproperty.unlock();
    }

    int onlineusernum=0;
    global_parameters->lock_clientsproperty.lockForRead();
    for(int  i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty[i].online)
            onlineusernum++;
    }
    global_parameters->lock_clientsproperty.unlock();

    int i=getUser(name);
    global_parameters->lock_clientsproperty.lockForWrite();
    if(global_parameters->clientsproperty.size()==1)
    {
        global_parameters->clientsproperty[i].Creator=true;

        global_parameters->lock_Creator.lockForWrite();
        global_parameters->Creator=name;
        global_parameters->lock_Creator.unlock();
    }
    global_parameters->lock_clientsproperty.unlock();

    SendToAll(QString("/system:" + name + " joined."));
    SendUserList();
    SendColortype();
    SendCreaorMsg();
}

void MessageSocket::askmessageProcess()
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();
    updateUserMessage(user);
}

void MessageSocket::hmdposProcess(const QString &hmd)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    SendToAll(QString("/hmdpos:" +user+" "+hmd ));
}

void MessageSocket::resindexProcess(const QString &msg)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_clientsproperty.lockForWrite();
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name==user)
        {
            if(global_parameters->clientsproperty.at(i).Creator)
            {
//                qDebug()<<"msg"<<msg;
//                qDebug()<<"msg.toInt()"<<msg.toInt();
                global_parameters->clientsproperty.at(i).Creator_res = msg.toInt();
//                qDebug()<<"update creator_res = "<<global_parameters->clientsproperty.at(i).Creator_res;
            }
        }
    }

    global_parameters->lock_clientsproperty.unlock();
}


void MessageSocket::segProcess(const QString &msg)
{
    qDebug()<<"msg :"<<msg;
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();
    global_parameters->messagelist.push_back(QString("/seg:"+user + " " + msg));
    emit signal_addseg(QString("/seg:"+user + " " + msg));
    global_parameters->lock_messagelist.unlock();

    //修改NeuronTreeList 参数QString(user + ":" + msg)
}

void MessageSocket::deleteProcess(const QString &delsegpos)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();
    global_parameters->messagelist.push_back(QString("/del_curve:" +user+" "+delsegpos ));
    emit signal_delseg(QString("/del_curve:" +user+" "+delsegpos ));
    global_parameters->lock_messagelist.unlock();

    //修改NeuronTreeList 参数QString("/del_curve:" +user+" "+delID )
}

void MessageSocket::markerProcess(const QString &markermsg)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();
    global_parameters->messagelist.push_back(QString("/marker:" +user+" "+markermsg));
    emit signal_addmarker(QString("/marker:" +user+" "+markermsg));
    global_parameters->lock_messagelist.unlock();

    //加Marker ,QString("/marker:" +user+" "+markermsg)
}

void MessageSocket::delmaekerProcess(const QString &delmarkerpos)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();
    global_parameters->messagelist.push_back(QString("/del_marker:" +user+" "+delmarkerpos ));
    emit signal_delmarker(QString("/del_marker:" +user+" "+delmarkerpos ));
    global_parameters->lock_messagelist.unlock();

    //减marker ，QString("/del_marker:" +user+" "+delmarkerpos )
}

//void MessageSocket::dragnodeProcess(const QString &dragnodepos)
//{
//    global_parameters->lock_clients.lockForRead();
//    QString user=global_parameters->clients.value(this);
//    global_parameters->lock_clients.unlock();

//    global_parameters->lock_messagelist.lockForWrite();
//    global_parameters->messagelist.push_back(QString("/drag_node:" +user+" "+dragnodepos));
//    global_parameters->lock_messagelist.unlock();

//    //移动MARKER ,QString("/drag_node:" +user+" "+dragnodepos)
//}
void MessageSocket::SendToUser(const QString &msg)
{
//    qDebug()<<msg;
//    QByteArray block;
//    QDataStream out(&block, QIODevice::WriteOnly);
//    out.setVersion(QDataStream::Qt_4_7);

//    out<<quint64(0)<<msg;
//    out.device()->seek(0);
//    out<<quint64(sizeof (block)-sizeof (quint64))<<msg;

    this->write(QString(msg+"\n").toUtf8());
}

void MessageSocket::SendToAll(const QString &msg)
{
    emit MessageSocketSignalToMessageServer_sendtoall(msg);
}

void MessageSocket::SendUserList()
{
    global_parameters->lock_clients.lockForRead();
    QString line="/users:"+ ((QStringList)global_parameters->clients.values()).join(",");
    global_parameters->lock_clients.unlock();
    SendToAll(line);
}

void MessageSocket::SendColortype()
{
    global_parameters->lock_clients.lockForRead();

    foreach(MessageSocket *socket,global_parameters->clients.keys())
    {
        QString username=global_parameters->clients.value(socket);

        global_parameters->lock_clientsproperty.lockForRead();

        for(int i=0;i<global_parameters->clientsproperty.size();i++)
        {
            if(global_parameters->clientsproperty.at(i).name==username)
            {
                QString msg=QString::number(global_parameters->clientsproperty.at(i).colortype, 10);
                SendToAll(QString("/color:"+username+" "+msg));break;
            }
        }
        global_parameters->lock_clientsproperty.unlock();
    }

    global_parameters->lock_clients.unlock();
}

void MessageSocket::SendCreaorMsg()
{
    global_parameters->lock_clients.lockForRead();

    foreach(MessageSocket *socket,global_parameters->clients.keys())
    {
        QString username=global_parameters->clients.value(socket);

        global_parameters->lock_clientsproperty.lockForRead();

        for(int i=0;i<global_parameters->clientsproperty.size();i++)
        {
            if(global_parameters->clientsproperty.at(i).name==username
                    &&global_parameters->clientsproperty.at(i).Creator)
            {
                QString msg= QString::number(global_parameters->clientsproperty.at(i).Creator_res, 10);
                SendToAll(QString("/creator:"+username+" "+msg+" "));break;
            }
        }
        global_parameters->lock_clientsproperty.unlock();
    }

    global_parameters->lock_clients.unlock();
}

void MessageSocket::updateUserMessage(QString username)
{
    int i=getUser(username);
    qDebug()<<" when update ,i ="<<i<<"+++";
    if(i==-1) return ;
    global_parameters->lock_clientsproperty.lockForRead();
    int messageindex=global_parameters->clientsproperty.at(i).messageindex;
    global_parameters->lock_clientsproperty.unlock();

    global_parameters->lock_messagelist.lockForRead();
    if(messageindex<global_parameters->messagelist.size())
    {
        global_parameters->lock_clientsproperty.lockForWrite();
        if(global_parameters->clientsproperty.at(i).online)
        {
            SendToUser(global_parameters->messagelist.at(messageindex));
            qDebug()<<"update:"<<global_parameters->messagelist.at(messageindex);
            global_parameters->clientsproperty[i].messageindex++;
        }
        global_parameters->lock_clientsproperty.unlock();
    }else {
        qDebug()<<"user is outline can't update messagelist";
    }
    global_parameters->lock_messagelist.unlock();
}

bool MessageSocket::containsClient(const QString &name)
{
    bool res = false;

    global_parameters->lock_clientsproperty.lockForRead();
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name==name)
        {
            res=true;break;
        }
    }
    global_parameters->lock_clientsproperty.unlock();
    return res;
}

int MessageSocket::getUser(const QString &name)
{
    int res=-1;
    global_parameters->lock_clientsproperty.lockForRead();
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name==name)
        {
            res=i;break;
        }
    }
    global_parameters->lock_clientsproperty.unlock();
    return res;
}

void MessageSocket::MessageSocketSlot_start()
{
    this->setSocketDescriptor(socketId);
    connect(this,SIGNAL(readyRead()),this,SLOT(MessageSocketSlot_Read()));
    connect(this,SIGNAL(disconnected()),this,SLOT(MessageSocketSlot_disconnect()));
    qDebug()<<this->peerAddress().toString()<<" ThreadId:"<<QThread::currentThreadId();

//    QRegExp tmp("(.*).ano");
//    if(tmp.indexIn(global_parameters->filename)!=-1)
//   {
//        QFile *f=new QFile("I://new/"+tmp.cap(1)+".ano");
//        f->open(QIODevice::ReadOnly);
//        QStringList tmplist;

//        while(f->canReadLine())
//        {
//            QString tmpstr=f->readLine();
//            tmplist.push_back(tmpstr);
//        }
//        f->close();
//        delete  f;
//        SendToUser("anofile:"+tmplist.join(";"));
//        tmplist.clear();

//        f=new QFile("I://new/"+tmp.cap(1)+".ano.eswc");
//        f->open(QIODevice::ReadOnly);

//        while(f->canReadLine())
//        {
//            QString tmpstr=f->readLine();
//            tmplist.push_back(tmpstr);
//        }
//        f->close();
//        delete  f;
//        SendToUser("eswcfile:"+tmplist.join(";"));
//        tmplist.clear();

//        f=new QFile("I://new/"+tmp.cap(1)+".ano.apo");
//        f->open(QIODevice::ReadOnly);

//        while(f->canReadLine())
//        {
//            QString tmpstr=f->readLine();
//            tmplist.push_back(tmpstr);
//        }
//        f->close();
//        delete  f;
//        SendToUser("apofile:"+tmplist.join(";"));
//        tmplist.clear();

//    }
}

void MessageSocket::MessageSocketSlot_disconnect()
{
    global_parameters->lock_clients.lockForWrite();
    QString username=global_parameters->clients.value(this);
    global_parameters->clients.remove(this);

    global_parameters->lock_clientsproperty.lockForWrite();
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name == username)
            global_parameters->clientsproperty[i].online=false;
    }
    global_parameters->lock_clientsproperty.unlock();

    global_parameters->lock_clients.unlock();

    SendToAll(QString("/system:"+username+" left."));
    SendUserList();


    emit MessageSocketSignalToMessageServer_disconnected();
}

void MessageSocket::MessageSocketSlotAnswerToMessageServer_sendtoall(const QString &msg)
{
    SendToUser(msg);
}
