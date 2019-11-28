#include "send_file.h"

FileSocket_send::FileSocket_send(QString ip,QString port,QString anofile_path,QObject *parent)
    :QTcpSocket (parent)
{
    connect(this,SIGNAL(readyRead()),this,SLOT(readMSG()));
    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    this->connectToHost(ip,port.toInt());
    QRegExp pathRex("(.*).ano");
    if(pathRex.indexIn(anofile_path)!=-1)
    {
        anopath=pathRex.cap(1);

        QFileInfo  anofile_info(anopath+".ano");
        anoname=anofile_info.fileName();
        QRegExp anoRex("(.*).ano");
        if(anoRex.indexIn(anoname)!=-1)
            anoname=anoRex.cap(1);
    }
    sendFile(anopath+".ano",anoname+".ano");

}

void FileSocket_send::sendFile(QString filepath, QString filename)
{
    QFile f(filepath);
    f.open(QIODevice::ReadOnly);
    QByteArray data=f.readAll();
    QByteArray block;

    QDataStream dts(&block,QIODevice::WriteOnly);
    dts.setVersion(QDataStream::Qt_4_7);

    dts<<qint64(0)<<qint64(0)<<filename;
    dts.device()->seek(0);
    dts<<(qint64)(block.size()+f.size());
    dts<<(qint64)(block.size()-sizeof(qint64)*2);
    dts<<filename;
    dts<<data;

    this->write(block);
    qDebug()<<filepath;
}

void FileSocket_send::readMSG()
{
    while (this->canReadLine()) {
        QRegExp anoRex("received (.*).ano\n");
        QRegExp swcRex("received (.*).eswc\n");
        QRegExp apoRex("received (.*).apo\n");
        QString MSG=QString::fromUtf8(this->readLine());
        if(anoRex.indexIn(MSG)!=-1)
        {
            sendFile(anopath+".ano.eswc",anoname+".ano.eswc");
        }else if(swcRex.indexIn(MSG)!=-1)
        {
            sendFile(anopath+".ano.apo",anoname+".ano.apo");
        }else if(apoRex.indexIn(MSG)!=-1)
        {
            qDebug()<<"filesocket disconnect";
            this->disconnectFromHost();
        }
    }
}
