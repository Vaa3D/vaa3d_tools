#ifndef STACKANALYZER_H
#define STACKANALYZER_H

#include <QObject>
#include <v3d_interface.h>
#include <QString>>
#include <QFileInfo>
#include <QDebug>

class StackAnalyzer : public QObject
{
    Q_OBJECT
public:
    StackAnalyzer(V3DPluginCallback2 &callback, QObject *parent = 0 );


signals:
    void analysisDone(QList<QList<int> > newTargets);
    void messageSignal(QString msg);
public slots:
    void loadScan();
    void processStack(Image4DSimple * pInputImage);
private:
    V3DPluginCallback2 * cb;

};

#endif // STACKANALYZER_H
