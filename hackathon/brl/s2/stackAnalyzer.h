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
    void analysisDone(QList<LandmarkList> newTipsList, LandmarkList newTargets, Image4DSimple* total4DImage_mip);
    void messageSignal(QString msg);
    void combinedSWC(QString fileSaveName);
    void loadingDone(Image4DSimple* total4DImage_mip);
public slots:
    void loadScan(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString, bool useGSDT, bool isSoma);
    void loadScan_MOST(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString, bool useGSDT, bool isSoma);

    void processStack(Image4DSimple InputImage);
    void processSmartScan(QString fileWithData);
    void loadGridScan(QString latestString,  LocationSimple tileLocation, QString saveDirString);
    NeuronTree sort_eliminate_swc(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage,bool isSoma);

private:
    V3DPluginCallback2 * cb;

};

#endif // STACKANALYZER_H
