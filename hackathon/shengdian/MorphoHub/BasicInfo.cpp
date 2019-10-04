#include "BasicInfo.h"

QList<SomaConfInfo> getSomaConf(const QString& scandstpath){
    QList<SomaConfInfo> outlist;
    outlist.clear();
    //1.scan conf path.
    QFile scanconffile(scandstpath);
    if(!scanconffile.exists())
        QMessageBox::warning(0,"File Not Found","Can't find configuration file at Input path!");
    else
    {
        //2.get all lines from the conf file.
        if (scanconffile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            bool startState=false;
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
                if(!startState)
                {
                    startState=true;
                    continue;
                }
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()==0)   continue;
                SomaConfInfo tempsdi;
                tempsdi.SomaID=qsl.at(0);
                tempsdi.SdataID=qsl.at(1);
                tempsdi.CellName=qsl.at(2);
                tempsdi.Zprojection=qsl.at(3);
                tempsdi.Xcoordinate=qsl.at(4);
                tempsdi.Ycoordinate=qsl.at(5);
                tempsdi.SomaComments=qsl.at(6);
                tempsdi.SomaState=qsl.at(7);
                tempsdi.SomaLocation=qsl.at(8);
                tempsdi.CellType=qsl.at(9);
                outlist.append(tempsdi);
            }
        }
    }
    //3.write to Qlist
    return outlist;
}
bool WriteSomalistConfToFile(const QString& confpath,QList<SomaConfInfo> &inputsdlist)
{
    if (confpath.isEmpty())
        return false;
    QFile scanconffile(confpath);
    QString confTitle ="SomaID,SdataID,CellName,Zprojection,Xcoordinate,Ycoordinate,Comments,State,SomaLocation,CellType";
    confTitle+="\n";
    if(scanconffile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //title
        //make a new conf file
        scanconffile.write(confTitle.toAscii());
        //inside
        for(int i=0;i<inputsdlist.size();i++)
        {
            SomaConfInfo tempsdi=inputsdlist.at(i);
            QString data=tempsdi.SomaID;
            data+=(","+tempsdi.SdataID);
            data+=(","+tempsdi.CellName);
            data+=(","+tempsdi.Zprojection);
            data+=(","+tempsdi.Xcoordinate);
            data+=(","+tempsdi.Ycoordinate);
            data+=(","+tempsdi.SomaComments);
            data+=(","+tempsdi.SomaState);
            data+=(","+tempsdi.SomaLocation);
            data+=(","+tempsdi.CellType);
            data+="\n";
            scanconffile.write(data.toAscii());
        }
        scanconffile.close();
        return true;
    }
    return false;
}

QList<SourceDataInfo> getSourceDatalist(const QString& scandstpath)
{
    QList<SourceDataInfo> outlist;
    outlist.clear();
    //1.scan conf path.
    QFile scanconffile(scandstpath);
    if(!scanconffile.exists())
        QMessageBox::warning(0,"File Not Found","Can't find configuration file at Input path!");
    else
    {
        //2.get all lines from the conf file.
        if (scanconffile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            bool startState=false;
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
                if(!startState)
                {
                    startState=true;
                    continue;
                }
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()==0)   continue;
                SourceDataInfo tempsdi;
                tempsdi.SdataID=qsl.at(0);
                tempsdi.SDSize=qsl.at(1);
                tempsdi.SDResolution=qsl.at(2);
                tempsdi.SDState=qsl.at(3);
                tempsdi.SDBit=qsl.at(4);
                tempsdi.SDComments=qsl.at(5);
                tempsdi.SDPriority=qsl.at(6);
                tempsdi.SDLabeledSomaNumber=qsl.at(7);
                outlist.append(tempsdi);
            }
        }
    }
    //3.write to Qlist
    return outlist;
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
            bool startState=false;
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
                if(!startState)
                {
                    startState=true;
                    continue;
                }
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
bool WriteSourceDataToFile(const QString& confpath,QList<SourceDataInfo> &inputsdlist)
{
    if (confpath.isEmpty())
        return false;
    QFile scanconffile(confpath);
    QString confTitle ="SdataID,Size,Resolution,State,Bit,Comments,Priority,LabeledSomaNumber";
    confTitle+="\n";
    if(!scanconffile.exists())
    {
        QMessageBox::warning(0,"File Not Found",QString("Can't find configuration file at %1 !").arg(confpath));
        return false;
    }
    else
    {
        if(scanconffile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            //title
            scanconffile.write(confTitle.toAscii());
            //inside
            for(int i=0;i<inputsdlist.size();i++)
            {
                SourceDataInfo tempsdi=inputsdlist.at(i);
                QString data=tempsdi.SdataID;
                data+=(","+tempsdi.SDSize);
                data+=(","+tempsdi.SDResolution);
                data+=(","+tempsdi.SDState);
                data+=(","+tempsdi.SDBit);
                data+=(","+tempsdi.SDComments);
                data+=(","+tempsdi.SDPriority);
                data+=(","+tempsdi.SDLabeledSomaNumber);
                data+="\n";
                scanconffile.write(data.toAscii());
            }
            scanconffile.close();
            return true;
        }
        return false;
    }
}
