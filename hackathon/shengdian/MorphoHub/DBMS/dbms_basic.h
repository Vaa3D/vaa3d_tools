#ifndef DBMS_BASIC_H
#define DBMS_BASIC_H

//#include "qstring.h"
#include <QtGui>
using namespace std;
//file or folder exists?
bool isFileExist(QString fullFilePath)
{
    QFile file(fullFilePath);
    if(file.exists())
    {
        return true;
    }
    return false;
}

//make new dir
bool makeDir(QString fullPath)
{
    QDir dir(fullPath);
    if(dir.exists())
    {
      return true;
    }
    else
    {
       bool ok = dir.mkdir(fullPath);
       return ok;
    }
}
//write basic info to conf file
bool writeBrainConf(const QString &filename)
{
    QString curFile = filename;
    if (curFile.trimmed().isEmpty()) //then open a file dialog to choose file
    {
        if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
            return false;
    }

    FILE * fp = fopen(curFile.toLatin1(), "wt");
    if (!fp)
    {
        qDebug()<<"Could not open the configuration file to save the configuration"<<endl;
        return false;
    }
    QStringList qsl;
    qsl<<"1"<<"17300"<<"converted"<<"high"<<"100"<<"299"<<"4343"<<"1.9T"<<"fullbit";
    fprintf(fp, "##id,name,state,priority,comments,x,y,z,size,bit\n");
    for(int io=0;io<qsl.size();io++)
    {
        if(io==0)
            fprintf(fp,"%s",qPrintable(qsl.at(io).trimmed()));
        else if(io>0&&io<qsl.size()-1)
            fprintf(fp,",%s",qPrintable(qsl.at(io).trimmed()));
        else
            fprintf(fp,",%s\n",qPrintable(qsl.at(io).trimmed()));

    }
    fclose(fp);
    return true;
}
//write basic info to conf file
bool readBrainConf(const QString &filename)
{
    //Qstring file = QFileInfo(filename).absoluteFilePath();
    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
            continue;
        }

        QStringList qsl = QString(buf).trimmed().split(",",QString::SkipEmptyParts);
        if (qsl.size()==0)   continue;

        for (int i=0; i<qsl.size(); i++)
        {
            qDebug()<<qsl.at(i);
        }
    }
    return true;
}

#endif // DBMS_BASIC_H
