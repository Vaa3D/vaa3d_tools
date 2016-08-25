#ifndef THREADEDTRACER_H
#define THREADEDTRACER_H

#include <QRunnable>
#include <v3d_interface.h>
#include <QString>>
#include <QFileInfo>
#include <QDebug>

class ThreadedTracer : public QRunnable
{
public:
    explicit ThreadedTracer(V3DPluginCallback2 &callback, QString latestString, LocationSimple tileLocation, QString saveDirString, QString channel, int tileNumber);
    void run();
signals:
    void done();
private:
    V3DPluginCallback2 * cb;
    QString latestString;
    LocationSimple tileLocation;
    QString saveDirString;
    QString channel;
    int tileNumber;
    
};

#endif // THREADEDTRACER_H
