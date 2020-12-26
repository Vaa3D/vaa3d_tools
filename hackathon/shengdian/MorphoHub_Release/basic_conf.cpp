#include "basic_conf.h"
mMorphometryDB convertconf2mlist(const QString& confpath)
{
    mMorphometryDB outmm;
    QList<mMorphometry> outlist;
    outlist.clear();
    //1.scan conf path.
    QFile scanconffile(confpath);
    if(!scanconffile.exists())
        QMessageBox::warning(0,"File Not Found","Can't find configuration file at Input path!");
    else
    {
        //2.get all lines from the conf file.
        if (scanconffile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            while (! scanconffile.atEnd())
            {
                char _buf[1000], *buf;
                scanconffile.readLine(_buf, sizeof(_buf));
                for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
                if (buf[0]=='\0')	continue;
                if (buf[0]=='#')
                {
                    continue;
                }
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()==0)   continue;
                mMorphometry tempsdi;
                tempsdi.id=qsl.at(0);
                tempsdi.name=qsl.at(1);
                tempsdi.comments=qsl.at(2);
                tempsdi.filename=qsl.at(3);
                outlist.append(tempsdi);
            }
        }
    }
    //3.write to Qlist
    outmm.listMorphometry=outlist;
    outmm.name="AutoTraced";
    outmm.getlistID();
    return outmm;
}

QList<QStringList> getshowlistmMorphometry(mMorphometryDB inlist)
{
    QList<QStringList> outlist;outlist.clear();
    if(!inlist.listMorphometry.size())
        return outlist;
    QList<mMorphometry> inmlist=inlist.listMorphometry;
    qDebug()<<"size of list: "<<inmlist.size();
    for(V3DLONG i=0;i<inmlist.size();i++)
    {
        mMorphometry mm=inmlist.at(i);
        QStringList out;
        mm.getShowlist();
        out=mm.showlist;
        outlist.append(out);
    }
    return outlist;
}

QList<QStringList> getshowlistpointCloud(pointCloud_morphometry_list inlist)
{
    QList<pointCloud_morphometry> inpclist=inlist.listpointCloud;
    QList<QStringList> outlist;outlist.clear();
    if(!inpclist.size())
        return outlist;
    for(V3DLONG i=0;i<inpclist.size();i++)
    {
        pointCloud_morphometry pcm=inpclist.at(i);
        QStringList out;
        pcm.getShowlist();
        out=pcm.showlist;
        outlist.append(out);
    }
    return outlist;
}

QList<QStringList> getshowlistpointCloud(QList<pointCloud_morphometry> inpclist)
{
    QList<QStringList> outlist;outlist.clear();
    if(!inpclist.size())
        return outlist;
    for(V3DLONG i=0;i<inpclist.size();i++)
    {
        pointCloud_morphometry pcm=inpclist.at(i);
        QStringList out;
        pcm.getShowlist();
        out=pcm.showlist;
        outlist.append(out);
    }
    return outlist;
}

pointCloud_morphometry_list convertApo2pclist(const QString& inapo)
{
    pointCloud_morphometry_list outpclist;
    QList<pointCloud_morphometry> listpc;listpc.clear();
    //read apo; set imageName, out to file
    if(inapo.isEmpty())
        return outpclist;
    QFile infile(inapo);
    if(!infile.exists())
        return outpclist;
    QList <CellAPO> inlist=readAPO_file(inapo);
    if(inlist.size())
    {
        for(V3DLONG i=0;i<inlist.size();i++)
        {
            CellAPO thisCell=inlist.at(i);
            pointCloud_morphometry pcm;
            pcm.id=thisCell.name;
            pcm.name=QString::number((int)thisCell.z)+"-X"+QString::number((int)thisCell.x)+"-Y"+QString::number((int)thisCell.y);
            pcm.comments=thisCell.comment;
            listpc.append(pcm);
        }
    }
    outpclist.listpointCloud=listpc;
    outpclist.getlistID();
    return outpclist;
}

QList<pointCloud_morphometry> convertApo2pc(const QString& inapo)
{
    QList<pointCloud_morphometry> listpc;listpc.clear();
    //read apo; set imageName, out to file
    if(inapo.isEmpty())
        return listpc;
    QFile infile(inapo);
    if(!infile.exists())
        return listpc;
    QList <CellAPO> inlist=readAPO_file(inapo);
    if(inlist.size())
    {
        for(V3DLONG i=0;i<inlist.size();i++)
        {
            CellAPO thisCell=inlist.at(i);
            pointCloud_morphometry pcm;
            pcm.id=thisCell.name;
            pcm.name=QString::number((int)thisCell.z)+"-X"+QString::number((int)thisCell.x)+"-Y"+QString::number((int)thisCell.y);
            pcm.comments=thisCell.comment;
            listpc.append(pcm);
        }
    }
    return listpc;
}

bool writepointCloud(const QString& inapo,const QString& outpath)
{
    QList<pointCloud_morphometry> listpc;
    //read apo; set imageName, out to file
    if(inapo.isEmpty()||outpath.isEmpty())
        return false;
    QFile infile(inapo);
    if(!infile.exists())
        return false;
    QList <CellAPO> inlist=readAPO_file(inapo);
    pointCloud_morphometry temp;
    QString confTitle="#";
    if(temp.showlist_name.size())
    {
        confTitle+=temp.showlist_name.at(0);
        for (V3DLONG i=1;i<temp.showlist_name.size();i++)
            confTitle+=(","+temp.showlist_name.at(i));
    }
    confTitle+="\n";
    if(inlist.size())
    {
        for(V3DLONG i=0;i<inlist.size();i++)
        {
            CellAPO thisCell=inlist.at(i);
            pointCloud_morphometry pcm;
            pcm.id=thisCell.name;
            pcm.name=QString::number((int)thisCell.z)+"-X"+QString::number((int)thisCell.x)+"-Y"+QString::number((int)thisCell.y);
            pcm.comments=thisCell.comment;
            listpc.append(pcm);
        }
        QFile outfile(outpath);
        if(outfile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            //title
            outfile.write(confTitle.toAscii());
            //inside
            for(V3DLONG i=0;i<listpc.size();i++)
            {
                pointCloud_morphometry pcm=listpc.at(i);
                pcm.getShowlist();
                QString data=pcm.showlist.at(0);
                for(int j=0;j<pcm.showlist.size();j++)
                    data+=(","+pcm.showlist.at(j));
                data+="\n";
                outfile.write(data.toAscii());
            }
            outfile.close();
            return true;
        }
        return false;
    }
    else
        return false;
}

QList<Annotator> getAnnotatorlist(const QString& confpath)
{
    QList<Annotator> outlist;
    outlist.clear();
    //1.scan conf path.
    QFile scanconffile(confpath);
    if(!scanconffile.exists())
        QMessageBox::warning(0,"File Not Found","Can't find configuration file at Input path!");
    else
    {
        //2.get all lines from the conf file.
        if (scanconffile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
//            bool startState=false;
            while (! scanconffile.atEnd())
            {
                char _buf[1000], *buf;
                scanconffile.readLine(_buf, sizeof(_buf));
                for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
                if (buf[0]=='\0')	continue;
                if (buf[0]=='#')
                {
                    continue;
                }
                //skip head
//                if(!startState)
//                {
//                    startState=true;
//                    continue;
//                }
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()==0)   continue;
                Annotator tempsdi;
                tempsdi.UserID=qsl.at(0);
                tempsdi.Name=qsl.at(1);
                tempsdi.workingplace=qsl.at(2);
                tempsdi.priority=(AuthorPriority)qsl.at(3).toInt();
                outlist.append(tempsdi);
            }
        }
    }
    //3.write to Qlist
    return outlist;
}

bool writeAnnotatorConfToFile(const QString& confpath,QList<Annotator> &inputsdlist)
{
    if (confpath.isEmpty())
        return false;
    QFile scanconffile(confpath);
    Annotator temp;
    QString confTitle="#";
    if(temp.mFeatures.size())
    {
        confTitle+=temp.mFeatures.at(0);
        for (V3DLONG i=1;i<temp.mFeatures.size();i++)
            confTitle+=(","+temp.mFeatures.at(i));
    }
    confTitle+="\n";
//    qDebug()<<confTitle;
    if(scanconffile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //title
        //make a new conf file
        scanconffile.write(confTitle.toAscii());
        //inside
        for(int i=0;i<inputsdlist.size();i++)
        {
            Annotator tempsdi=inputsdlist.at(i);
            QString data=tempsdi.UserID;
            data+=(","+tempsdi.Name);
            data+=(","+tempsdi.workingplace);
            data+=(","+QString::number((int)tempsdi.priority));
            data+="\n";
            scanconffile.write(data.toAscii());
        }
        scanconffile.close();
        return true;
    }
    return false;
}

QList<mImage> getImagelist(const QString& inpath)
{
    QList<mImage> outlist;
    outlist.clear();
    //1.scan conf path.
    QFile scanconffile(inpath);
    if(!scanconffile.exists())
        QMessageBox::warning(0,"File Not Found","Can't find configuration file at Input path!");
    else
    {
        //2.get all lines from the conf file.
        if (scanconffile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
//            bool startState=false;
            while (! scanconffile.atEnd())
            {
                char _buf[1000], *buf;
                scanconffile.readLine(_buf, sizeof(_buf));
                for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
                if (buf[0]=='\0')	continue;
                if (buf[0]=='#')
                {
                    continue;
                }
                //skip head
//                if(!startState)
//                {
//                    startState=true;
//                    continue;
//                }
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()==0)   continue;
                mImage tempi;
                tempi.id=qsl.at(0).toUInt();
                tempi.name=qsl.at(1);
                tempi.objectType=qsl.at(2);
                tempi.bit=qsl.at(3).toUInt();
                tempi.dirname=qsl.at(4);
                tempi.mformat=qsl.at(5);
                if(qsl.size()>=9)
                {
                    tempi.mIS.sx=qsl.at(6).toULong();
                    tempi.mIS.sy=qsl.at(7).toULong();
                    tempi.mIS.sz=qsl.at(8).toULong();
                }
                if(qsl.size()>=12)
                {
                    tempi.mIR.rx=qsl.at(9).toDouble();
                    tempi.mIR.ry=qsl.at(10).toDouble();
                    tempi.mIR.rz=qsl.at(11).toDouble();
                }
                outlist.append(tempi);
            }
        }
    }
    //3.write to Qlist
    return outlist;
}
bool writeImagelistToFile(const QString &topath, QList<mImage> &inlist)
{
    if (topath.isEmpty()||inlist.size()==0)
        return false;
    QFile scanconffile(topath);
    //get title
    mImage temp;QString confTitle="#";
    if(temp.mFeatures.size())
    {
        confTitle+=temp.mFeatures.at(0);
        for (V3DLONG i=1;i<temp.mFeatures.size();i++)
            confTitle+=(","+temp.mFeatures.at(i));
    }
    confTitle+="\n";
    if(scanconffile.exists())
        QMessageBox::warning(0,"File overwrite",QObject::tr("already exist %1 !").arg(topath));
    if(scanconffile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //title
        scanconffile.write(confTitle.toAscii());
        //inside
        for(V3DLONG i=0;i<inlist.size();i++)
        {
            mImage tempi=inlist.at(i);
            QString data=QString::number(tempi.id);
            data+=(","+tempi.name);
            data+=(","+tempi.objectType);
            data+=(","+QString::number(tempi.bit));
            data+=(","+tempi.dirname);
            data+=(","+tempi.mformat);
            if(tempi.mIS.isAvailable())
            {
                data+=(","+QString::number(tempi.mIS.sx));
                data+=(","+QString::number(tempi.mIS.sy));
                data+=(","+QString::number(tempi.mIS.sz));
            }
            else
                data+=",0,0,0";
            if(tempi.mIR.isAvailable())
            {
                data+=(","+QString::number(tempi.mIR.rx));
                data+=(","+QString::number(tempi.mIR.rx));
                data+=(","+QString::number(tempi.mIR.rx));
            }
            else
                data+=",0.0,0.0,0.0";
            data+="\n";
            scanconffile.write(data.toAscii());
        }
        scanconffile.close();
        return true;
    }
    return false;
}
