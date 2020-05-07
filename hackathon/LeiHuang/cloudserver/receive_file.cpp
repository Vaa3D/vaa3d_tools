#include "receive_file.h"
FileServer::FileServer(QObject *parent):QTcpServer (parent)
{
    clientNum=0;
}

void FileServer::incomingConnection(int socketDesc)
{
    FileSocket_receive *filesocket=new FileSocket_receive(socketDesc);
//    connect(filesocket,SIGNAL(receivefile(QString)),this,SIGNAL(receivedfile(QString)));
    connect(filesocket,SIGNAL(disconnected()),this,SLOT(Socketdisconnect()));
    clientNum++;
}

void FileServer::Socketdisconnect()
{
    if(--clientNum==0)
    {
//        qDebug()<<"delete fileser in server";
//        this->deleteLater();
        emit fileserverdeleted();
    }
}

FileSocket_receive::FileSocket_receive(int socketDesc,QObject *parent)
    :socketId(socketDesc),QTcpSocket (parent)
{
    totalsize=0;
    filenamesize=0;
    m_bytesreceived=0;
    this->setSocketDescriptor(socketId);
    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    connect(this,SIGNAL(readyRead()),this,SLOT(readFile()));
}


void FileSocket_receive::readFile()
{
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_4_7);
    if(this->m_bytesreceived==0)
    {
        if(this->bytesAvailable()>=sizeof (quint64)*2)
        {
            in>>totalsize>>filenamesize;
//            qDebug()<<totalsize <<"\t"<<filenamesize;
            m_bytesreceived+=sizeof (quint64)*2;
        }
        if(this->bytesAvailable()+m_bytesreceived>=totalsize)
        {
            QDir rootDir("./");
            if(!rootDir.cd("clouddata"))
            {
                rootDir.mkdir("clouddata");
                rootDir.cd("clouddata");
            }

            QString filename;
            in>>filename;
//            qDebug()<<filename;
            QByteArray block;
            in>>block;
            QFile file("./clouddata/"+filename);
            file.open(QIODevice::WriteOnly);
            file.write(block);
            file.close();
            m_bytesreceived=0;
            this->write(QString("received "+filename+"\n").toUtf8());
//            qDebug()<<QString("received "+filename+"\n");
        }
    }else {
            if(this->bytesAvailable()+m_bytesreceived>=totalsize)
            {
                QDir rootDir("./");
                if(!rootDir.cd("clouddata"))
                {
                    rootDir.mkdir("clouddata");
                    rootDir.cd("clouddata");
                }

                QString filename;
                in>>filename;
//                qDebug()<<filename;
                QByteArray block;
                in>>block;
                QFile file("./clouddata/"+filename);
                file.open(QIODevice::WriteOnly);
                file.write(block);
                file.close();
                m_bytesreceived=0;
                this->write(QString("received "+filename+"\n").toUtf8());
//                qDebug()<<QString("received "+filename+"\n");

            }
        }
}



