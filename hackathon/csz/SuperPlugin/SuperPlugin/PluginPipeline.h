#ifndef PLUGINPIPELINE_H
#define PLUGINPIPELINE_H

#include "FileManage.h"
#include "TaskManage.h"
#include "ImageQuality.h"
#include "v3d_interface.h"
#include "OutSource.h"
#include <QVector>
#include <QMap>
#include "ExternalFunction.h"
#include "DetachConnectedDomain.h"
#include "GlobalConfig.h"
#include "../../vaa3d_tools/hackathon/zhi/APP2_large_scale/my_surf_objs.h"

#define INF 3.4e+38
struct pointer{
    void *p;
    int type;
    bool done;
};


class PluginPipeLine:public QObject{
    Q_OBJECT
public:
    PluginPipeLine(V3DPluginCallback2 &callback);
    ~PluginPipeLine();
    void getcenter(){
        QString das="";
        das+=QString::number((mxs+mxe)/2)+"_"+QString::number((mys+mye)/2)+"_"+QString::number((mzs+mze)/2);
        v3d_msg(das);

    }
signals:
    void sendLandmark(LandmarkList);
    void regionDone(float,float,float,float,float,float);
    void readytodel();
    void readytoconcat();
    void concatdone(QString,PluginPipeLine*);
    void finished();
public slots:

    void registerMemory(void* mem,int type);
    void releaseMemory(void* mem);
    void registerTaskMemory();

    void CropRawImage(QString ImageName,QVector<long> &boundingbox);
    void ConvertFileFormat(QString ImageName,PluginPipeLine* p);
    void tracingapp2(QString name,PluginPipeLine* p);
    void getLandMarkList(QString name,PluginPipeLine* p);
    QVector<long> getBoundingBox(long xs,long ys,long zs,long xe,long ye,long ze);
    char* qstring2char(QString text);
    void LimitBoundary(QVector<long> &boundingbox,Image4DSimple* img);

    void OutSourcedTask(QJsonObject &ost);
    void define(QString name,int type);

    void predict(QString name,PluginPipeLine* p);
    void Advantra(QString name);
    void neutube(QString name);
    void entv2s(QString name);
    void entv2n(QString name);
    void most(QString name);
    void mst(QString name);

    void DetachConnectedDomain(QString name);

    void selectAlgorithm(QString name,int method,PluginPipeLine *p);

    void submitSWC(QString swcname,PluginPipeLine *p);
    void ConcatSWC();

    void sendDelivery(Delivery delivery);

    void deletethis();

    void getstartpoint(QString name,QString IP="127.0.0.1",QString Port="6870");

    void ObtainStartPoint(int x,int y,int z,PluginPipeLine *p);

    vector<int> getCentroid();
private:
    QList<LandmarkList> group_tips(LandmarkList tips,int block_size, int direction);
    bool ada_win_finding_3D(LandmarkList tips,LocationSimple tileLocation,LandmarkList *newTargetList,int block_size,int direction);

    QJsonObject packing(QString path,int type);
    int connectdomaincount;
    QSet<QString> SWCtoConcat;
    QString tempSWCname;
    long mxs,mys,mzs,mxe,mye,mze;
    QVector<long> mboundingbox;
    V3DPluginCallback2 *_callback;
    QList<pointer> memory;
    QMap<void *,int> Mem2Pointer;
    QString FinalFileName;
    static QReadWriteLock lockfile;
    static QMutex debug;
    static QMutex debug1;
    OutSource *outsourcework;
    int px,py,pz;
    bool startready;

};



#endif // PLUGINPIPELINE_H
