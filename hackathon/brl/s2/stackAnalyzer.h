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
    StackAnalyzer(V3DPluginCallback2 &callback);


signals:
    void analysisDone(QList<LandmarkList> newTipsList, LandmarkList newTargets);
    void messageSignal(QString msg);
    void combinedSWC(QString fileSaveName);
public slots:
    void loadScan(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation);
    void processStack(Image4DSimple InputImage);
    void processSmartScan(QString fileWithData);
private:
    V3DPluginCallback2 * cb;

};

#endif // STACKANALYZER_H
