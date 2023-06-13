#ifndef WORKSHOP_H
#define WORKSHOP_H

#include "FileManage.h"
#include "PluginPipeline.h"
#include "v3d_interface.h"
#include "ExternalFunction.h"
#include "GlobalConfig.h"

struct Region{
    float xs,xe,ys,ye,zs,ze;
    bool inside(float x,float y,float z){
        if(x<xs||x>xe||y<ys||y>ye||z<zs||z>ze)
            return false;
        return true;
    }
};

class WorkShop:public QObject{
    Q_OBJECT
public:
    WorkShop(V3DPluginCallback2 &callback);
    ~WorkShop();


    bool iscompleted(float x,float y,float z);
    void AssignImage(QString Image);
    void newPipeline(long xs, long ys, long zs, long xe, long ye, long ze);

public slots:

    void RegionDone(float xs,float xe,float ys,float ye,float zs,float ze);


    void ComboTracing(LandmarkList newTargetList);

private:
    QString currentImage;
    QList<PluginPipeLine*> ComboList;
    V3DPluginCallback2 *_callback;
    QList<Region> completedRegion;
    int blocksize=128;

    long xdim,ydim,zdim;

    static QReadWriteLock debuglockfile;
    static QMutex combolock;
    static int markernum;
};

#endif // WORKSHOP_H
