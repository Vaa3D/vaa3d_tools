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
    void loadScan_adaptive(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString, bool useGSDT, bool isSoma);

    void loadScanSubtractive(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString, bool useGSDT, bool isSoma, int methodChoice);
    void loadScanSubtractiveAdaptive(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString, bool useGSDT, bool isSoma, int methodChoice);

    void processStack(Image4DSimple InputImage);
    void processSmartScan(QString fileWithData);

    void loadGridScan(QString latestString,  LocationSimple tileLocation, QString saveDirString);
    void updateChannel(QString);
    NeuronTree sort_eliminate_swc(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage,bool isSoma);
    LandmarkList eliminate_seed(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage);


    void ada_win_finding(LandmarkList tips,LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList,Image4DSimple* total4DImage,int block_size,int direction,float overlap);
    bool combine_list2file(QList<NeuronSWC> & lN, QString fileSaveName);

    QList<LandmarkList> group_tips(LandmarkList tips,int block_size, int direction);

    int methodSelection(QString latestString,int background, LandmarkList inputRootList,bool isSoma);

private:
    V3DPluginCallback2 * cb;
    QString channel;



    template <class T> void gaussian_filter(T* data1d,
                         V3DLONG *in_sz,
                         unsigned int Wx,
                         unsigned int Wy,
                         unsigned int Wz,
                         unsigned int c,
                         double sigma,
                         float* &outimg);




};

#endif // STACKANALYZER_H
