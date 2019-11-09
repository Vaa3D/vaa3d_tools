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
    qDebug()<<"make a message server";

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
    global_parameters->timer->start(5*60*1000);
}

void MessageServer::incomingConnection(int socketDesc)
{
    qDebug()<<"a new connection";
    MessageSocket *messagesocket=new MessageSocket(socketDesc,global_parameters);
//    global_parameters->lock_clients.lockForWrite();
//    qDebug()<<"incomming "<<messagesocket;
//    global_parameters->clients.insert(messagesocket->peerAddress().toString(),"");
//    global_parameters->lock_clients.unlock();

    QThread *thread=new QThread;

    messagesocket->moveToThread(thread);

    connect(thread,SIGNAL(started()),messagesocket,SLOT(MessageSocketSlot_start()));
    connect(messagesocket,SIGNAL(MessageSocketSignalToMessageServer_disconnected()),
            messagesocket,SLOT(deleteLater()));
    connect(messagesocket,SIGNAL(MessageSocketSignalToMessageServer_disconnected()),
            thread,SLOT(deleteLater()));
    connect(messagesocket,SIGNAL(MessageSocketSignalToMessageServer_disconnected()),
            this,SLOT(MessageServerSlotAnswerMessageSocket_disconnected()));
    connect(messagesocket,
            SIGNAL(MessageSocketSignalToMessageServer_sendtoall(const QString &)),
            this,SLOT(MessageServerSlotAnswerMessageSocket_sendtoall(const QString &)));
    connect(this,SIGNAL(MessageServerSignal_sendtoall(const QString &)),
            messagesocket,
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
    qDebug()<<"socket disconnected ,userNUM--";
    global_parameters->lock_clientNum.lockForWrite();
    if(--global_parameters->clientNum==0)
    {
        QRegExp fileExp("(.*)_stamp_(.*).ano");
        if(fileExp.indexIn(filename)!=-1)
        {
            //qDebug()<<"in disconnected.++++";
            QDateTime time=QDateTime::currentDateTime();
            QString strtime=time.toString("yyyy_MM_dd_hh_mm_ss");

            QString tempname =fileExp.cap(1)+"_stamp_"+strtime;
                //qDebug()<<tempname<<"here 100";

            QFile anofile("./clouddata/"+tempname+".ano");
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

//            global_parameters->lock_wholeNT.lockForWrite();
            writeESWC_file("./clouddata/"+tempname+".ano.eswc",global_parameters->wholeNT);
//            global_parameters->lock_wholeNT.unlock();
//            global_parameters->lock_wholePoint.lockForRead();
            writeAPO_file("./clouddata/"+tempname+".ano.apo",global_parameters->wholePoint);
//            global_parameters->lock_wholePoint.unlock();
        }

        global_parameters->lock_clientNum.unlock();
        emit MessageServerDeleted(filename);
        delete global_parameters;
        this->deleteLater();
        qDebug()<<"save successfully";
        return;
    }
    else
    {
        global_parameters->lock_clientNum.unlock();
        qDebug()<<"server is on";
    }
}

void MessageServer::MessageServerSlotAnswerMessageSocket_addseg(QString MSG)
{
    qDebug()<<"MessageServerSlotAnswerMessageSocket_addseg\n"<<MSG;
    /*MSG=QString("/seg:"+user + " " + msg)*/
    QRegExp Reg("/seg:(.*)__(.*)");
    QString seg;
    QString username;

    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1);
        qDebug()<< username;
        seg=Reg.cap(2).trimmed();
    }

//    qDebug()<<"MessageServerSlotAnswerMessageSocket_addseg:================\n"<<seg<<"\n===============================";
    global_parameters->lock_clientsproperty.lockForRead();
    int colortype=21;
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name==username)
        {
            colortype=global_parameters->clientsproperty.at(i).colortype;
            qDebug()<<username<<":"<<colortype;
            break;
        }
    }
    global_parameters->lock_clientsproperty.unlock();

    QStringList qsl=QString(seg).trimmed().split("_",QString::SkipEmptyParts);

    NeuronTree newTempNT;
    newTempNT.listNeuron.clear();
    newTempNT.hashNeuron.clear();
    newTempNT.name  = "sketch_"+ QString("%1").arg(sketchNum++);
    //qDebug()<<qsl[0];
    for(int i=1;i<qsl.size();i++)
    {
       // qDebug()<<qsl[i]<<endl;
        NeuronSWC S_temp;
        QStringList temp=qsl[i].trimmed().split(" ");

        if(temp.size()==11)//use message head to judge
        {
//            S_temp.n=temp[0].toLongLong();
            S_temp.n=i;
            S_temp.type=colortype;
            S_temp.x=temp[2].toFloat();
            S_temp.y=temp[3].toFloat();
            S_temp.z=temp[4].toFloat();
            S_temp.r=temp[5].toFloat();

//            S_temp.pn=temp[6].toLongLong();
            if(i==1)
                S_temp.pn=-1;
            else
                S_temp.pn=i-1;

            S_temp.level=temp[7].toFloat();
            S_temp.creatmode=temp[8].toFloat();
            S_temp.timestamp=temp[9].toFloat();
            S_temp.tfresindex=temp[10].toFloat();

        }else if(temp.size()==7)
        {
            S_temp.n=temp[0].toLongLong();
            S_temp.type=colortype;
            S_temp.x=temp[2].toFloat();
            S_temp.y=temp[3].toFloat();
            S_temp.z=temp[4].toFloat();
            S_temp.r=temp[5].toFloat();
            S_temp.pn=temp[6].toLongLong();
            S_temp.level=0;
            S_temp.creatmode=0;
            S_temp.timestamp=0;
            S_temp.tfresindex=0;
        }
//        S_temp
        newTempNT.listNeuron.append(S_temp);
        newTempNT.hashNeuron.insert(S_temp.n,newTempNT.listNeuron.size()-1);
    }
    qDebug()<<"add seg end==========================================";
    sketchedNTList.push_back(newTempNT);
    global_parameters->messageUsedIndex++;
}

void MessageServer::MessageServerSlotAnswerMessageSocket_delseg(QString MSG)
{
    qDebug()<<"MessageServerSlotAnswerMessageSocket_delseg\n"<<MSG;
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

        for (int j=0;i<sketchedNTList.size();j++)
        {
            NeuronTree NT=sketchedNTList.at(i);
            NeuronSWC ss=NT.listNeuron.at(NT.listNeuron.size()-2);
            NeuronSWC ss0=NT.listNeuron.at(1);

            if(sqrt(pow(ss.x-x,2)+pow(ss.y-y,2)+pow(ss.z-z,2))<=2.0||sqrt(pow(ss0.x-x,2)+pow(ss0.y-y,2)+pow(ss0.z-z,2))<=2.0)
            {
                sketchedNTList.removeAt(i);break;
            }
        }
    }
    qDebug()<<"MessageServerSlotAnswerMessageSocket_delseg end=============";

//    for(int)
//    if(delMSGs.size()<2)
//    {
//            qDebug()<<"size < 2";
//            return;
//    }
////    QString user = delMSGs.at(0);//why have username;
//    float dx = delMSGs.at(0).toFloat();
//    float dy = delMSGs.at(1).toFloat();
//    float dz = delMSGs.at(2).toFloat();//global or local?? ->ask liqi
//    float resx = delMSGs.at(3).toFloat();
//    float resy = delMSGs.at(4).toFloat();
//    float resz = delMSGs.at(5).toFloat();

//    QString delID="";

//    /*find nearest segment*/
//    if(sketchedNTList.size()>=1)
//    {
//        for(int i=0;i<sketchedNTList.size();i++)
//        {
//            NeuronTree nt=sketchedNTList.at(i);
//            for(int j=0;j<nt.listNeuron.size();j++)
//            {
////                NeuronSWC ss0;
////                ss0=nt.listNeuron.at(j);
////                float dist;
////                dist=sqrt((dx-ss0.x)*(dx-ss0.x)+(dy-ss0.y)*(dy-ss0.y)
////                          +(dz-ss0.z)*(dz-ss0.z));

//                NeuronSWC ss0=nt.listNeuron.at(nt.listNeuron.size()-2);
//                if(ss0.x==dx&&ss0.y==dy)

//                if(global_parameters->global_scale!=0&& dist<dist_thres/global_parameters->global_scale)
//                {
//                    delID=nt.name;
//                    goto L;
//                }
//            }
//        }
//    }

//    L:  if(delID=="")
//        {
//            qDebug()<<"cannot find segID";
//            return;
//        }
//    for(int i=0;i<sketchedNTList.size();i++)
//    {
//        QString NTname="";
//        NTname = sketchedNTList.at(i).name;
//        if(delID==NTname)
//        {
//            sketchedNTList.removeAt(i);
//            qDebug()<<"delete segment success";
//        }
//    }



}

void MessageServer::MessageServerSlotAnswerMessageSocket_addmarker(QString MSG)
{
    qDebug()<<"MessageServerSlotAnswerMessageSocket_addmarker\n"<<MSG;
    /*MSG=QString("/marker:" +user+" "+markermsg)*/
    QRegExp Reg("/marker:(.*)__(.*)");
    QString markerpos;
    QString username;
    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1);
        markerpos=Reg.cap(2);
    }
    global_parameters->lock_clientsproperty.lockForRead();
    qDebug()<<"+++++++++++_____________________+++++++++++";
    int colortype=21;
    for(int i=0;i<global_parameters->clientsproperty.size();i++)
    {
        if(global_parameters->clientsproperty.at(i).name==username)
        {
            colortype=global_parameters->clientsproperty.at(i).colortype;
            qDebug()<<username<<":"<<colortype;
            break;
        }
    }
    global_parameters->lock_clientsproperty.unlock();

    QStringList markerMSGs=markerpos.trimmed().split(" ");
    if(markerMSGs.size()<3) return;
    qDebug()<<"markerMSGS.SIZE:"<<markerMSGs.size();
    qDebug()<<markerMSGs;
    float mx = markerMSGs.at(0).toFloat();
    float my = markerMSGs.at(1).toFloat();
    float mz = markerMSGs.at(2).toFloat();
    //不判断附近的点，直接add
    CellAPO marker0;
    marker0.x=mx;marker0.y=my;marker0.z=mz;
    qDebug()<<"marker:(x,y,z)"<<marker0.x<<","<<marker0.y<<","<<marker0.z;

    marker0.color.r=255;
    marker0.color.g=0;
    marker0.color.b=0;
    qDebug()<<"________+++++++____________";
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
        if(dist<8.0)
        {
            global_parameters->wholePoint.removeAt(i);
            return;
        }
    }

    global_parameters->wholePoint.push_back(marker0);

    qDebug()<<"MessageServerSlotAnswerMessageSocket_addmarker end;";

}
void MessageServer::MessageServerSlotAnswerMessageSocket_delmarker(QString MSG)
{
    qDebug()<<"MessageServerSlotAnswerMessageSocket_delmarker\n"<<MSG;
    /*MSG=QString("/del_marker:" +user+" "+delmarkerpos )*/
    QRegExp Reg("/del_marker:(.*)__(.*)");
    QString delmarkerpos;
    QString username;
    if(Reg.indexIn(MSG)!=-1)
    {
        username=Reg.cap(1);
        delmarkerpos=Reg.cap(2);
    }

//    global_parameters->lock_clientsproperty.lockForRead();
//    int colortype=21;
//    for(int i=0;i<global_parameters->clientsproperty.size();i++)
//    {
//        if(global_parameters->clientsproperty.at(i).name==username)
//        {
//            colortype=global_parameters->clientsproperty.at(i).colortype;
//            break;
//        }
//    }
//    global_parameters->lock_clientsproperty.unlock();
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
        CellAPO marker0=global_parameters->wholePoint.at(i);
        double dist=sqrt((mx-marker0.x)*(mx-marker0.x)+
                         (my-marker0.y)*(my-marker0.y)+(mz-marker0.z)*(mz-marker0.z));
//        if(global_parameters->global_scale&& dist<dist_thres/global_parameters->global_scale)
        if(dist<8.0)
        {
            global_parameters->wholePoint.removeAt(i);
            break;
        }
    }

}


void MessageServer::autoSave()
{
    qDebug()<<"autosave";
    QDir rDir("./");
    if(!rDir.cd("autosave"))
    {
        rDir.mkdir("autosave");
    }
    QRegExp fileExp("(.*)_stamp_(.*).ano");
    if(fileExp.indexIn(filename)!=-1)
    {
        QDateTime time=QDateTime::currentDateTime();
        QString strtime=time.toString("yyyy_MM_dd_hh_mm_ss");

        QString tempname =fileExp.cap(1)+"_stamp_autosave_"+strtime;
            qDebug()<<tempname<<"here 100";

        QFile anofile("./autosave/"+tempname+".ano");
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

//        global_parameters->lock_wholeNT.lockForWrite();
        writeESWC_file("./autosave/"+tempname+".ano.eswc",global_parameters->wholeNT);
//        global_parameters->lock_wholeNT.unlock();
//        global_parameters->lock_wholePoint.lockForRead();
        writeAPO_file("./autosave/"+tempname+".ano.apo",global_parameters->wholePoint);
//        global_parameters->lock_wholePoint.unlock();
    }
//    global_parameters->timer->start(5*60*1000);
}



