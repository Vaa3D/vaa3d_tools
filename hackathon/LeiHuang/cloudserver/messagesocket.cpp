#include "messagesocket.h"
MessageSocket::MessageSocket(int socketDesc,Global_Parameters *parameters,QObject *parent)
    :socketId(socketDesc),global_parameters(parameters),QTcpSocket (parent)
{
    nextblocksize=0;
    qDebug()<<"socket conected "<<this<<" "<<socketDesc;
}

void MessageSocket::MessageSocketSlot_Read()
{

        QString msg;
        msg.clear();
        QRegExp loginRex("^/login:(.*)$");
        QRegExp askmessageRex("^/ask:(.*)$");
        QRegExp hmdposRex("^/hmdpos:(.*)$");
        QRegExp ResIndexRex("^/ResIndex:(.*)$");

        QRegExp segmentRex("^/seg:(.*)$");
        QRegExp deleteRex("^/del_curve:(.*)$");
        QRegExp markerRex("^/marker:(.*)$");
        QRegExp delmarkerRex("^/del_marker:(.*)$");
        QRegExp retypeRex("^/retype:(.*)$");

        QRegExp scaleRex("^/scale:(.*)");
        QRegExp creatorRex("^/creator:(.*)");
        QRegExp undoRex("^/undo:(.*)");
        QDataStream in(this);
        in.setVersion(QDataStream::Qt_4_7);


        while(1)
        {
            if(nextblocksize==0)
            {
                if(this->bytesAvailable()>=sizeof (quint16))
                {
                    in>>nextblocksize;
                }else
                {
//                    qDebug()<<"bytes <quint16";
                    return;
                }
            }

            if(nextblocksize>0&&this->bytesAvailable()>=nextblocksize)
            {
                in >>msg;
                nextblocksize=0;
            }else
            {
                return ;
            }

            msg=msg.trimmed();
            if(loginRex.indexIn(msg)!=-1)
            {
                QString user=loginRex.cap(1).trimmed();
                loginProcess(user);
            }else if(askmessageRex.indexIn(msg)!=-1)
            {
                askmessageProcess();
            }else if(hmdposRex.indexIn(msg)!=-1)
            {
                QString hmd=hmdposRex.cap(1).trimmed();
                hmdposProcess(hmd);
            }else if(ResIndexRex.indexIn(msg)!=-1)
            {
                QString ResMsg=ResIndexRex.cap(1).trimmed();
                resindexProcess(ResMsg);
            }else if(segmentRex.indexIn(msg)!=-1)
            {
                qDebug()<<"receive:"<<msg;
                QString seg=segmentRex.cap(1).trimmed();
                segProcess(seg);
            }else if(deleteRex.indexIn(msg)!=-1)
            {
                qDebug()<<"receive:"<<msg;
                QString delcurvepos=deleteRex.cap(1).trimmed();
                deleteProcess(delcurvepos);
            }else if(markerRex.indexIn(msg)!=-1)
            {
                qDebug()<<"receive:"<<msg;
                QString markerpos=markerRex.cap(1).trimmed();
                markerProcess(markerpos);
            }else if(delmarkerRex.indexIn(msg)!=-1)
            {
                qDebug()<<"receive:"<<msg;
                QString delmarkerpos=delmarkerRex.cap(1).trimmed();
                delmarkerProcess(delmarkerpos);
            }else if(scaleRex.indexIn(msg)!=-1)
            {
                float scale=scaleRex.cap(1).toFloat();
                if(global_parameters->global_scale!=0)
                {
                    global_parameters->global_scale=scale;
                }
            }else if(creatorRex.indexIn(msg)!=-1)
            {
                creatorProcess(creatorRex.cap(1));
            }else if(undoRex.indexIn(msg)!=-1)
            {
                qDebug()<<msg;
                msg=undoRex.cap(1).trimmed();
                QRegExp segmentRex("^/seg:(.*)$");
                QRegExp deleteRex("^/del_curve:(.*)$");
                QRegExp markerRex("^/marker:(.*)$");
                if(segmentRex.indexIn(msg)!=-1)
                {
                    QString seg=segmentRex.cap(1).trimmed();
                    segProcess(seg,1000);
                }else if(deleteRex.indexIn(msg)!=-1)
                {
                    QString delcurvepos=deleteRex.cap(1).trimmed();

                    deleteProcess(delcurvepos,1000);
                }else if(markerRex.indexIn(msg)!=-1)
                {
                    QString markerpos=markerRex.cap(1).trimmed();

                    markerProcess(markerpos,1000);
                }
            }else if(retypeRex.indexIn(msg)!=-1)
            {
                QString MSG=retypeRex.cap(1).trimmed();
                qDebug()<<msg;
                retypeProcess(MSG);
            }

        }
}



void MessageSocket::loginProcess(const QString &name)
{
    id.clear();
    global_parameters->lock_clients.lockForWrite();
    global_parameters->clients[this]=name;
    qDebug()<<global_parameters->clients[this];
    global_parameters->lock_clients.unlock();

    global_parameters->lock_userInfo.lockForWrite();
    if(global_parameters->userInfo->value(name,-1)!=-1)
    {
        id=QString::number(global_parameters->userInfo->value(name));
    }else
    {
        int count=global_parameters->userInfo->size()+1;
        id=QString::number(count);
        global_parameters->userInfo->insert(name,count);
    }
    global_parameters->lock_userInfo.unlock();


    clientproperty client00={global_parameters->clientNum,name,21,false,true, 0};
    if(!containsClient(name))
    {
        client00.messageindex=global_parameters->Map_Ip_NumMessage[this->peerAddress().toString()];
        global_parameters->Map_Ip_NumMessage.remove(this->peerAddress().toString());
        global_parameters->lock_clientNum.lockForWrite();
        global_parameters->clientNum++;
        client00.colortype=global_parameters->clientNum+3;
        global_parameters->lock_clientNum.unlock();

        global_parameters->lock_clientsproperty.lockForWrite();
        global_parameters->clientsproperty.push_back(client00);
        global_parameters->lock_clientsproperty.unlock();
    }else {
        int i=getUser(name);
        global_parameters->lock_clientNum.lockForWrite();
        global_parameters->clientNum++;
        global_parameters->lock_clientNum.unlock();

        global_parameters->lock_clientsproperty.lockForWrite();
        global_parameters->clientsproperty[i].online=true;
        global_parameters->clientsproperty[i].messageindex=global_parameters->Map_Ip_NumMessage[this->peerAddress().toString()];
        global_parameters->Map_Ip_NumMessage.remove(this->peerAddress().toString());
        qDebug()<<global_parameters->clientsproperty[i].name<<" "<<global_parameters->clientsproperty[i].messageindex;
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
    qDebug()<<name<< " log in";

//    SendToAll(QString("/system:" + name + " joined."));
//    SendUserList();
//    SendColortype();
//    SendCreaorMsg();
}

void MessageSocket::askmessageProcess()
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    qDebug()<<"askmessageProcess: "<<user;
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
                global_parameters->clientsproperty.at(i).Creator_res = msg.toInt();
            }
        }
    }

    global_parameters->lock_clientsproperty.unlock();
}

//add seg
void MessageSocket::segProcess(const QString &msg,int undoP)
{

    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();

    global_parameters->messagelist.push_back(QString("/seg:"+    user+QString::number(undoP) + "__" + msg));
    qDebug()<<"global_parameters->messagelist "<<global_parameters->messagelist.size();
//    qDebug()<<QString("/seg:"+    QString::number(user.toInt()+undoP) + "__" + msg);
    emit signal_addseg(QString("/seg:"+id + "__" + msg));
    global_parameters->lock_messagelist.unlock();
}

//delete seg
void MessageSocket::deleteProcess(const QString &delsegpos,int undoP)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();
        qDebug()<<"global_parameters->messagelist "<<global_parameters->messagelist.size();
    global_parameters->messagelist.push_back(QString("/del_curve:" +    user+QString::number(undoP)+"__"+delsegpos ));
//    qDebug()<<QString("/del_curve:" +    QString::number(user.toInt()+undoP)+"__"+delsegpos );
    emit signal_delseg(QString("/del_curve:" +id+"__"+delsegpos ));
    global_parameters->lock_messagelist.unlock();


}

//add marker
void MessageSocket::markerProcess(const QString &markermsg,int undoP)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();
        qDebug()<<"global_parameters->messagelist "<<global_parameters->messagelist.size();
    global_parameters->messagelist.push_back(QString("/marker:" +    user+QString::number(undoP)+"__"+markermsg));
//    qDebug()<<QString("/marker:" +    QString::number(user.toInt()+undoP)+"__"+markermsg);
    emit signal_addmarker(QString("/marker:" +id+"__"+markermsg));
    global_parameters->lock_messagelist.unlock();


}

//delete marker
void MessageSocket::delmarkerProcess(const QString &delmarkerpos)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();
    global_parameters->messagelist.push_back(QString("/del_marker:" +user+"__"+delmarkerpos ));
    emit signal_delmarker(QString("/del_marker:" +id+"__"+delmarkerpos ));
    global_parameters->lock_messagelist.unlock();

    //减marker ，QString("/del_marker:" +user+" "+delmarkerpos )
}

void MessageSocket::retypeProcess(const QString &retypeMSG)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();
        qDebug()<<"global_parameters->messagelist "<<global_parameters->messagelist.size();
    global_parameters->messagelist.push_back(QString("/retype:" +user+"__"+retypeMSG ));
    emit signal_retype(QString("/retype:" +id+"__"+retypeMSG ));
    global_parameters->lock_messagelist.unlock();
}
void MessageSocket::creatorProcess(const QString msg)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    global_parameters->lock_messagelist.lockForWrite();
        qDebug()<<"global_parameters->messagelist "<<global_parameters->messagelist.size();
    global_parameters->messagelist.push_back(QString("/creator:"+user+"__"+msg));
    emit signal_addmarker(QString("/marker:" +user+"__"+msg));
    global_parameters->lock_messagelist.unlock();


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
    if(this->state()==QAbstractSocket::ConnectedState)
    {
        QByteArray block;
        QDataStream dts(&block,QIODevice::WriteOnly);
        dts.setVersion(QDataStream::Qt_4_7);

        dts<<quint16(0)<<msg;
        global_parameters->lock_clients.lockForRead();
        QString user=global_parameters->clients.value(this);
        global_parameters->lock_clients.unlock();

        qDebug()<<"send to "+user<<":"<<msg;
        dts.device()->seek(0);
        dts<<quint16(block.size()-sizeof (quint16));
        this->write(block);
        this->flush();
    }
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
    if(i==-1)
    {
        qDebug()<<"ERROR:username " <<username<<" cannot find";
        return ;
    }
    global_parameters->lock_clientsproperty.lockForRead();
    int messageindex=global_parameters->clientsproperty.at(i).messageindex;
    global_parameters->lock_clientsproperty.unlock();

    global_parameters->lock_messagelist.lockForRead();

        if(messageindex<global_parameters->messagelist.size())
        {
//            qDebug()<<"jkghkjg";
            global_parameters->lock_clientsproperty.lockForWrite();
            if(global_parameters->clientsproperty.at(i).online)
            {
                SendToUser(global_parameters->messagelist.at(messageindex));
//                qDebug()<< global_parameters->clientsproperty.at(i).name<<" messindex "<<":"<<messageindex<<" "<<global_parameters->messagelist.at(messageindex);
                global_parameters->clientsproperty[i].messageindex++;
            }
            global_parameters->lock_clientsproperty.unlock();

        }
/*else {
        if(messageindex<global_parameters->messagelist.size())
        {
            int message_send=global_parameters->messagelist.size();
            SendToUser(global_parameters->messagelist.join(","));
            global_parameters->lock_clientsproperty.lockForWrite();
            global_parameters->clientsproperty[i].messageindex=message_send;
            qDebug()<< global_parameters->clientsproperty.at(i).name<<" message_send "<<":"<<message_send;
            global_parameters->lock_clientsproperty.unlock();
        }
    }*/
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
}

void MessageSocket::MessageSocketSlot_disconnect()
{

    global_parameters->lock_clients.lockForWrite();
    QString username=global_parameters->clients.value(this);
    qDebug()<<username<<"IS DISCONNECTED";
    global_parameters->clients.remove(this);

    global_parameters->lock_clientsproperty.lockForWrite();
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name == username)
        {
            global_parameters->clientsproperty[i].online=false;
//            global_parameters->clientsproperty.erase(global_parameters->clientsproperty.begin()+i);
        }
    }
    global_parameters->lock_clientsproperty.unlock();       
    global_parameters->lock_clients.unlock();

    global_parameters->lock_clientNum.lockForWrite();
    global_parameters->clientNum--;
    if(global_parameters->clientNum==0)
    {
        global_parameters->lock_clientNum.unlock();      
        emit MessageSocketSignalToMessageServer_disconnected();
//        qDebug()<<"global_parameters->clientNum=0";
    }
    else
    {
        global_parameters->lock_clientNum.unlock();
//        SendToAll(QString("/system:"+username+" left."));
//        SendUserList();
//        qDebug()<<"global_parameters->clientNum="<<global_parameters->clientNum;
    }

}

void MessageSocket::MessageSocketSlotAnswerToMessageServer_sendtoall(const QString &msg)
{
    SendToUser(msg);
}
