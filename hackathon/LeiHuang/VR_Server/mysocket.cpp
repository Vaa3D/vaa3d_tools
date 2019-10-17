#include "mysocket.h"

MySocket::MySocket(int socketDesc, Global_Parameters *parameters, QObject *parent)
    :global_parameters(parameters),socketId(socketDesc),QTcpSocket (parent),receivemessageindex(0)
{
    qDebug()<<"make a MySocket."<<QThread::currentThreadId();
    connect(this,SIGNAL(readyRead()),this,SLOT(MySocketSlot_Read()));
    connect(this,SIGNAL(disconnected()),this,SLOT(MySocketSlot_disconnect()));

}

void MySocket::MySocketSlot_Read()
{
    QRegExp loginRex("^/login:(.*)$");
    QRegExp messageRex("^/say:(.*)$");
    QRegExp hmdposRex("^/hmdpos:(.*)$");
    QRegExp deleteRex("^/del_curve:(.*)$");
    QRegExp markerRex("^/marker:(.*)$");
    QRegExp delmarkerRex("^/del_marker:(.*)$");
    QRegExp dragnodeRex("^/drag_node:(.*)$");
    QRegExp askmessageRex("^/ask:(.*)$");
    QRegExp ResIndexRex("^/ResIndex:(.*)$");

    while (this->canReadLine()) {
        QString line=QString::fromUtf8(this->readLine()).trimmed();
//        qDebug()<<line;

        if(loginRex.indexIn(line)!=-1)
        {
            QString user=loginRex.cap(1);
//            qDebug()<<"in login";
            loginProcess(user);
        }else if (messageRex.indexIn(line)!=-1)
        {
            qDebug()<< "received NO."<<receivemessageindex<<"  :"<<line;
            receivemessageindex++;
            QString msg=messageRex.cap(1);
            qDebug()<<"in message";
            messageProcess(msg);
        }else if(deleteRex.indexIn(line)!=-1)
        {
            QString delID = deleteRex.cap(1);
//            qDebug()<<"in delete";
            deleteProcess(delID);
        }else if(markerRex.indexIn(line)!=-1)
        {
            QString markermsg=markerRex.cap(1);
//            qDebug()<<"in marker";
            markerProcess(markermsg);
        }else if(delmarkerRex.indexIn(line)!=-1)
        {
            QString delmarkerpos=delmarkerRex.cap(1);
//            qDebug()<<"in delmarker";
            delmaekerProcess(delmarkerpos);
        }else if(dragnodeRex.indexIn(line)!=-1)
        {
            QString dragnodepos = dragnodeRex.cap(1);
//            qDebug()<<"in dragnode";
            dragnodeProcess(dragnodepos);
        }else if(hmdposRex.indexIn(line)!=-1)
        {
             QString hmd = hmdposRex.cap(1);
//             qDebug()<<"in hmdpos";
             hmdposProcess(hmd);
        }else if(askmessageRex.indexIn(line)!=-1)
        {
//            qDebug()<<"in askmessage";
            askmessageProcess();
        }else if(ResIndexRex.indexIn(line)!=-1)
        {
             QString msg = ResIndexRex.cap(1);
//             qDebug()<<"in resindex";
             resindexProcess(msg);
        }else
        {
             qDebug() << "Bad message  ";
        }
    }

}
void MySocket::messageProcess(const QString msg)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

//    global_parameters->lock_messagelist.lockForWrite();
//    global_parameters->messagelist.push_back(QString(user + ":" + msg + "\n"));
//    global_parameters->lock_messagelist.unlock();

    //修改成2个messagelist，方便后期写文件
    //begin
    global_parameters->lock_currentmessagelist->lockForWrite();
    global_parameters->currentmessagelist->push_back(QString(user + ":" + msg + "\n"));
    global_parameters->lock_currentmessagelist->unlock();
    //end

    qDebug() << "User:" << user;
    qDebug() << "Message:" << msg;
}

void MySocket::deleteProcess(const QString &delID)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

//    global_parameters->lock_messagelist.lockForWrite();
//    global_parameters->messagelist.push_back(QString("/del_curve:" +user+" "+delID + "\n"));
//    global_parameters->lock_messagelist.unlock();
    //修改成2个messagelist，方便后期写文件
    //begin
    global_parameters->lock_currentmessagelist->lockForWrite();
    global_parameters->currentmessagelist->push_back(QString("/del_curve:" +user+" "+delID + "\n"));
    global_parameters->lock_currentmessagelist->unlock();
    //end

}

void MySocket::markerProcess(const QString &markermsg)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

//    global_parameters->lock_messagelist.lockForWrite();
//    global_parameters->messagelist.push_back(QString("/marker:" +user+" "+markermsg + " \n"));
//    global_parameters->lock_messagelist.unlock();

    //修改成2个messagelist，方便后期写文件
    //begin
    global_parameters->lock_currentmessagelist->lockForWrite();
    global_parameters->currentmessagelist->push_back(QString("/marker:" +user+" "+markermsg + " \n"));
    global_parameters->lock_currentmessagelist->unlock();
    //end

}
void MySocket::delmaekerProcess(const QString &delmarkerpos)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

//    global_parameters->lock_messagelist.lockForWrite();
//    global_parameters->messagelist.push_back(QString("/del_marker:" +user+" "+delmarkerpos + " \n"));
//    global_parameters->lock_messagelist.unlock();

    //修改成2个messagelist，方便后期写文件
    //begin
    global_parameters->lock_currentmessagelist->lockForWrite();
    global_parameters->currentmessagelist->push_back(QString("/del_marker:" +user+" "+delmarkerpos + " \n"));
    global_parameters->lock_currentmessagelist->unlock();
    //end
}
void MySocket::dragnodeProcess(const QString &dragnodepos)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

//    global_parameters->lock_messagelist.lockForWrite();
//    global_parameters->messagelist.push_back(QString("/drag_node:" +user+" "+dragnodepos + " \n"));
//    global_parameters->lock_messagelist.unlock();

    //修改成2个messagelist，方便后期写文件
    //begin
    global_parameters->lock_currentmessagelist->lockForWrite();
    global_parameters->currentmessagelist->push_back(QString("/drag_node:" +user+" "+dragnodepos + " \n"));
    global_parameters->lock_currentmessagelist->unlock();
    //end
}
void MySocket::hmdposProcess(const QString &hmd)
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();

    SendToAll(QString("/hmdpos:" +user+" "+hmd + " \n"));
}
void MySocket::askmessageProcess()
{
    global_parameters->lock_clients.lockForRead();
    QString user=global_parameters->clients.value(this);
    global_parameters->lock_clients.unlock();
    updateUserMessage(user);

}
void MySocket::resindexProcess(const QString &msg)
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
                qDebug()<<"msg"<<msg;
                qDebug()<<"msg.toInt()"<<msg.toInt();
                global_parameters->clientsproperty.at(i).Creator_res = msg.toInt();
                qDebug()<<"update creator_res = "<<global_parameters->clientsproperty.at(i).Creator_res;
            }
        }
    }

    global_parameters->lock_clientsproperty.unlock();
}

void MySocket::loginProcess(const QString &user)
{
    global_parameters->lock_clients.lockForWrite();
    global_parameters->clients[this]=user;
    global_parameters->lock_clients.unlock();

    qDebug() << user << "logged in.";

    global_parameters->lock_clientNum.lockForRead();
    clientproperty client00={global_parameters->clientNum,user,21,false,true, 0};
    global_parameters->lock_clientNum.unlock();

    if(!containsClient(user))
    {
         global_parameters->lock_clientNum.lockForWrite();
         global_parameters->clientNum++;
         client00.colortype=global_parameters->clientNum+2;
         global_parameters->lock_clientNum.unlock();

         global_parameters->lock_clientsproperty.lockForWrite();
         global_parameters->clientsproperty.push_back(client00);
         global_parameters->lock_clientsproperty.unlock();
         qDebug()<<"this client's colortype is  " <<client00.colortype;
    }
    else {
        int i=getUser(user);
        global_parameters->lock_clientsproperty.lockForWrite();
        global_parameters->clientsproperty[i].online=true;
        global_parameters->lock_clientsproperty.unlock();
    }

     int onlineusernum = 0;

     global_parameters->lock_clientsproperty.lockForRead();
     for(int i= 0;i<global_parameters->clientsproperty.size();i++)// search online user num
     {
         if(global_parameters->clientsproperty[i].online)
             onlineusernum++;
     }
     global_parameters->lock_clientsproperty.unlock();

     int i=getUser(user);
     global_parameters->lock_clientsproperty.lockForWrite();
     if(global_parameters->clientsproperty.size() == 1)//do not update user name
     {
         global_parameters->clientsproperty[i].Creator = true;
         qDebug()<<"current creator is "<<user;
         global_parameters->lock_Creator.lockForWrite();
         global_parameters->Creator = user;
         global_parameters->lock_Creator.unlock();
     }
     global_parameters->lock_clientsproperty.unlock();
     SendToAll(QString("/system:" + user + " joined.\n"));
     SendUserList();

     SendColortype();
     SendCreatorMsg();
}

void MySocket::MySocketSlot_disconnect()
{
    qDebug()<<"client disconnected "<<this->peerAddress().toString();

    global_parameters->lock_clients.lockForWrite();
    QString username=global_parameters->clients.value(this);
    global_parameters->clients.remove(this);
    global_parameters->lock_clients.unlock();

    SendToAll(QString("/system:" + username + " left .\n"));

    global_parameters->lock_clientsproperty.lockForWrite();
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name == username)
            global_parameters->clientsproperty[i].online=false;
    }
    global_parameters->lock_clientsproperty.unlock();

    SendUserList();
    emit MySocketSignalToMyThread_disconnected();
}

void MySocket::MySocketSlotAnswerMyThread_sendtouser(const QString &msg)
{
    SendToUser(msg);
}

void MySocket::SendToUser(const QString &msg)
{

    this->write(msg.toUtf8());
    qDebug()<<"send :"<<msg;
}

void MySocket::SendToAll(const QString &msg)
{
    emit MySocketSignalToMyThread_sendtoall(msg);
}

void MySocket::SendUserList()
{
    global_parameters->lock_clients.lockForRead();
    QString line = "/users:"+ ((QStringList)global_parameters->clients.values()).join(",") + "\n";
    global_parameters->lock_clients.unlock();

    SendToAll(line);
}

void MySocket::SendColortype()
{
    global_parameters->lock_clients.lockForRead();

    foreach(MySocket* socket,global_parameters->clients.keys())
    {
        QString username=global_parameters->clients.value(socket);

        global_parameters->lock_clientsproperty.lockForRead();

        for(int i=0;i<global_parameters->clientsproperty.size();i++)
        {
            if(global_parameters->clientsproperty.at(i).name==username)
            {
                QString msg= QString::number(global_parameters->clientsproperty.at(i).colortype, 10);
                SendToAll(QString("/color:"+username+" "+msg+" \n"));
            }
        }

        global_parameters->lock_clientsproperty.unlock();
    }

    global_parameters->lock_clients.unlock();
}

void MySocket::SendCreatorMsg()
{
    global_parameters->lock_clients.lockForRead();

    foreach(MySocket* socket,global_parameters->clients.keys())
    {
        QString username=global_parameters->clients.value(socket);

        global_parameters->lock_clientsproperty.lockForRead();

        for(int i=0;i<global_parameters->clientsproperty.size();i++)
        {
            if(global_parameters->clientsproperty.at(i).name==username && global_parameters->clientsproperty.at(i).Creator)
            {
                QString msg= QString::number(global_parameters->clientsproperty.at(i).Creator_res, 10);
                SendToAll(QString("/creator:"+username+" "+msg+" \n"));
            }
        }

        global_parameters->lock_clientsproperty.unlock();
    }

    global_parameters->lock_clients.unlock();
}

void MySocket::updateUserMessage(QString username)
{
    int i=getUser(username);
    int messageindex=global_parameters->clientsproperty[i].messageindex;

//    global_parameters->lock_messagelist.lockForRead();
//    if(messageindex<global_parameters->messagelist.size())
//    {
//        if(global_parameters->clientsproperty[i].online)
//        {
//            SendToUser(global_parameters->messagelist[messageindex]);
//             global_parameters->lock_clientsproperty.lockForWrite();

//            global_parameters->clientsproperty[i].messageindex++;
//            global_parameters->lock_clientsproperty.unlock();
//            qDebug()<<"send user message success";
//        }
//        else
//            qDebug()<<"user is outline can't update messagelist";
//    }
//    global_parameters->lock_messagelist.unlock();

    global_parameters->lock_currentmessagelist->lockForRead();
    if(messageindex<global_parameters->currentmessagelist->size())
    {
        if(global_parameters->clientsproperty[i].online)
        {
            SendToUser(global_parameters->currentmessagelist->at(messageindex) );

//            qDebug()<<"send No."<<messageindex<<"   ."<<global_parameters->currentmessagelist->at(messageindex);
            global_parameters->lock_clientsproperty.lockForWrite();
            global_parameters->clientsproperty[i].messageindex++;
            global_parameters->lock_clientsproperty.unlock();

            qDebug()<<"send user message success";
        }
        else
            qDebug()<<"user is outline can't update messagelist";
    }
    global_parameters->lock_currentmessagelist->unlock();
}

bool MySocket::containsClient(const QString &name)
{
    bool res = false;
    global_parameters->lock_clientsproperty.lockForRead();

    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name==name)
        {
            res= true;
        }
    }
    global_parameters->lock_clientsproperty.unlock();
    return  res;
}

int MySocket::getUser(const QString &name)
{
    int res=-1;
    global_parameters->lock_clientsproperty.lockForRead();

    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name==name)
        {
            res= i;
        }
    }
    global_parameters->lock_clientsproperty.unlock();
    return  res;
}
































