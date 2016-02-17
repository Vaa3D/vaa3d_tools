#include <v3d_interface.h>
#include "v3d_message.h"
#include "tracing_func.h"
#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"


using namespace std;

bool crawler_raw(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &P,bool bmenu)
{
    QString fileOpenName = P.inimg_file;

    unsigned char * datald = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;

    int datatype;
    if (!loadRawRegion(const_cast<char *>(P.inimg_file.toStdString().c_str()), datald, in_zz, in_sz,datatype,0,0,0,1,1,1))
    {
        return false;
    }
    if(datald) {delete []datald; datald = 0;}

    P.in_sz[0] = in_zz[0];
    P.in_sz[1] = in_zz[1];
    P.in_sz[2] = in_zz[2];

    vector<MyMarker> file_inmarkers;
    file_inmarkers = readMarker_file(string(qPrintable(P.markerfilename)));

    LocationSimple tileLocation;
    tileLocation.x = file_inmarkers[0].x;
    tileLocation.y = file_inmarkers[0].y;
    tileLocation.z = file_inmarkers[0].z;

    LandmarkList inputRootList;
    inputRootList.push_back(tileLocation);

    tileLocation.x = tileLocation.x -int(P.block_size/2);
    tileLocation.y = tileLocation.y -int(P.block_size/2);

    QString tmpfolder = QFileInfo(fileOpenName).path()+("/tmp");
    system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));
    if(tmpfolder.isEmpty())
    {

        printf("Can not create a tmp folder!\n");
        return false;
    }

    app2_tracing(callback,P,inputRootList,tileLocation);
    return true;
}


bool app2_tracing(V3DPluginCallback2 &callback,APP2_LS_PARA &P,LandmarkList inputRootList, LocationSimple tileLocation)
{
    unsigned char * total1dData = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;

    int datatype;
    if (!loadRawRegion(const_cast<char *>(P.inimg_file.toStdString().c_str()), total1dData, in_zz, in_sz,datatype,tileLocation.x,tileLocation.y,tileLocation.z,
                       tileLocation.x+P.block_size-1,tileLocation.y+P.block_size-1,tileLocation.z + P.in_sz[2]-1))
    {
        printf("can not load the region");
        if(total1dData) {delete []total1dData; total1dData = 0;}
        return false;
    }

    Image4DSimple* total4DImage = new Image4DSimple;
    total4DImage->setData((unsigned char*)total1dData, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
    total4DImage->setOriginX(tileLocation.x);
    total4DImage->setOriginY(tileLocation.y);
    total4DImage->setOriginZ(tileLocation.z);


    PARA_APP2 p;
    p.is_gsdt = P.is_gsdt;
    p.is_coverage_prune = true;
    p.is_break_accept = false;
    p.bkg_thresh = P.bkg_thresh;
    p.length_thresh = P.length_thresh;
    p.cnn_type = 2;
    p.channel = 0;
    p.SR_ratio = 3.0/9.9;
    p.b_256cube = P.b_256cube;
    p.b_RadiusFrom2D = true;
    p.b_resample = 1;
    p.b_intensity = 0;
    p.b_brightfiled = 0;
    p.b_menu = 0; //if set to be "true", v3d_msg window will show up.

    p.p4dImage = total4DImage;
    p.xc0 = p.yc0 = p.zc0 = 0;
    p.xc1 = p.p4dImage->getXDim()-1;
    p.yc1 = p.p4dImage->getYDim()-1;
    p.zc1 = p.p4dImage->getZDim()-1;
    QString versionStr = "v2.621";

    qDebug()<<"starting app2";
    qDebug()<<"rootlist size "<<QString::number(inputRootList.size());
    LandmarkList imageLandmarks;
    QString saveDirString = QFileInfo(P.inimg_file).path().append("/tmp");
    QString swcString = saveDirString;
    swcString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append(".swc");


    if(inputRootList.size() <1)
    {
        p.outswc_file =swcString;
        proc_app2(callback, p, versionStr);
    }
    else
    {
        vector<MyMarker*> tileswc_file;
        for(int i = 0; i < inputRootList.size(); i++)
        {
            p.outswc_file =swcString + (QString::number(i)) + (".swc");
            LocationSimple RootNewLocation;
            RootNewLocation.x = inputRootList.at(i).x - p.p4dImage->getOriginX();
            RootNewLocation.y = inputRootList.at(i).y - p.p4dImage->getOriginY();
            RootNewLocation.z = inputRootList.at(i).z - p.p4dImage->getOriginZ();

            p.landmarks.push_back(RootNewLocation);
            proc_app2(callback, p, versionStr);
            p.landmarks.clear();
            vector<MyMarker*> inputswc = readSWC_file(p.outswc_file.toStdString());
            qDebug()<<"ran app2";
            for(V3DLONG d = 0; d < inputswc.size(); d++)
            {
                tileswc_file.push_back(inputswc[d]);
            }
        }
        saveSWC_file(swcString.toStdString().c_str(), tileswc_file);
    }
}

