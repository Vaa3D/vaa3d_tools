#include "manage.h"
#include <QtGlobal>
#include "basic_c_fun/basic_surf_objs.h"
FileServer *fileserver=0;
ManageServer::ManageServer(QObject *parent)
    :QTcpServer (parent)
{
    if(listen(QHostAddress::Any,9999))
    {
        qDebug()<<"ManageServer is started.";
    }else {
        qDebug()<<"ManageServer is not started ,please try again.";
    }
}

void ManageServer::incomingConnection(int socketDesc)
{
    ManageSocket *managesocket=new ManageSocket;
    managesocket->setSocketDescriptor(socketDesc);
    qDebug()<<managesocket->peerAddress().toString()<<" connected ----";
    connect(managesocket,SIGNAL(makeMessageServer(ManageSocket *,QString)),
            this,SLOT(makeMessageServer(ManageSocket *,QString)));


}

void ManageServer::makeMessageServer(ManageSocket *managesocket,QString anofile_name)
{
    //
//    qDebug()<<"in make message server";
    QRegExp fileExp("(.*).ano");
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


//            qDebug()<<messageport<<"_____here make messageport__________";
            Global_Parameters *global_parameters=new Global_Parameters;

            global_parameters->clients.clear();
            global_parameters->clientNum=0;
            global_parameters->clientsproperty.clear();
            global_parameters->messagelist.clear();
            global_parameters->Creator.clear();
            global_parameters->wholeNT=readSWC_file("./clouddata/"+fileExp.cap(1)+".ano.eswc");
            global_parameters->wholePoint=readAPO_file("./clouddata/"+fileExp.cap(1)+".ano.apo");

//            qDebug()<<"haunglei ----------------------------";
//            global_parameters->NeuronList.clear();
            /*-----------------问李琦------------------------*/
//            global_parameters->sketchNum=global_parameters->wholeNT.listNeuron.size();//需要修改
            /*-----------------问李琦------------------------*/
            global_parameters->filename=anofile_name;
            global_parameters->timer= new QTimer;
            global_parameters->messageUsedIndex=0;
            global_parameters->global_scale=0;

//            qDebug()<<"inital success.-----------------------";
            MessageServer *messageserver=new MessageServer(anofile_name, global_parameters);
            if(!messageserver->listen(QHostAddress::Any,messageport.toInt()))
            {
//                qDebug()<<anofile_name<<" MessageServer is not started.";

            }else {
                 managesocket->write(QString(messageport+":messageport"+".\n").toUtf8());
//                 qDebug()<<"here 3";
                 Map_File_MessageServer[anofile_name]=messageserver;
                 connect(messageserver,SIGNAL(MessageServerDeleted(QString)),
                        this,SLOT(messageserver_ondeltete(QString)) );
            }
        }else {
            QString messageport=QString::number(Map_File_MessageServer[anofile_name]->serverPort());
            managesocket->write(QString(messageport+":messageport"+".\n").toUtf8());
        }
    }
}

void ManageServer::messageserver_ondeltete(QString filename)
{
    qDebug()<<"Map_File_MessageServer.remove(filename);";

    Map_File_MessageServer.remove(filename);
}

ManageSocket::ManageSocket(QObject *parent)
    :QTcpSocket (parent)
{
    connect(this,SIGNAL(readyRead()),this,SLOT(readManage()));
//    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    connect(this,SIGNAL(disconnected()),this,SLOT(ondisconnect()));
}

void ManageSocket::ondisconnect()
{

    qDebug()<<this->peerAddress().toString()<<"disconnected";
    this->deleteLater();
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

                fileserver=new FileServer;
                connect(fileserver,SIGNAL(fileserverdeleted()),this,SLOT(resetfileserver()));
                if(!fileserver->listen(QHostAddress::Any,port_receivefile.toInt()))
                {
                    qDebug()<<"error:cannot start fileserver.";
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
            qDebug()<<QString(currentDir()+":currentDir_load."+"\n");
            this->write(QString(currentDir()+":currentDir_load."+"\n").toUtf8());

        }else if(FileDownRex.indexIn(manageMSG)!=-1)
        {
            QString filename=FileDownRex.cap(1);
            QString anopath="./clouddata/"+filename;
            FileSocket_send *filesocket=new FileSocket_send(this->peerAddress().toString(),"9998",anopath);

        }else if(FileLoadRex.indexIn(manageMSG)!=-1)
        {

            QString filename=FileLoadRex.cap(1);
            qDebug()<<"load :"<<filename;
//            qDebug()<<"emit makeMessageServer(this,filename);";
            emit makeMessageServer(this,filename);

//            qDebug()<<"load";
            QString anopath="./clouddata/"+filename;
            FileSocket_send *filesocket=new FileSocket_send(this->peerAddress().toString(),"9998",anopath);
        }

    }
}

void ManageSocket::resetfileserver()
{
    if(fileserver!=0)
    {qDebug()<<"fileserver!=0,when deleteLater"; delete  fileserver;fileserver=0;}

//    fileserver=0
}
QString currentDir()
{
    QDir rDir("./");
    if(!rDir.cd("clouddata"))
    {
        rDir.mkdir("clouddata");
        rDir.cd("clouddata");
    }

    QDir rootDir("./clouddata");
    QFileInfoList list=rootDir.entryInfoList();

    QStringList TEMPLIST;
    TEMPLIST.clear();

    for(unsigned i=0;i<list.size();i++)
    {
        QRegExp anoRex("(.*).ano");
        QRegExp eswcRex("(.*).ano.eswc");
        QRegExp apoRex("(.*).ano.apo");
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






