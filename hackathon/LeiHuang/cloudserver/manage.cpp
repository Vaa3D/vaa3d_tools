#include "manage.h"
#include <QtGlobal>
#include "basic_c_fun/basic_surf_objs.h"
#include <iostream>

#define IMAGEDIR "image"
//#include "cropimage.h"
FileServer *fileserver=0;
FileServer_send *fileserver_send=0;

ManageServer::ManageServer(QObject *parent)
    :QTcpServer (parent)
{
    if(listen(QHostAddress::Any,9999))
    {
        qDebug()<<"ManageServer is started.";
    }else {
        qDebug()<<"ManageServer is not started ,please try again.";
    }

    Map_File_MessageServer.clear();
    userList.clear();
    fileserver_send=new FileServer_send(this);//make server for load and down

    if(!QDir("./userInfo").exists())
        QDir("./").mkdir("userInfo");
    if(!QDir("./v3draw").exists())
        QDir("./").mkdir("v3draw");

    QFile userInfoFile("./userInfo/userInfo.txt");
    if(userInfoFile.exists())
    {
        if(userInfoFile.open(QIODevice::ReadOnly| QIODevice::Text))
        {
            while(!userInfoFile.atEnd())
            {
                QByteArray line = userInfoFile.readLine();
                QStringList user=QString(line).split(" ",QString::SkipEmptyParts);
                if(user.size()!=2)
                {
                    qDebug()<<"ERROR user info:"<<user;
                    continue;
                }
                userList.insert(user.at(0).trimmed(),user.at(1).trimmed().toInt());
            }
        }
    }
    userInfoFile.close();

}

void ManageServer::incomingConnection(int socketDesc)
{
    ManageSocket *managesocket=new ManageSocket(this);
    managesocket->setSocketDescriptor(socketDesc);
    qDebug()<<managesocket->peerAddress().toString()<<" connected ----";
    connect(managesocket,SIGNAL(makeMessageServer(ManageSocket *,QString)),
            this,SLOT(makeMessageServer(ManageSocket *,QString)));
}

void ManageServer::makeMessageServer(ManageSocket *managesocket,QString anofile_name)
{
    QRegExp fileExp("(.*).ano");
    qDebug()<<"makeMessageServer:"<<anofile_name;
    if(fileExp.indexIn(anofile_name)!=-1)
    {
        if(!Map_File_MessageServer.contains(anofile_name))
        {
          label:  qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
            QString messageport=QString::number(qrand()%2000+5000);

            foreach(QString filename,Map_File_MessageServer.keys())
            {
                if(Map_File_MessageServer.value(filename)->serverPort()==messageport.toInt())
                    goto label;
            }
            qDebug()<<"makeMessageServer:1";
            Global_Parameters *global_parameters=new Global_Parameters;

            global_parameters->clients.clear();
            global_parameters->clientNum=0;
            global_parameters->clientsproperty.clear();
            global_parameters->messagelist.clear();
            global_parameters->Creator.clear();
            global_parameters->wholeNT=readSWC_file("./clouddata/"+fileExp.cap(1)+".ano.eswc");
            global_parameters->wholePoint=readAPO_file("./clouddata/"+fileExp.cap(1)+".ano.apo");

            global_parameters->filename=anofile_name;
            global_parameters->timer= new QTimer;
            global_parameters->messageUsedIndex=0;
            global_parameters->global_scale=0;

            global_parameters->Map_Ip_NumMessage.clear();
            global_parameters->userInfo=&userList;
            global_parameters->messageUsedIndex=0;


            MessageServer *messageserver=new MessageServer(anofile_name, global_parameters,this);
//            QThread *thread=new QThread(this);
//            messageserver->moveToThread(thread);
            //
            qDebug()<<"makeMessageServer:2";
//            connect(this,SIGNAL(userload(ForAUTOSave)),messageserver,SLOT(userLoad(ForAUTOSave)));
            if(!messageserver->listen(QHostAddress::Any,messageport.toInt()))
            {
                std::cerr<<"can not make messageserver for "<<anofile_name.toStdString()<<endl;
                messageserver->deleteLater();
//                thread->deleteLater();
                return;
            }else {

                //timeout xinhao ->槽
                ForAUTOSave forautosave;
                forautosave.ip=managesocket->peerAddress().toString();//
                forautosave.fileserver_send=fileserver_send;//
                forautosave.managesocket=managesocket;//
                forautosave.Map_File_MessageServer=&Map_File_MessageServer;//
                forautosave.anofile_name=anofile_name;//
                forautosave.messageport=messageport;//
                connect(this,SIGNAL(userload(ForAUTOSave)),messageserver,SLOT(userLoad(ForAUTOSave)));
                emit(userload(forautosave));
                disconnect(this,SIGNAL(userload(ForAUTOSave)),messageserver,SLOT(userLoad(ForAUTOSave)));

                 connect(messageserver,SIGNAL(MessageServerDeleted(QString)),
                        this,SLOT(messageserver_ondeltete(QString)) );
//                 connect(messageserver,SIGNAL(MessageServerDeleted(QString)),
//                        thread,SLOT(quiet()) );
//                 connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()) );

            }
        }else {

//            QMap<quint32 ,QString> map=Map_File_MessageServer.value(anofile_name)->autoSave();
//            Map_File_MessageServer.value(anofile_name)->global_parameters-> Map_Ip_NumMessage[managesocket->peerAddress().toString()]=map.keys().at(0);
//            fileserver_send->sendFile(managesocket->peerAddress().toString(),map.values().at(0));
            QString messageport=QString::number(Map_File_MessageServer.value(anofile_name)->serverPort());
//            managesocket->write(QString(messageport+":messageport"+".\n").toUtf8());

            qDebug()<<"makeMessageServer:4";
            ForAUTOSave forautosave;
            forautosave.ip=managesocket->peerAddress().toString();//
            forautosave.fileserver_send=fileserver_send;//
            forautosave.managesocket=managesocket;//
            forautosave.Map_File_MessageServer=&Map_File_MessageServer;//
            forautosave.anofile_name=anofile_name;//
            forautosave.messageport=messageport;//
            connect(this,SIGNAL(userload(ForAUTOSave)),Map_File_MessageServer.value(anofile_name),SLOT(userLoad(ForAUTOSave)));
            emit(userload(forautosave));
            disconnect(this,SIGNAL(userload(ForAUTOSave)),Map_File_MessageServer.value(anofile_name),SLOT(userLoad(ForAUTOSave)));

//            Map_File_MessageServer.value(anofile_name)->emitUserLoad(forautosave);
            qDebug()<<"makeMessageServer:5";
        }
    }
}

void ManageServer::messageserver_ondeltete(QString filename)
{
//    qDebug()<<"Map_File_MessageServer.remove(filename);";
    Map_File_MessageServer.remove(filename);
}

ManageSocket::ManageSocket(QObject *parent)
    :QTcpSocket (parent)
{
    connect(this,SIGNAL(readyRead()),this,SLOT(readManage()));
    connect(this,SIGNAL(disconnected()),this,SLOT(ondisconnect()));
}

void ManageSocket::ondisconnect()
{
    qDebug()<<this->peerAddress().toString()<<"disconnected";
    this->deleteLater();
//    this=0;
}

void ManageSocket::readManage()
{
    QRegExp LoginRex("(.*):login.\n");
    QRegExp LogoutRex("(.*):logout.\n");
    QRegExp ImportRex("(.*):import.\n");
    QRegExp DownRex("(.*):down.\n");
    QRegExp LoadRex("(.*):load.\n");
    QRegExp FileDownRex("(.*):choose1.\n");
    QRegExp FileLoadRex("(.*):choose2.\n");
    QRegExp ImageDownRex("(.*):choose3.\n");
    QRegExp ImgBlockRex("(.*):imgblock.\n");

    while(this->canReadLine())
    {
        QString manageMSG=QString::fromUtf8(this->readLine());
        qDebug()<<manageMSG;
        if(LoginRex.indexIn(manageMSG)!=-1)
        {
            QString username=LoginRex.cap(1);
            this->write(QString(username+":log in success."+"\n").toUtf8());
        }else if(LogoutRex.indexIn(manageMSG)!=-1)
        {
            QString username=LogoutRex.cap(1);
            this->write(QString(username+":log out success."+"\n").toUtf8());
        }else if(ImportRex.indexIn(manageMSG)!=-1)
        {

            QString username=ImportRex.cap(1);
            QString port_receivefile="9998";
            if(fileserver==0)
            {
                fileserver=new FileServer(this);
                connect(fileserver,SIGNAL(fileserverdeleted()),this,SLOT(resetfileserver()));
                if(!fileserver->listen(QHostAddress::Any,port_receivefile.toInt()))
                {
                    std::cerr<<"error:cannot start fileserver.";
                    return;
                }
            }
            this->write(QString(QString::number(fileserver->serverPort())+":import port."+"\n").toUtf8());
        }else if(DownRex.indexIn(manageMSG)!=-1)
        {
            QString username=DownRex.cap(1);
            this->write(QString(currentDir()+":currentDir_down."+"\n").toUtf8());
        }else if(LoadRex.indexIn(manageMSG)!=-1)
        {
            QString username=LoadRex.cap(1);
            this->write(QString(currentDir()+":currentDir_load."+"\n").toUtf8());
        }else if(FileDownRex.indexIn(manageMSG)!=-1)
        {
            QString filename=FileDownRex.cap(1).trimmed();
            fileserver_send->sendFile(this->peerAddress().toString(),filename);
        }else if(FileLoadRex.indexIn(manageMSG)!=-1)
        {
            QString filename=FileLoadRex.cap(1).trimmed();
            emit makeMessageServer(this,filename);
        }
    }
}

void ManageSocket::resetfileserver()
{
    if(fileserver!=0)
    {/*qDebug()<<"fileserver!=0,when deleteLater"; */delete  fileserver;fileserver=0;}
}
QString currentDir()
{
    QDir rDir("./");
    if(!rDir.cd("clouddata"))
    {
        rDir.mkdir("clouddata");
    }

    QDir rootDir("./clouddata");
    QFileInfoList list=rootDir.entryInfoList(QStringList()<<"*.ano",QDir::Files|QDir::NoDotAndDotDot);

    QStringList TEMPLIST;
    TEMPLIST.clear();

    for(unsigned i=0;i<list.size();i++)
    {
        QRegExp anoRex("(.*).ano$");
        QRegExp eswcRex("(.*).ano.eswc$");
        QRegExp apoRex("(.*).ano.apo$");
        QFileInfo tmpFileInfo=list.at(i);
        if(!tmpFileInfo.isDir())
        {
            QString fileName = tmpFileInfo.fileName();
            if(anoRex.indexIn(fileName)!=-1&&eswcRex.indexIn(fileName)==-1&&apoRex.indexIn(fileName)==-1)
                TEMPLIST.append(fileName);
        }
    }
    return TEMPLIST.join(";");
}






