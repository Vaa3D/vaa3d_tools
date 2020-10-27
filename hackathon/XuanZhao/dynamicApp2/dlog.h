#ifndef DLOG_H
#define DLOG_H

#include <QtMsgHandler>
#include <QMutex>
#include <QFile>
#include <QTextStream>

void messageOutput(QtMsgType type, const char *msg)
{
    static QMutex mutex;
    mutex.lock();
    QString txtMessage;
    switch (type) {
    case QtDebugMsg:
        txtMessage = QString("[Debug] %1").arg(msg);
        break;
    case QtWarningMsg:
        txtMessage = QString("[Warning] %1").arg(msg);
        break;
    case QtCriticalMsg:
        txtMessage = QString("[Critical] %1").arg(msg);
        break;
    case QtFatalMsg:
        txtMessage = QString("[Fatal] %1").arg(msg);
        abort();
    default:
        break;
    }
    txtMessage += QString("\r\n");
    QFile file(QObject::tr("D:\\reTraceTest\\qDebug.txt"));
    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream out(&file);
        out<<txtMessage;
    }
    file.flush();
    file.close();
    mutex.unlock();
}


#endif // DLOG_H
