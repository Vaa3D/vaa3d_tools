#include "GlobalConfig.h"
#include <QFile>
#include <QHash>
#include <QDebug>

bool globalconfig;

QString DebugPath;

QString UnetIP;
QString UnetPort;

QString AlgorithmPredictIP;
QString AlgorithmPredictPort;

int ThreadPoolSize;

int app2id;
int advantraid;


void ReadConfigFile(QString filepath)
{
    QFile qf(filepath);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    QHash <QString,QString> fileConfigs;

    auto IsDuplicateOption = [&](QString const& confOption)
    {
        auto const& itr = fileConfigs.find(confOption);
        if (itr != fileConfigs.end())
        {
            return true;
        }

        return false;
    };

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
        QString qsl = QString(buf).trimmed();
        if(qsl.size()==0||!qsl.contains("="))
            continue;
        QStringList list=qsl.split("=");
        if (IsDuplicateOption(list.at(0)))
        {
            continue;
        }

        if(list.size()!=2)
            continue;
        fileConfigs.insert(list.at(0),list.at(1));
    }

    DebugPath=fileConfigs["DebugPath"];
    UnetIP=fileConfigs["UnetIP"];
    UnetPort=fileConfigs["UnetPort"];
    AlgorithmPredictIP=fileConfigs["AlgorithmPredictIP"];
    AlgorithmPredictPort=fileConfigs["AlgorithmPredictPort"];
    ThreadPoolSize=fileConfigs["ThreadPoolSize"].toInt();
    app2id=fileConfigs["app2id"].toInt();
    advantraid=fileConfigs["advantraid"].toInt();

    globalconfig=true;
}
