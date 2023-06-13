#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include <QString>

extern bool globalconfig;

extern QString DebugPath;

extern QString UnetIP;
extern QString UnetPort;

extern QString AlgorithmPredictIP;
extern QString AlgorithmPredictPort;

extern int ThreadPoolSize;

extern int app2id;
extern int advantraid;

extern QString PluginRunStartTime;

void ReadConfigFile(QString filepath);

#endif // GLOBALCONFIG_H
