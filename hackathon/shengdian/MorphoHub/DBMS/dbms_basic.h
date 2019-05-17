#ifndef DBMS_BASIC_H
#define DBMS_BASIC_H

//#include "qstring.h"
#include <QtGui>
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
#endif // DBMS_BASIC_H
