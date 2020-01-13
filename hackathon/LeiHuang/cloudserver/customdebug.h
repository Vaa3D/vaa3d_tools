#ifndef CUSTOMDEBUG_H
#define CUSTOMDEBUG_H
#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <iostream>


void customMessageHandler(QtMsgType type, const char *msg)
{
    QString txtMessage;
    QString time;
    switch (type)
    {
        case QtDebugMsg:    //调试信息提示
            txtMessage = QString("Debug: %1").arg(msg);
            break;

        case QtWarningMsg:    //一般的warning提示
            txtMessage = QString("Warning: %1").arg(msg);
            break;

        case QtCriticalMsg:    //严重错误提示
            txtMessage = QString("Critical: %1").arg(msg);
            break;

        case QtFatalMsg:    //致命错误提示
            txtMessage = QString("Fatal: %1").arg(msg);
            abort();
    }

    //保存输出相关信息到指定文件
    QFile outputFile("customMessageLog.txt");


    outputFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&outputFile);
    time = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    textStream << time<<":"<<txtMessage << endl;
    std::cout<<time.toStdString().data()<<txtMessage.toStdString().data()<<"\n";
}

#endif // CUSTOMDEBUG_H
