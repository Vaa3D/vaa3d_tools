#include "SpLog.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include "v3d_message.h"


QMutex SpLog::mainlock = QMutex();
QMutex SpLog::instancelock = QMutex();
QMutex SpLog::templock = QMutex();
SpLog *SpLog::_instance = nullptr;

SpLog::~SpLog()
{

}

void SpLog::DebugToFile(QString txt,QString path)
{
    templock.lock();
    QFile file(path);
    if (!file.open(QIODevice::Append | QIODevice::Text))
            v3d_msg(file.errorString());
    QTextStream out(&file);
    out<<txt<<"\n";
    file.close();
    templock.unlock();
}

void SpLog::WritetoText(QString txt)
{
    mainlock.lock();
    QFile file(DebugPath+"/log"+PluginRunStartTime+".txt");
    if (!file.open(QIODevice::Append | QIODevice::Text))
            v3d_msg(file.errorString());
    QTextStream out(&file);
    out<<txt<<"\n";
    file.close();
    mainlock.unlock();
}
