#include <v3d_interface.h>
#include "v3d_message.h"
#include "tracing_func.h"
#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"


using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))


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

    LandmarkList allTargetList;
    QList<LandmarkList> allTipsList;

    LocationSimple tileLocation;
    tileLocation.x = file_inmarkers[0].x;
    tileLocation.y = file_inmarkers[0].y;
    tileLocation.z = file_inmarkers[0].z;

    LandmarkList inputRootList;
    inputRootList.push_back(tileLocation);
    allTipsList.push_back(inputRootList);

    tileLocation.x = tileLocation.x -int(P.block_size/2);
    tileLocation.y = tileLocation.y -int(P.block_size/2);
    allTargetList.push_back(tileLocation);

    QString tmpfolder = QFileInfo(fileOpenName).path()+("/tmp");
    system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));
    if(tmpfolder.isEmpty())
    {
        printf("Can not create a tmp folder!\n");
        return false;
    }


    LandmarkList newTargetList;
    QList<LandmarkList> newTipsList;

    while(allTargetList.size()>0)
    {
        newTargetList.clear();
        newTipsList.clear();
        app2_tracing(callback,P,allTipsList.at(0),allTargetList.at(0),&newTargetList,&newTipsList);
        allTipsList.removeAt(0);
        allTargetList.removeAt(0);
        if(newTipsList.size()>0)
        {
            for(int i = 0; i < newTipsList.size(); i++)
            {
                allTargetList.push_back(newTargetList.at(i));
                allTipsList.push_back(newTipsList.at(i));
            }
        }
    }

    return true;
}


bool app2_tracing(V3DPluginCallback2 &callback,APP2_LS_PARA &P,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList)
{
    QString saveDirString = QFileInfo(P.inimg_file).path().append("/tmp");
    QString imageSaveString = saveDirString;

    imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append(".v3draw");

    ifstream ifs_image(imageSaveString.toStdString().c_str());
    if(ifs_image)
    {
        printf("the tile was scanned");
        return true;
    }

    unsigned char * total1dData = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;

    if(tileLocation.x >= P.in_sz[0] - 1 || tileLocation.y >= P.in_sz[1] - 1)
    {
        printf("hit the boundary");
        return true;
    }
    V3DLONG end_x,end_y;
    end_x = tileLocation.x+P.block_size;
    end_y = tileLocation.y+P.block_size;
    if(end_x > P.in_sz[0]) end_x = P.in_sz[0];
    if(end_y > P.in_sz[1]) end_y = P.in_sz[1];

    int datatype;
    if (!loadRawRegion(const_cast<char *>(P.inimg_file.toStdString().c_str()), total1dData, in_zz, in_sz,datatype,tileLocation.x,tileLocation.y,tileLocation.z,
                       end_x,end_y,tileLocation.z + P.in_sz[2]))
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

    V3DLONG mysz[4];
    mysz[0] = total4DImage->getXDim();
    mysz[1] = total4DImage->getYDim();
    mysz[2] = total4DImage->getZDim();
    mysz[3] = total4DImage->getCDim();

    simple_saveimage_wrapper(callback, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, total4DImage->getDatatype());

    QString scanDataFileString = saveDirString;
    scanDataFileString.append("/").append("scanData.txt");
    QString swcString = saveDirString;
    swcString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append(".swc");


    qDebug()<<scanDataFileString;
    QFile saveTextFile;
    saveTextFile.setFileName(scanDataFileString);// add currentScanFile
    if (!saveTextFile.isOpen()){
        if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
            qDebug()<<"unable to save file!";
            return false;}     }
    QTextStream outputStream;
    outputStream.setDevice(&saveTextFile);
    outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<"\n";
    saveTextFile.close();


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
   // LandmarkList imageLandmarks;


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

    NeuronTree nt;
    nt = readSWC_file(swcString);
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    LandmarkList tip_left;
    LandmarkList tip_right;
    LandmarkList tip_up ;
    LandmarkList tip_down;
    QList<NeuronSWC> list = nt.listNeuron;
    for (V3DLONG i=0;i<list.size();i++)
    {
        if (childs[i].size()==0)
        {
            NeuronSWC curr = list.at(i);
            LocationSimple newTip;
            if( curr.x < 0.05*  p.p4dImage->getXDim() || curr.x > 0.95 *  p.p4dImage->getXDim() || curr.y < 0.05 * p.p4dImage->getYDim() || curr.y > 0.95* p.p4dImage->getYDim())
            {
                V3DLONG node_pn = getParent(i,nt);// Zhi, what if there's no parent?
                V3DLONG node_pn_2nd;
                if( list.at(node_pn).pn < 0)
                {
                    node_pn_2nd = node_pn;
                }
                else
                {
                    node_pn_2nd = getParent(node_pn,nt);
                }

                newTip.x = list.at(node_pn_2nd).x + p.p4dImage->getOriginX();
                newTip.y = list.at(node_pn_2nd).y + p.p4dImage->getOriginY();
                newTip.z = list.at(node_pn_2nd).z + p.p4dImage->getOriginZ();
            }
            if( curr.x < 0.05* p.p4dImage->getXDim())
            {
                tip_left.push_back(newTip);
            }else if (curr.x > 0.95 * p.p4dImage->getXDim())
            {
                tip_right.push_back(newTip);
            }else if (curr.y < 0.05 * p.p4dImage->getYDim())
            {
                tip_up.push_back(newTip);
            }else if (curr.y > 0.95*p.p4dImage->getYDim())
            {
                tip_down.push_back(newTip);
            }
        }
    }

    double overlap = 0.1;
    LocationSimple newTarget;
    if(tip_left.size()>0)
    {
        newTipsList->push_back(tip_left);
        newTarget.x = -p.p4dImage->getXDim()*(1.0-overlap) + p.p4dImage->getOriginX();
        if (newTarget.x < 0) newTarget.x = 0;
        if (newTarget.x > P.in_sz[0] - 1) newTarget.x = newTarget.x > P.in_sz[0]-1;
        newTarget.y = p.p4dImage->getOriginY();
        newTarget.z = p.p4dImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_right.size()>0)
    {
        newTipsList->push_back(tip_right);
        newTarget.x = p.p4dImage->getXDim()*(1.0-overlap) + p.p4dImage->getOriginX();
        if (newTarget.x < 0) newTarget.x = 0;
        if (newTarget.x > P.in_sz[0] - 1) newTarget.x = newTarget.x > P.in_sz[0]-1;
        newTarget.y = p.p4dImage->getOriginY();
        newTarget.z = p.p4dImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_up.size()>0)
    {
        newTipsList->push_back(tip_up);
        newTarget.x = p.p4dImage->getOriginX();
        newTarget.y = -p.p4dImage->getYDim()*(1.0-overlap) + p.p4dImage->getOriginY();
        if (newTarget.y < 0) newTarget.y = 0;
        if (newTarget.y > P.in_sz[0] - 1) newTarget.y = newTarget.y > P.in_sz[0]-1;
        newTarget.z = p.p4dImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_down.size()>0)
    {
        newTipsList->push_back(tip_down);
        newTarget.x = p.p4dImage->getOriginX();
        newTarget.y = p.p4dImage->getYDim()*(1.0-overlap) + p.p4dImage->getOriginY();
        if (newTarget.y < 0) newTarget.y = 0;
        if (newTarget.y > P.in_sz[0] - 1) newTarget.y = newTarget.y > P.in_sz[0]-1;
        newTarget.z = p.p4dImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }

    return true;
}

