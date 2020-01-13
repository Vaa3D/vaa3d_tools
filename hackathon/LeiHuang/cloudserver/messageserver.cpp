#include "messageserver.h"
#include <QDataStream>
#include <QByteArray>
#include <QtGlobal>
#include "QTextStream"
#include "basic_c_fun/v_neuronswc.h"

const double dist_thres=0.05;

MessageServer::MessageServer(QString filename,Global_Parameters *parameters,QObject *parent)
    :global_parameters(parameters),QTcpServer (parent),filename(filename),sketchNum(0)
{
//    qDebug()<<"make a message server";

    V_NeuronSWC_list testVNL= NeuronTree__2__V_NeuronSWC_list(global_parameters->wholeNT);
    for(int i=0;i<testVNL.seg.size();i++)
    {
        NeuronTree SS;
        V_NeuronSWC seg_temp =  testVNL.seg.at(i);
        seg_temp.reverse();
        SS = V_NeuronSWC__2__NeuronTree(seg_temp);
        SS.name = "loaded_" + QString("%1").arg(i);
        if (SS.listNeuron.size()>0)
            sketchedNTList.push_back(SS);
    }
    sketchNum=sketchedNTList.size();

    connect(global_parameters->timer,SIGNAL(timeout()),this,SLOT(autoSave()));
    autoSave();
    global_parameters->timer->start(3*60*1000);
}

void MessageServer::incomingConnection(int socketDesc)
{
//    qDebug()<<"a new connection";
    MessageSocket *messagesocket=new MessageSocket(socketDesc,global_parameters);


    QThread *thread=new QThread;

    messagesocket->moveToThread(thread);
    connect(thread,SIGNAL(started()),messagesocket,SLOT(MessageSocketSlot_start()));
    connect(messagesocket,SIGNAL(MessageSocketSignalToMessageServer_disconnected()),
            this,SLOT(MessageServerSlotAnswerMessageSocket_disconnected()));

    connect(messagesocket,SIGNAL(MessageSocketSignalToMessageServer_disconnected()),
            thread,SLOT(quit()));
    connect(thread,SIGNAL(finished()),messagesocket,SLOT(deleteLater()));
    connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));

    connect(messagesocket,SIGNAL(MessageSocketSignalToMessageServer_sendtoall(const QString &)),
            this,SLOT(MessageServerSlotAnswerMessageSocket_sendtoall(const QString &)));
    connect(this,SIGNAL(MessageServerSignal_sendtoall(const QString &)),messagesocket,
            SLOT(MessageSocketSlotAnswerToMessageServer_sendtoall(const QString &)));

    connect(messagesocket,SIGNAL(signal_addseg(QString)),this,SLOT(MessageServerSlotAnswerMessageSocket_addseg(QString)));
    connect(messagesocket,SIGNAL(signal_delseg(QString)),this,SLOT(MessageServerSlotAnswerMessageSocket_delseg(QString)));
    connect(messagesocket,SIGNAL(signal_addmarker(QString)),this,SLOT(MessageServerSlotAnswerMessageSocket_addmarker(QString)));
    connect(messagesocket,SIGNAL(signal_delmarker(QString)),this,SLOT(MessageServerSlotAnswerMessageSocket_delmarker(QString)));

    thread->start();
}

void MessageServer::MessageServerSlotAnswerMessageSocket_sendtoall(const QString &msg)
{
    emit MessageServerSignal_sendtoall(msg);
}

void MessageServer::MessageServerSlotAnswerMessageSocket_disconnected()
{
//    qDebug()<<"socket disconnected ,userNUM--";

        QRegExp fileExp("(.*).ano");
        if(fileExp.indexIn(filename)!=-1)
        {
            QString tempname ="./clouddata/"+fileExp.cap(1)+".ano";
            QFile *f=new QFile(tempname);
            if(f->exists())
            {
                f->remove();
                delete f;
                f=new QFile(tempname+".eswc");
                if(f->exists())
                {
                    f->remove();
                    delete f;
                    f=new QFile(tempname+".apo");
                    if(f->exists())
                    {
                        f->remove();
                        delete f;
                    }
                }

            }
            QFile anofile(tempname);
            anofile.open(QIODevice::WriteOnly);
            QString str1="APOFILE="+fileExp.cap(1)+".ano.apo";
            QString str2="SWCFILE="+fileExp.cap(1)+".ano.eswc";

            QTextStream out(&anofile);
            out<<str1<<endl<<str2;
            anofile.close();

            {
                V_NeuronSWC_list tosave;
                tosave.clear();
                for(int i=0;i<sketchedNTList.size();i++)
                {
                    NeuronTree ss=sketchedNTList.at(i);
                    V_NeuronSWC ss_temp=NeuronTree__2__V_NeuronSWC_list(ss).seg.at(0);
                    tosave.seg.push_back(ss_temp);
                }
                global_parameters->wholeNT=V_NeuronSWC_list__2__NeuronTree(tosave);
            }
            writeESWC_file("./clouddata/"+fileExp.cap(1)+".ano.eswc",global_parameters->wholeNT);
            writeAPO_file("./clouddata/"+fileExp.cap(1)+".ano.apo",global_parameters->wholePoint);
        }
        emit MessageServerDeleted(filename);
        delete global_parameters;
        this->deleteLater();
        qDebug()<<"save successfully";
        return;
}

void MessageServer::MessageServerSlotAnswerMessageSocket_addseg(QString MSG)
{
//    qDebug()<<"MessageServerSlotAnswerMessageSocket_addseg";
    /*MSG=QString("/seg:"+user + "__" + msg)*/
    QRegExp Reg("/seg:(.*)__(.*)");
    QString seg;
    QString username;

    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1);
        qDebug()<< username;
        seg=Reg.cap(2).trimmed();
    }

    global_parameters->lock_clientsproperty.lockForRead();
    int colortype=21;
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name==username)
        {
            colortype=global_parameters->clientsproperty.at(i).colortype;
//            qDebug()<<username<<":"<<colortype;
            break;
        }
    }
    global_parameters->lock_clientsproperty.unlock();

    QStringList qsl=QString(seg).trimmed().split("_",QString::SkipEmptyParts);
    if(qsl.size()<=1) return;
    NeuronTree newTempNT;
    newTempNT.listNeuron.clear();
    newTempNT.hashNeuron.clear();
    newTempNT.name  = "sketch_"+ QString("%1").arg(sketchNum++);

    QString head=qsl[0];

    for(int i=1;i<qsl.size();i++)
    {
        NeuronSWC S_temp;
        QStringList temp=qsl[i].trimmed().split(" ");

        if(head.trimmed().split(" ").at(0)=="TeraFly")
        {
            S_temp.n=i;
            S_temp.type=temp[1].toInt();
            S_temp.x=temp[2].toFloat();
            S_temp.y=temp[3].toFloat();
            S_temp.z=temp[4].toFloat();
            S_temp.r=username.toInt()*10+1;
            if(i==1)
                S_temp.pn=-1;
            else
                S_temp.pn=i-1;
            S_temp.level=0;
            S_temp.creatmode=0;
            S_temp.timestamp=0;
            S_temp.tfresindex=0;
            if(i==qsl.size()-1)
            {
                MessageServerSlotAnswerMessageSocket_addmarker(QString("/marker:%1__%2 %3 %4")
                  .arg(username).arg(temp[2].toFloat()).arg(temp[3].toFloat()).arg(temp[4].toFloat()));
            }
            if(i==1)
            {
                MessageServerSlotAnswerMessageSocket_delmarker(QString("/del_marker:%1__%2 %3 %4")
                  .arg(username).arg(temp[2].toFloat()).arg(temp[3].toFloat()).arg(temp[4].toFloat()));
            }

        }else if(head.trimmed().split(" ").at(0)=="TeraVR")
        {
            S_temp.n=temp[0].toLongLong();
            S_temp.type=temp[1].toInt();
            S_temp.x=temp[2].toFloat();
            S_temp.y=temp[3].toFloat();
            S_temp.z=temp[4].toFloat();

            S_temp.r=username.toInt()*10+2;
            S_temp.pn=temp[6].toLongLong();
            S_temp.level=0;
            S_temp.creatmode=0;
            S_temp.timestamp=0;
            S_temp.tfresindex=0;
            if(i==qsl.size()-1)
            {
                MessageServerSlotAnswerMessageSocket_addmarker(QString("/marker:%1__%2 %3 %4")
                  .arg(username).arg(temp[2].toFloat()).arg(temp[3].toFloat()).arg(temp[4].toFloat()));
            }
            if(i==1)
            {
                MessageServerSlotAnswerMessageSocket_delmarker(QString("/del_marker:%1__%2 %3 %4")
                  .arg(username).arg(temp[2].toFloat()).arg(temp[3].toFloat()).arg(temp[4].toFloat()));
            }
        }else if(head.trimmed().split(" ").at(0)=="TeraAI")
        {
            S_temp.n=i;
            S_temp.type=18;
            S_temp.x=temp[2].toFloat();
            S_temp.y=temp[3].toFloat();
            S_temp.z=temp[4].toFloat();
            S_temp.r=username.toInt()*10+3;
            if(i==1)
                S_temp.pn=-1;
            else
                S_temp.pn=i-1;
            S_temp.level=0;
            S_temp.creatmode=0;
            S_temp.timestamp=0;
            S_temp.tfresindex=0;
        }

        newTempNT.listNeuron.append(S_temp);
        newTempNT.hashNeuron.insert(S_temp.n,newTempNT.listNeuron.size()-1);
    }

    sketchedNTList.push_back(newTempNT);
    global_parameters->messageUsedIndex++;
//    qDebug()<<"add seg end==========================================";
}

void MessageServer::MessageServerSlotAnswerMessageSocket_delseg(QString MSG)
{
//    qDebug()<<"MessageServerSlotAnswerMessageSocket_delseg\n"<<MSG;
    /*MSG=QString("/del_curve:"+user + "__" + msg)*/
    QRegExp Reg("/del_curve:(.*)__(.*)"); //msg=node 1_node 2_....
    QString delseg;
    QString username;
    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1).trimmed();
        delseg=Reg.cap(2).trimmed();
    }


    QStringList delMSGs = delseg.split("_",QString::SkipEmptyParts);

    if(delMSGs.size()<1) return;

    for(int i=0;i<delMSGs.size();i++)
    {
        QString tempNode=delMSGs.at(i);
        QStringList tempNodeList=tempNode.split(" ",QString::SkipEmptyParts);

        if(tempNodeList.size()<3) return ;
        float x=tempNodeList.at(0).toFloat();
        float y=tempNodeList.at(1).toFloat();
        float z=tempNodeList.at(2).toFloat();

        for (int j=0;j<sketchedNTList.size();j++)
        {
            NeuronTree NT=sketchedNTList.at(j);
            NeuronSWC ss=NT.listNeuron.at(NT.listNeuron.size()-2);
            NeuronSWC ss0=NT.listNeuron.at(1);

            if(sqrt(pow(ss.x-x,2)+pow(ss.y-y,2)+pow(ss.z-z,2))<=0.01||sqrt(pow(ss0.x-x,2)+pow(ss0.y-y,2)+pow(ss0.z-z,2))<=0.01)
            {

                sketchedNTList.removeAt(j);break;
            }
        }
    }
//    qDebug()<<"MessageServerSlotAnswerMessageSocket_delseg end=============";

}

void MessageServer::MessageServerSlotAnswerMessageSocket_addmarker(QString MSG)
{
//    qDebug()<<"MessageServerSlotAnswerMessageSocket_addmarker\n"<<MSG;
    /*MSG=QString("/marker:" +user+"__"+markermsg)*/
    QRegExp Reg("/marker:(.*)__(.*)");
    QString markerpos;
    QString username;
    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1);
        markerpos=Reg.cap(2);
    }
    global_parameters->lock_clientsproperty.lockForRead();

    int colortype=21;
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name==username)
        {
            colortype=global_parameters->clientsproperty.at(i).colortype;
//            qDebug()<<username<<":"<<colortype;
            break;
        }
    }
    global_parameters->lock_clientsproperty.unlock();

    QStringList markerMSGs=markerpos.trimmed().split(" ");
    if(markerMSGs.size()<3) return;
    float mx = markerMSGs.at(0).toFloat();
    float my = markerMSGs.at(1).toFloat();
    float mz = markerMSGs.at(2).toFloat();

    CellAPO marker0;
    marker0.x=mx;marker0.y=my;marker0.z=mz;

    marker0.color.r=255;
    marker0.color.g=0;
    marker0.color.b=0;

    qDebug()<<global_parameters->wholePoint.size();
    marker0.n=global_parameters->wholePoint[global_parameters->wholePoint.size()-1].n+1;//need do something

    marker0.volsize=314.159;
    marker0.orderinfo="";
    marker0.name="";
    marker0.comment="";

    for(int i=0;i<global_parameters->wholePoint.size();i++)
    {
        float dist = /*glm::*/sqrt((global_parameters->wholePoint.at(i).x-marker0.x)*(global_parameters->wholePoint.at(i).x-marker0.x)+
                               (global_parameters->wholePoint.at(i).y-marker0.y)*(global_parameters->wholePoint.at(i).y-marker0.y)+
                               (global_parameters->wholePoint.at(i).z-marker0.z)*(global_parameters->wholePoint.at(i).z-marker0.z));
//        qDebug()<<i<<" "<<global_parameters->wholePoint.at(i).x<<" "<<global_parameters->wholePoint.at(i).y<<" "<<global_parameters->wholePoint.at(i).z;
        if(dist<5)
        {

            global_parameters->wholePoint.removeAt(i);
            qDebug()<<"delete marker "<<global_parameters->wholePoint.size();
            return;
        }
    }

    global_parameters->wholePoint.push_back(marker0);
//    qDebug()<<"MessageServerSlotAnswerMessageSocket_addmarker end;";

}
//not use it
void MessageServer::MessageServerSlotAnswerMessageSocket_delmarker(QString MSG)
{
//    qDebug()<<"MessageServerSlotAnswerMessageSocket_delmarker\n"<<MSG;
    /*MSG=QString("/del_marker:" +user+" "+delmarkerpos )*/
    QRegExp Reg("/del_marker:(.*)__(.*)");
    QString delmarkerpos;
    QString username;
    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1);
        delmarkerpos=Reg.cap(2);
    }


    QStringList delmarkerPOS = delmarkerpos.trimmed().split(" ");
    if(delmarkerPOS.size()<3)
    {
            qDebug()<<"size < 4";
            return;
    }
//    QString user = delmarkerPOS.at(0);
    float mx = delmarkerPOS.at(0).toFloat();
    float my = delmarkerPOS.at(1).toFloat();
    float mz = delmarkerPOS.at(2).toFloat();

    for(int i=0;i<global_parameters->wholePoint.size();i++)
    {
//        qDebug()<<i<<" "<<global_parameters->wholePoint.at(i).x<<" "<<global_parameters->wholePoint.at(i).y<<" "<<global_parameters->wholePoint.at(i).z;
        CellAPO marker0=global_parameters->wholePoint.at(i);
        double dist=sqrt((mx-marker0.x)*(mx-marker0.x)+
                         (my-marker0.y)*(my-marker0.y)+(mz-marker0.z)*(mz-marker0.z));
//        if(global_parameters->global_scale&& dist<dist_thres/global_parameters->global_scale)
        if(dist<8.0)
        {

            global_parameters->wholePoint.removeAt(i);
//            qDebug()<<"delete marker "<<global_parameters->wholePoint.size();
            break;
        }
    }

}

//
void MessageServer::autoSave()
{
    qDebug()<<"autosave";
    QDir rDir("./");
    if(!rDir.cd("autosave"))
    {
        rDir.mkdir("autosave");
    }
    qDebug()<<"filename"<<filename;
    QRegExp fileExp("(.*).ano");
    if(fileExp.indexIn(filename)!=-1)
    {
        QDateTime time=QDateTime::currentDateTime();
        QString strtime=time.toString("yyyy_MM_dd_hh_mm_ss");

        QString tempname =fileExp.cap(1)+"_stamp_autosave_"+strtime;

        QFile anofile("./autosave/"+tempname+".ano");
        anofile.open(QIODevice::WriteOnly);
        QString str1="APOFILE="+tempname+".ano.apo";
        QString str2="SWCFILE="+tempname+".ano.eswc";

        QTextStream out(&anofile);
        out<<str1<<endl<<str2;
        anofile.close();

        {
            V_NeuronSWC_list tosave;
            tosave.clear();
            for(int i=0;i<sketchedNTList.size();i++)
            {
                NeuronTree ss=sketchedNTList.at(i);
                V_NeuronSWC ss_temp=NeuronTree__2__V_NeuronSWC_list(ss).seg.at(0);
                tosave.seg.push_back(ss_temp);
            }
            global_parameters->wholeNT=V_NeuronSWC_list__2__NeuronTree(tosave);
        }
        writeESWC_file("./autosave/"+tempname+".ano.eswc",global_parameters->wholeNT);
        writeAPO_file("./autosave/"+tempname+".ano.apo",global_parameters->wholePoint);
    }
}



