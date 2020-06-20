#include "messageserver.h"
#include <QDataStream>
#include <QByteArray>
#include <QtGlobal>
#include "QTextStream"
#include "basic_c_fun/v_neuronswc.h"
#include <QDir>
#include <QTextStream>
#include <QString>
#include <QMap>
#include <iostream>

const double dist_thres=0.05;

void MessageServer::userLoad(ForAUTOSave foraotosave)
{
    QString ip=foraotosave.ip;
    FileServer_send  *fileserver_send=foraotosave.fileserver_send;
    ManageSocket *managesocket=foraotosave.managesocket;
    QMap <QString ,MessageServer* > *Map_File_MessageServer=foraotosave.Map_File_MessageServer;
    QString messageport=foraotosave.messageport;
    QString anofile_name=foraotosave.anofile_name;

    QMap<quint32 ,QString> map=this->autoSave();//message index->autosave filename
    this->global_parameters->Map_Ip_NumMessage[managesocket->peerAddress().toString()]=map.keys().at(0);
    qDebug()<<"success to 1";


    try {
        fileserver_send->sendFile(ip,map.values().at(0));
        qDebug()<<"success to 2";
        managesocket->write(QString(messageport+":messageport"+".\n").toUtf8());
        (*Map_File_MessageServer)[anofile_name]=this;
        qDebug()<<"success to send";
    } catch (...) {
        qDebug()<<"failed to send";
    }
}

MessageServer::MessageServer(QString filename,Global_Parameters *parameters,QObject *parent)
    :global_parameters(parameters),QTcpServer (parent),filename(filename),sketchNum(0)
{
    V_NeuronSWC_list testVNL= NeuronTree__2__V_NeuronSWC_list(global_parameters->wholeNT);
    for(int i=0;i<testVNL.seg.size();i++)
    {
        NeuronTree SS;
        V_NeuronSWC seg_temp =  testVNL.seg.at(i);
        seg_temp.reverse();
        SS = V_NeuronSWC__2__NeuronTree(seg_temp);
        SS.name = "loaded_" + QString("%1").arg(i);
        if (SS.listNeuron.size()>2)
            sketchedNTList.push_back(SS);
    }
    sketchNum=sketchedNTList.size();
//    connect(this,SIGNAL(userLoadSignal(ForAUTOSave)),this,SLOT(userLoad(ForAUTOSave)));
    connect(global_parameters->timer,SIGNAL(timeout()),this,SLOT(autoSave()));
    //autoSave();
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
    connect(messagesocket,SIGNAL(signal_retype(QString)),this,SLOT(MessageServerSlotAnswerMessageSocket_retype(QString)));

    thread->start();
}

void MessageServer::MessageServerSlotAnswerMessageSocket_retype( QString MSG)
{
    global_parameters->messageUsedIndex++;
//    qDebug()<<"messageindex="<<global_parameters->messageUsedIndex;
    orderList.push_back(QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm:ss ")+MSG);
    QRegExp Reg("/retype:(.*)__(.*)");
    QString delseg;
    QString username;
    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1).trimmed();
        delseg=Reg.cap(2).trimmed();
    }
     QStringList delMSGs = delseg.split("_",QString::SkipEmptyParts);
     qDebug()<<"delMSGs:"<<delMSGs;
     if(delMSGs.size()<1) return;

     for(int i=0;i<delMSGs.size();i++)
     {
         QString tempNode=delMSGs.at(i);
         QStringList tempNodeList=tempNode.split(" ",QString::SkipEmptyParts);

         if(tempNodeList.size()<4) return ;
         float x=tempNodeList.at(0).toFloat();
         float y=tempNodeList.at(1).toFloat();
         float z=tempNodeList.at(2).toFloat();
         int type=tempNodeList.at(3).toInt();


         for (int j=0;j<sketchedNTList.size();j++)
         {
             NeuronTree NT=sketchedNTList.at(j);
             if(NT.listNeuron.size()<=2) continue;
             NeuronSWC ss=NT.listNeuron.at(NT.listNeuron.size()-2);
             NeuronSWC ss0=NT.listNeuron.at(1);

             if(sqrt(pow(ss.x-x,2)+pow(ss.y-y,2)+pow(ss.z-z,2))<=0.01||sqrt(pow(ss0.x-x,2)+pow(ss0.y-y,2)+pow(ss0.z-z,2))<=0.01)
             {
                 sketchedNTList.removeAt(j);
                 NeuronTree newTempNT;
                 newTempNT.listNeuron.clear();
                 newTempNT.hashNeuron.clear();

                 newTempNT.name=NT.name;
                 for(int k=0;k<NT.listNeuron.size();k++)
                 {
                     NeuronSWC S_temp;
                     S_temp.n=NT.listNeuron.at(k).n;
                     S_temp.type=type;
                     S_temp.x=NT.listNeuron.at(k).x;
                     S_temp.y=NT.listNeuron.at(k).y;
                     S_temp.z=NT.listNeuron.at(k).z;
                     S_temp.r=NT.listNeuron.at(k).r;

                     S_temp.pn=NT.listNeuron.at(k).pn;
                     S_temp.level=NT.listNeuron.at(k).level;
                     S_temp.creatmode=NT.listNeuron.at(k).creatmode;
                     S_temp.timestamp=NT.listNeuron.at(k).timestamp;
                     S_temp.tfresindex=NT.listNeuron.at(k).tfresindex;
                     newTempNT.listNeuron.append(S_temp);
                     newTempNT.hashNeuron.insert(S_temp.n,newTempNT.listNeuron.size()-1);
                 }
                 sketchedNTList.push_front(newTempNT);break;

             }
         }
     }
     qDebug()<<"retype end==================================";
}

void MessageServer::MessageServerSlotAnswerMessageSocket_sendtoall(const QString &msg)
{
    emit MessageServerSignal_sendtoall(msg);
}

void MessageServer::MessageServerSlotAnswerMessageSocket_disconnected()
{
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

        //write log
        if(!QDir("./orderfile").exists()) QDir("./").mkdir("orderfile");
        QFile f("./orderfile/"+filename+".txt");

        if(f.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
        {
            QTextStream txtoutput(&f);

            while (!orderList.isEmpty()) {

                txtoutput<<orderList.at(0)<<endl;
                orderList.removeFirst();
            }
            txtoutput<<endl;
        }
        f.close();

        if(!QDir("./removelog").exists()) QDir("./").mkdir("removelog");

        V_NeuronSWC_list testVNL;
        if(QFile("./removelog/"+filename+".swc").exists())
        {
            NeuronTree remove_stroed=readSWC_file("./removelog/"+filename+".swc");
            testVNL= NeuronTree__2__V_NeuronSWC_list(remove_stroed);
        }

        while(!removedNTList.isEmpty())
        {
            NeuronTree NT=removedNTList.at(0).NT;
            V_NeuronSWC seg=NeuronTree__2__V_NeuronSWC_list(NT).seg.at(0);
            for(int j=0;j<seg.row.size();j++)
            {
                seg.row.at(j).timestamp=removedNTList.at(0).time;
                seg.row.at(j).r+=removedNTList.at(0).id*10000;
            }
            testVNL.seg.push_back(seg);
            removedNTList.removeAt(0);
        }

        NeuronTree tmp=V_NeuronSWC_list__2__NeuronTree(testVNL);
        writeESWC_file("./removelog/"+filename+".swc",tmp);

        QFile userInfoFile("./userInfo/userInfo.txt");
        if(userInfoFile.open(QIODevice::Truncate|QIODevice::Text|QIODevice::WriteOnly))
        {
            QTextStream __stream(&userInfoFile);
            global_parameters->lock_userInfo.lockForRead();
            for(quint64 i=0;i<global_parameters->userInfo->size();i++)
            {
                QString _=global_parameters->userInfo->keys().at(i)+" "+QString::number(global_parameters->userInfo->value(global_parameters->userInfo->keys().at(i)));
                __stream<<_<<endl;
            }
            global_parameters->lock_userInfo.unlock();
        }
        userInfoFile.close();
        emit MessageServerDeleted(filename);
        delete global_parameters;

        this->deleteLater();
        qDebug()<<"save successfully";
        return;
}

void MessageServer::MessageServerSlotAnswerMessageSocket_addseg(QString MSG)
{
    global_parameters->messageUsedIndex++;
//    qDebug()<<"messageindex="<<global_parameters->messageUsedIndex;
    orderList.push_back(QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm:ss ")+MSG);
    QRegExp Reg("/seg:(.*)__(.*)");
    QString seg;
    QString username;

    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1);
        qDebug()<< username;
        seg=Reg.cap(2).trimmed();
    }

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
        QStringList temp=qsl[i].trimmed().split(" ",QString::SkipEmptyParts);

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
            S_temp.timestamp=time(NULL);
            S_temp.tfresindex=0;
//            if(i==qsl.size()-1)
//            {
//                MessageServerSlotAnswerMessageSocket_addmarker(QString("/marker:%1__%2 %3 %4")
//                  .arg(username).arg(temp[2].toFloat()).arg(temp[3].toFloat()).arg(temp[4].toFloat()),1*100+temp[1].toInt());
//            }
//            if(i==1)
//            {
//                MessageServerSlotAnswerMessageSocket_delmarker(QString("/del_marker:%1__%2 %3 %4")
//                  .arg(username).arg(temp[2].toFloat()).arg(temp[3].toFloat()).arg(temp[4].toFloat()),1);
//            }

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
            S_temp.timestamp=time(NULL);
            S_temp.tfresindex=0;
//            if(i==qsl.size()-1)
//            {
//                MessageServerSlotAnswerMessageSocket_addmarker(QString("/marker:%1__%2 %3 %4")
//                  .arg(username).arg(temp[2].toFloat()).arg(temp[3].toFloat()).arg(temp[4].toFloat()),1*100+temp[1].toInt());
//            }
//            if(i==1)
//            {
//                MessageServerSlotAnswerMessageSocket_delmarker(QString("/del_marker:%1__%2 %3 %4")
//                  .arg(username).arg(temp[2].toFloat()).arg(temp[3].toFloat()).arg(temp[4].toFloat()),1);
//            }
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
            S_temp.timestamp=time(NULL);
            S_temp.tfresindex=0;
        }

        newTempNT.listNeuron.append(S_temp);
        newTempNT.hashNeuron.insert(S_temp.n,newTempNT.listNeuron.size()-1);
    }

    sketchedNTList.push_back(newTempNT);
    qDebug()<<"add seg end==========================================";
}

void MessageServer::MessageServerSlotAnswerMessageSocket_delseg(QString MSG)
{

    /*MSG=QString("/del_curve:"+user + "__" + msg)*/
    global_parameters->messageUsedIndex++;
//    qDebug()<<"messageindex="<<global_parameters->messageUsedIndex<<MSG;
    orderList.push_back(QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm:ss ")+MSG);
    QRegExp Reg("/del_curve:(.*)__(.*)"); //msg=node 1_node 2_....
    QString delseg;
    QString username;
//    qDebug()<<"messageindex=7";
    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1).trimmed();
//        qDebug()<<"messageindex=8";
        delseg=Reg.cap(2).trimmed();
    }


//    qDebug()<<"messageindex=1";
    QStringList delMSGs = delseg.split("_",QString::SkipEmptyParts);

    if(delMSGs.size()<2) return;

    for(int i=0;i<delMSGs.size();i++)
        qDebug()<<delMSGs.at(i);

    for(int i=1;i<delMSGs.size();i++)
    {
        QString tempNode=delMSGs.at(i);
        QStringList tempNodeList=tempNode.split(" ",QString::SkipEmptyParts);

//        qDebug()<<"messageindex=2";
        if(tempNodeList.size()<3) return ;
        float x=tempNodeList.at(0).toFloat();
        float y=tempNodeList.at(1).toFloat();
        float z=tempNodeList.at(2).toFloat();

//        qDebug()<<"messageindex=3";
        for (int j=0;j<sketchedNTList.size();j++)
        {

            NeuronTree NT=sketchedNTList.at(j);
            if(NT.listNeuron.size()<=2)
            {
                std::cerr<<j << "SIZE <=2"<<std::endl;
                continue;
            }

//            qDebug()<<"messageindex=3   "<<sketchedNTList.size()<<" "<<NT.listNeuron.size();
            NeuronSWC ss=NT.listNeuron.at(NT.listNeuron.size()-2);
            NeuronSWC ss0=NT.listNeuron.at(1);
//            qDebug()<<"messageindex=3   "<<sketchedNTList.size();
//            qDebug()<<"messageindex=9   "<<j;
            if(sqrt(pow(ss.x-x,2)+pow(ss.y-y,2)+pow(ss.z-z,2))<=0.01||sqrt(pow(ss0.x-x,2)+pow(ss0.y-y,2)+pow(ss0.z-z,2))<=0.01)
            {
                RemoveInfo info;
                info.NT=NT;
                info.time=time(0);
//                qDebug()<<"messageindex=3   "<<username;
                if(delMSGs[0]=="TeraFly")
                info.id=username.toInt()*10+1;
                else if(delMSGs[0]=="TeraVR")
                    info.id=username.toInt()*10+2;
//                qDebug()<<"messageindex=6";
                removedNTList.push_back(info);
                sketchedNTList.removeAt(j);
                break;
            }
//            else
//            {
//                qDebug()<<"messageindex=4";
//            }
        }
    }
//    global_parameters->messageUsedIndex++;
    qDebug()<<"MessageServerSlotAnswerMessageSocket_delseg end=============";
}

void MessageServer::MessageServerSlotAnswerMessageSocket_addmarker(QString MSG,int flag)
{
    if(!flag/100)
        global_parameters->messageUsedIndex++;
//    qDebug()<<"messageindex="<<global_parameters->messageUsedIndex;
    /*MSG=QString("/marker:" +user+"__"+markermsg)*/
    orderList.push_back(QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm:ss ")+MSG);
    QRegExp Reg("/marker:(.*)__(.*)");
    QString markerpos;
    QString username;
    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1);
        markerpos=Reg.cap(2);
    }

    int type=2;
    QStringList markerMSGs=markerpos.trimmed().split(" ",QString::SkipEmptyParts);
    if(markerMSGs.size()<3) return;
    float mx = markerMSGs.at(0).toFloat();
    float my = markerMSGs.at(1).toFloat();
    float mz = markerMSGs.at(2).toFloat();
    if(markerMSGs.size()==7)
        type=markerMSGs.at(6).toInt();
    if(flag/100==1)
        type=flag%100;

    CellAPO marker0;
    marker0.x=mx;marker0.y=my;marker0.z=mz;
//    qDebug()<<"marker0"

    marker0.color.r=0;
    marker0.color.g=0;
    marker0.color.b=255;


//    qDebug()<<global_parameters->wholePoint.size();
    if(global_parameters->wholePoint.size())
        marker0.n=global_parameters->wholePoint[global_parameters->wholePoint.size()-1].n+1;//need do something
    else
        marker0.n=1;
    marker0.volsize=314.159;
    marker0.orderinfo="";
    marker0.name="";
    marker0.comment="";

    for(int i=0;i<global_parameters->wholePoint.size();i++)
    {
        float dist = sqrt((global_parameters->wholePoint.at(i).x-marker0.x)*(global_parameters->wholePoint.at(i).x-marker0.x)+
                               (global_parameters->wholePoint.at(i).y-marker0.y)*(global_parameters->wholePoint.at(i).y-marker0.y)+
                               (global_parameters->wholePoint.at(i).z-marker0.z)*(global_parameters->wholePoint.at(i).z-marker0.z));

        if(dist<1.0)
        {

            global_parameters->wholePoint.removeAt(i);
            for(int i=0;i<global_parameters->wholePoint.size();i++)
            {
                global_parameters->wholePoint[i].n=i+1;
            }
            return;
        }
    }

    const int neuron_type_color[ ][3] = {///////////////////////////////////////////////////////
            {255, 255, 255},  // white,   0-undefined
            {20,  20,  20 },  // black,   1-soma
            {200, 20,  0  },  // red,     2-axon
            {0,   20,  200},  // blue,    3-dendrite
            {200, 0,   200},  // purple,  4-apical dendrite
            //the following is Hanchuan's extended color. 090331
            {0,   200, 200},  // cyan,    5
            {220, 200, 0  },  // yellow,  6
            {0,   200, 20 },  // green,   7
            {188, 94,  37 },  // coffee,  8
            {180, 200, 120},  // asparagus,	9
            {250, 100, 120},  // salmon,	10
            {120, 200, 200},  // ice,		11
            {100, 120, 200},  // orchid,	12
        //the following is Hanchuan's further extended color. 111003
        {255, 128, 168},  //	13
        {128, 255, 168},  //	14
        {128, 168, 255},  //	15
        {168, 255, 128},  //	16
        {255, 168, 128},  //	17
        {168, 128, 255}, //	18
        {0, 0, 0}, //19 //totally black. PHC, 2012-02-15
        //the following (20-275) is used for matlab heat map. 120209 by WYN
        {0,0,131}, //20

            };
    marker0.color.r=neuron_type_color[type][0];
    marker0.color.g=neuron_type_color[type][1];
    marker0.color.b=neuron_type_color[type][2];
//    qDebug()<<"AddMarkerFunction: AddMarker("<<marker0.x<<","<<marker0.y<<","<<marker0.z<<","<<marker0.color.r<<","<<marker0.color.g<<","<<marker0.color.b<<")"<<endl;
    global_parameters->wholePoint.push_back(marker0);

//    qDebug()<<"still "<<global_parameters->wholePoint.size()<<" marker(s)";
    qDebug()<<"MessageServerSlotAnswerMessageSocket_addmarker end;";
//    global_parameters->messageUsedIndex++;
}

void MessageServer::MessageServerSlotAnswerMessageSocket_delmarker(QString MSG,bool flag)
{
    if(!flag)
    global_parameters->messageUsedIndex++;
//    qDebug()<<"messageindex="<<global_parameters->messageUsedIndex;
    orderList.push_back(QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm:ss ")+MSG);
    QRegExp Reg("/del_marker:(.*)__(.*)");
    QString delmarkerpos;
    QString username;
    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1);
        delmarkerpos=Reg.cap(2);
    }
    QStringList delmarkerPOS = delmarkerpos.trimmed().split(" ",QString::SkipEmptyParts);
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
        CellAPO marker0=global_parameters->wholePoint.at(i);
        double dist=sqrt((mx-marker0.x)*(mx-marker0.x)+
                         (my-marker0.y)*(my-marker0.y)+(mz-marker0.z)*(mz-marker0.z));
        if(dist<8.0)
        {
            global_parameters->wholePoint.removeAt(i);
            for(int i=0;i<global_parameters->wholePoint.size();i++)
            {
                global_parameters->wholePoint[i].n=i+1;
            }
            break;
        }

    }
}

//
QMap<quint32 ,QString> MessageServer::autoSave()
{
    quint32 cnt=global_parameters->messageUsedIndex;
    QMap<quint32 ,QString> map;
    QDir rDir("./");
    if(!rDir.cd("autosave"))
    {
        rDir.mkdir("autosave");
    }
    QRegExp fileExp("(.*).ano");
    if(fileExp.indexIn(filename)!=-1)
    {
        QDateTime time=QDateTime::currentDateTime();
        QString strtime=time.toString("yyyy_MM_dd_hh_mm_ss");

        QString filebaseName=fileExp.cap(1);
        qDebug()<<"makeMessageServer:1";
        //save ano apo eswc
        {
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

            QString tempname =filebaseName+"_stamp_autosave_"+strtime+"_"+QString::number(cnt);
            QFile anofile("./autosave/"+tempname+".ano");
            anofile.open(QIODevice::WriteOnly);
            QString str1="APOFILE="+tempname+".ano.apo";
            QString str2="SWCFILE="+tempname+".ano.eswc";

            QTextStream out(&anofile);
            out<<str1<<endl<<str2;
            anofile.close();
            writeESWC_file(QString("./autosave/%1.ano.eswc").arg(tempname),global_parameters->wholeNT);
            writeAPO_file(QString("./autosave/%1.ano.apo").arg(tempname),global_parameters->wholePoint);

            map[cnt]=tempname+".ano";
            qDebug()<<"map[cnt]"<<map[cnt];
            {
                QString tempname ="./clouddata/"+filebaseName+".ano";
                QFile anofile(tempname);
                anofile.open(QIODevice::WriteOnly|QIODevice::Truncate);
                QString str1="APOFILE="+filebaseName+".ano.apo";
                QString str2="SWCFILE="+filebaseName+".ano.eswc";
                QTextStream out(&anofile);
                out<<str1<<endl<<str2;
                anofile.close();
//                qDebug()<<
                writeESWC_file(tempname+".eswc",global_parameters->wholeNT);
//                qDebug()<<"dasdsa";
                writeAPO_file(tempname+".apo",global_parameters->wholePoint);
            }
           qDebug()<<"write swc ano apo success";
        }
    }else
    {
        std::cerr<<"error :cannot find the finename"<<filename.toStdString();
    }

    //write log
    if(!QDir("./orderfile").exists()) QDir("./").mkdir("orderfile");
    QFile f("./orderfile/"+filename+".txt");
    if(f.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QTextStream txtoutput(&f);
        while (!orderList.isEmpty()) {
            txtoutput<<orderList.at(0)<<endl;
            orderList.removeFirst();
        }
    }
    qDebug()<<"write log success";
    f.close();

    //
    if(!QDir("./removelog").exists()) QDir("./").mkdir("removelog");

    V_NeuronSWC_list testVNL;
    if(QFile("./removelog/"+filename+".swc").exists())
    {
        NeuronTree remove_stroed=readSWC_file("./removelog/"+filename+".swc");
        testVNL= NeuronTree__2__V_NeuronSWC_list(remove_stroed);
    }
    qDebug()<<"read  former removelog success";
    while(removedNTList.size()!=0)
    {
        NeuronTree NT=removedNTList.at(0).NT;
        V_NeuronSWC seg=NeuronTree__2__V_NeuronSWC_list(NT).seg.at(0);
        for(int j=0;j<seg.row.size();j++)
        {
            seg.row.at(j).timestamp=removedNTList.at(0).time;
            seg.row.at(j).r+=removedNTList.at(0).id*10000;
        }
        testVNL.seg.push_back(seg);
        removedNTList.removeAt(0);
    }

    NeuronTree tmp=V_NeuronSWC_list__2__NeuronTree(testVNL);
    writeESWC_file("./removelog/"+filename+".swc",tmp);
    qDebug()<<"write removelog success";

    QFile userInfoFile("./userInfo/userInfo.txt");
    if(userInfoFile.open(QIODevice::Truncate|QIODevice::Text|QIODevice::WriteOnly))
        {
            QTextStream __stream(&userInfoFile);
            global_parameters->lock_userInfo.lockForRead();
            for(quint64 i=0;i<global_parameters->userInfo->size();i++)
            {
                QString _=global_parameters->userInfo->keys().at(i)+" "
                        +QString::number(global_parameters->userInfo->value(global_parameters->userInfo->keys().at(i)));

                __stream<<_<<endl;
            }
            global_parameters->lock_userInfo.unlock();
        }
    userInfoFile.close();
    qDebug()<<"autosave end success";
    return map;
}


