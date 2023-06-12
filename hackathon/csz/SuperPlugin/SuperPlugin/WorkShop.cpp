#include "WorkShop.h"


QReadWriteLock WorkShop::debuglockfile = QReadWriteLock();
int WorkShop::markernum=1;
QMutex WorkShop::combolock = QMutex();


WorkShop::WorkShop(V3DPluginCallback2 &callback)
{
    _callback=&callback;
    ComboList.clear();
    currentImage="";
}

WorkShop::~WorkShop()
{

}

void WorkShop::newPipeline(long xs, long ys, long zs, long xe, long ye, long ze)
{
    if(xdim<blocksize){
        xs=0;
        xe=xdim;
    }
    if(ydim<blocksize){
        ys=0;
        ye=ydim;
    }
    if(zdim<blocksize){
        zs=0;
        ze=zdim;
    }
    if(xs<0)
        xs=0;
    if(xe>xdim)
        xe=xdim;
    if(ys<0)
        ys=0;
    if(ye>ydim)
        ye=ydim;
    if(zs<0)
        zs=0;
    if(ze>zdim)
        ze=zdim;
    PluginPipeLine *np=new PluginPipeLine(*_callback);
    QVector<long> bb;
    bb=np->getBoundingBox(xs,ys,zs,xe,ye,ze);
    connect(np,SIGNAL(sendLandmark(LandmarkList)),this,SLOT(ComboTracing(LandmarkList)));
//    np->define(currentImage,1);
    ComboList.append(np);
    RegionDone(xs,xe,ys,ye,zs,ze);
    QVector<long> bbox=np->getBoundingBox(xs,ys,zs,xe,ye,ze);
    np->CropRawImage(currentImage, bbox);
}

void WorkShop::RegionDone(float xs, float xe, float ys, float ye, float zs, float ze)
{
    Region newregion;
    newregion.xs=xs;
    newregion.xe=xe;
    newregion.ys=ys;
    newregion.ye=ye;
    newregion.zs=zs;
    newregion.ze=ze;
    completedRegion.append(newregion);
}

void WorkShop::ComboTracing(LandmarkList newTargetList)
{
    combolock.lock();
    for(int i=0;i<newTargetList.size();i++){
        LocationSimple tileLocation=newTargetList[i];
        V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = (tileLocation.x < 0)?  0 : tileLocation.x;
        start_y = (tileLocation.y < 0)?  0 : tileLocation.y;
        start_z = (tileLocation.z < 0)?  0 : tileLocation.z;
        end_x = tileLocation.x+tileLocation.ev_pc1;
        end_y = tileLocation.y+tileLocation.ev_pc2;
        end_z = tileLocation.z+tileLocation.ev_pc3;
        if(xdim<blocksize){
            start_x=0;
            end_x=xdim;
        }
        if(ydim<blocksize){
            start_y=0;
            end_y=ydim;
        }
        if(zdim<blocksize){
            start_z=0;
            end_z=zdim;
        }
//        if(iscompleted((start_x+end_x)/2,(start_y+end_y)/2,(start_z+end_z)/2)){
//            continue;
//        }

//        debuglockfile.lockForWrite();
        QFile file(DebugPath+"/output"+PluginRunStartTime+".swc");
        if (!file.open(QIODevice::Append | QIODevice::Text))
                v3d_msg(file.errorString());
        QTextStream out(&file);
        out <<markernum++<<" 3 "<<(start_x+end_x)/2<<" "<<(start_y+end_y)/2<<" "<<(start_z+end_z)/2<<" "<<3<<" "<<-1<< "\n";
        file.close();
//        debuglockfile.unlock();

        newPipeline(start_x,start_y,start_z,end_x,end_y,end_z);
    }
    combolock.unlock();
}

bool WorkShop::iscompleted(float x, float y, float z)
{
    for(int i=0;i<completedRegion.size();i++){
        if(completedRegion[i].inside(x,y,z)){
            return true;
        }
    }
    return false;
}

void WorkShop::AssignImage(QString Image)
{
    currentImage=Image;
    Image4DSimple *newimg=_callback->loadImage(qstring2char(Image));
    xdim=newimg->getXDim();
    ydim=newimg->getYDim();
    zdim=newimg->getZDim();
}
