#include <v3d_interface.h>
#include "v3d_message.h"
#include "tracing_func.h"
#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app1.h"

//#include "../../../hackathon/zhi/AllenNeuron_postprocessing/sort_swc_IVSCC.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"


#include <boost/lexical_cast.hpp>
template <class T> T pow2(T a)
{
    return a*a;

}

using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
bool export_list2file(vector<MyMarker*> & outmarkers, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    QFile qf(fileOpenName);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QString info;
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
           info = buf;
           myfile<< info.remove('\n') <<endl;
        }

    }

    map<MyMarker*, int> ind;
    for(int i = 0; i < outmarkers.size(); i++) ind[outmarkers[i]] = i+1;

    for(V3DLONG i = 0; i < outmarkers.size(); i++)
    {
        MyMarker * marker = outmarkers[i];
        int parent_id;
        if(marker->parent == 0) parent_id = -1;
        else parent_id = ind[marker->parent];
        myfile<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<"\n";
    }

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<outmarkers.size()<<endl;
    return true;
};

bool crawler_raw_app(V3DPluginCallback2 &callback, QWidget *parent,APP_LS_PARA &P,bool bmenu)
{
    QElapsedTimer timer1;
    timer1.start();

    QString fileOpenName = P.inimg_file;

    if(P.image)
    {
        P.in_sz[0] = P.image->getXDim();
        P.in_sz[1] = P.image->getYDim();
        P.in_sz[2] = P.image->getZDim();
    }else
    {
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
        LocationSimple t;
        for(int i = 0; i < file_inmarkers.size(); i++)
        {
            t.x = file_inmarkers[i].x + 1;
            t.y = file_inmarkers[i].y + 1;
            t.z = file_inmarkers[i].z + 1;
            P.listLandmarks.push_back(t);
        }
    }

    LandmarkList allTargetList;
    QList<LandmarkList> allTipsList;

    LocationSimple tileLocation;
    tileLocation.x = P.listLandmarks[0].x;
    tileLocation.y = P.listLandmarks[0].y;
    tileLocation.z = P.listLandmarks[0].z;

    LandmarkList inputRootList;
    inputRootList.push_back(tileLocation);
    allTipsList.push_back(inputRootList);

    tileLocation.x = tileLocation.x -int(P.block_size/2);
    tileLocation.y = tileLocation.y -int(P.block_size/2);
    tileLocation.z = 0;
    tileLocation.radius = P.block_size;
    allTargetList.push_back(tileLocation);

    QString tmpfolder;
    if(P.visible_thresh)
        tmpfolder= QFileInfo(fileOpenName).path()+("/tmp_APP1");
    else
        tmpfolder= QFileInfo(fileOpenName).path()+("/tmp_APP2");

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
        if(P.adap_win)
            app_tracing_ada_win(callback,P,allTipsList.at(0),allTargetList.at(0),&newTargetList,&newTipsList);
        else
            app_tracing(callback,P,allTipsList.at(0),allTargetList.at(0),&newTargetList,&newTipsList);
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

    qint64 etime1 = timer1.elapsed();

    list<string> infostring;
    string tmpstr; QString qtstr;
    if(P.visible_thresh)
    {
        tmpstr =  qPrintable( qtstr.prepend("## NeuronCrawler_APP1")); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.channel).prepend("#channel = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.b_256cube).prepend("#b_256cube = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.visible_thresh).prepend("#visible_thresh = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.block_size).prepend("#block_size = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(etime1).prepend("#neuron preprocessing time (milliseconds) = ") ); infostring.push_back(tmpstr);

    }else
    {
        tmpstr =  qPrintable( qtstr.prepend("## NeuronCrawler_APP2")); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.channel).prepend("#channel = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);

        tmpstr =  qPrintable( qtstr.setNum(P.length_thresh).prepend("#length_thresh = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.SR_ratio).prepend("#SR_ratio = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.is_gsdt).prepend("#is_gsdt = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.is_break_accept).prepend("#is_gap = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.cnn_type).prepend("#cnn_type = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.b_256cube).prepend("#b_256cube = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.b_RadiusFrom2D).prepend("#b_radiusFrom2D = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.block_size).prepend("#block_size = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(etime1).prepend("#neuron preprocessing time (milliseconds) = ") ); infostring.push_back(tmpstr);
    }

    processSmartScan(callback,infostring,tmpfolder +"/scanData.txt");


    v3d_msg(QString("The tracing uses %1 for tracing. Now you can drag and drop the generated swc fle [%2] into Vaa3D."
                    ).arg(etime1).arg(tmpfolder +"/scanData.txt.swc"), 1);

    return true;
}

bool app_tracing(V3DPluginCallback2 &callback,APP_LS_PARA &P,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList)
{

    QString saveDirString;
    if(P.visible_thresh)
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_APP1");
    else
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_APP2");

    QString imageSaveString = saveDirString;

    V3DLONG start_x,start_y,end_x,end_y;
    start_x = (tileLocation.x < 0)?  0 : tileLocation.x;
    start_y = (tileLocation.y < 0)?  0 : tileLocation.y;

    end_x = tileLocation.x+P.block_size;
    end_y = tileLocation.y+P.block_size;
    if(end_x > P.in_sz[0]) end_x = P.in_sz[0];
    if(end_y > P.in_sz[1]) end_y = P.in_sz[1];

    if(tileLocation.x >= P.in_sz[0] - 1 || tileLocation.y >= P.in_sz[1] - 1 || end_x <= 0 || end_y <= 0 )
    {
        printf("hit the boundary");
        return true;
    }

    unsigned char * total1dData = 0;
    V3DLONG *in_sz = 0;

    if(P.image)
    {
        in_sz = new V3DLONG[4];
        in_sz[0] = end_x - start_x;
        in_sz[1] = end_y - start_y;
        in_sz[2] = P.in_sz[2];
        V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
        try {total1dData = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for loading the region.",0); return false;}
        V3DLONG i = 0;
        for(V3DLONG iz = 0; iz < P.in_sz[2]; iz++)
        {
            V3DLONG offsetk = iz*P.in_sz[1]*P.in_sz[0];
            for(V3DLONG iy = start_y; iy < end_y; iy++)
            {
                V3DLONG offsetj = iy*P.in_sz[0];
                for(V3DLONG ix = start_x; ix < end_x; ix++)
                {
                    total1dData[i] = P.image->getRawData()[offsetk + offsetj + ix];
                    i++;
                }
            }
        }
    }else
    {
        V3DLONG *in_zz = 0;
        int datatype;
        if (!loadRawRegion(const_cast<char *>(P.inimg_file.toStdString().c_str()), total1dData, in_zz, in_sz,datatype,start_x,start_y,tileLocation.z,
                           end_x,end_y,tileLocation.z + P.in_sz[2]))
        {
            printf("can not load the region");
            if(total1dData) {delete []total1dData; total1dData = 0;}
            return false;
        }
    }

    Image4DSimple* total4DImage = new Image4DSimple;
    total4DImage->setData((unsigned char*)total1dData, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
    total4DImage->setOriginX(start_x);
    total4DImage->setOriginY(start_y);
    total4DImage->setOriginZ(tileLocation.z);

    V3DLONG mysz[4];
    mysz[0] = total4DImage->getXDim();
    mysz[1] = total4DImage->getYDim();
    mysz[2] = total4DImage->getZDim();
    mysz[3] = total4DImage->getCDim();

    imageSaveString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y).append(".v3draw"));

    QString scanDataFileString = saveDirString;
    scanDataFileString.append("/").append("scanData.txt");
    QString swcString = saveDirString;
    swcString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".swc");

    PARA_APP1 p1;
    PARA_APP2 p2;
    QString versionStr = "v0.001";

    if(P.visible_thresh)
    {
        p1.bkg_thresh = P.bkg_thresh;
        p1.channel = P.channel-1;
        p1.b_256cube = P.b_256cube;
        p1.visible_thresh = P.visible_thresh;

        p1.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p1.p4dImage = total4DImage;
        p1.xc0 = p1.yc0 = p1.zc0 = 0;
        p1.xc1 = p1.p4dImage->getXDim()-1;
        p1.yc1 = p1.p4dImage->getYDim()-1;
        p1.zc1 = p1.p4dImage->getZDim()-1;
        qDebug()<<"starting app1";
    }
    else
    {
        p2.is_gsdt = P.is_gsdt;
        p2.is_coverage_prune = true;
        p2.is_break_accept = false;
        p2.bkg_thresh = P.bkg_thresh;
        p2.length_thresh = P.length_thresh;
        p2.cnn_type = 2;
        p2.channel = 0;
        p2.SR_ratio = 3.0/9.9;
        p2.b_256cube = P.b_256cube;
        p2.b_RadiusFrom2D = true;
        p2.b_resample = 1;
        p2.b_intensity = 0;
        p2.b_brightfiled = 0;
        p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p2.p4dImage = total4DImage;
        p2.xc0 = p2.yc0 = p2.zc0 = 0;
        p2.xc1 = p2.p4dImage->getXDim()-1;
        p2.yc1 = p2.p4dImage->getYDim()-1;
        p2.zc1 = p2.p4dImage->getZDim()-1;
    }

    NeuronTree nt;

    ifstream ifs_image(imageSaveString.toStdString().c_str());
    if(!ifs_image)
    {
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

        simple_saveimage_wrapper(callback, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, total4DImage->getDatatype());

        if(P.visible_thresh)
            qDebug()<<"starting app1";
        else
            qDebug()<<"starting app2";
        qDebug()<<"rootlist size "<<QString::number(inputRootList.size());

       // LandmarkList imageLandmarks;

        if(inputRootList.size() <1)
        {
            if(P.visible_thresh)
            {
                p1.outswc_file =swcString;
                proc_app1(callback, p1, versionStr);
            }
            else
            {
                p2.outswc_file =swcString;
                proc_app2(callback, p2, versionStr);
            }
        }
        else
        {
            vector<MyMarker*> tileswc_file;
            for(int i = 0; i < inputRootList.size(); i++)
            {
                QString poutswc_file = swcString + (QString::number(i)) + (".swc");
                if(P.visible_thresh)
                    p1.outswc_file =poutswc_file;
                else
                    p2.outswc_file =poutswc_file;

                LocationSimple RootNewLocation;
                RootNewLocation.x = inputRootList.at(i).x - total4DImage->getOriginX();
                RootNewLocation.y = inputRootList.at(i).y - total4DImage->getOriginY();
                RootNewLocation.z = inputRootList.at(i).z - total4DImage->getOriginZ();

                bool flag = false;
                if(tileswc_file.size()>0)
                {
                    for(V3DLONG dd = 0; dd < tileswc_file.size();dd++)
                    {
                        double dis = sqrt(pow2(RootNewLocation.x - tileswc_file.at(dd)->x) + pow2(RootNewLocation.y - tileswc_file.at(dd)->y) + pow2(RootNewLocation.z - tileswc_file.at(dd)->z));
                        if(dis < 10.0)
                        {
                           flag = true;
                           break;
                        }
                    }
                }

                if(!flag)
                {
                    if(P.visible_thresh)
                    {
                        p1.landmarks.push_back(RootNewLocation);
                        proc_app1(callback, p1, versionStr);
                        p1.landmarks.clear();
                    }else
                    {
                        p2.landmarks.push_back(RootNewLocation);
                        proc_app2(callback, p2, versionStr);
                        p2.landmarks.clear();
                    }

                    vector<MyMarker*> inputswc = readSWC_file(poutswc_file.toStdString());
                    for(V3DLONG d = 0; d < inputswc.size(); d++)
                    {
                        tileswc_file.push_back(inputswc[d]);
                    }
                }
            }
            saveSWC_file(swcString.toStdString().c_str(), tileswc_file);
            nt = readSWC_file(swcString);
        }

    }else
    {
        NeuronTree nt_tile = readSWC_file(swcString);
        LandmarkList inputRootList_pruned = eliminate_seed(nt_tile,inputRootList,total4DImage);
        if(inputRootList_pruned.size()<1)
            return true;
        else
        {
            vector<MyMarker*> tileswc_file;
            QString swcString_2nd = swcString + ("_2.swc");
            for(int i = 0; i < inputRootList_pruned.size(); i++)
            {
                QString poutswc_file = swcString + (QString::number(i)) + ("_2.swc");
                if(P.visible_thresh)
                    p1.outswc_file = poutswc_file;
                else
                    p2.outswc_file = poutswc_file;

                LocationSimple RootNewLocation;
                RootNewLocation.x = inputRootList_pruned.at(i).x - total4DImage->getOriginX();
                RootNewLocation.y = inputRootList_pruned.at(i).y - total4DImage->getOriginY();
                RootNewLocation.z = inputRootList_pruned.at(i).z - total4DImage->getOriginZ();

                bool flag = false;
                if(tileswc_file.size()>0)
                {
                    for(V3DLONG dd = 0; dd < tileswc_file.size();dd++)
                    {
                        double dis = sqrt(pow2(RootNewLocation.x - tileswc_file.at(dd)->x) + pow2(RootNewLocation.y - tileswc_file.at(dd)->y) + pow2(RootNewLocation.z - tileswc_file.at(dd)->z));
                        if(dis < 10.0)
                        {
                           flag = true;
                           break;
                        }
                    }
                }

                if(!flag)
                {
                    if(P.visible_thresh)
                    {
                        p1.landmarks.push_back(RootNewLocation);
                        proc_app1(callback, p1, versionStr);
                        p1.landmarks.clear();
                    }else
                    {
                        p2.landmarks.push_back(RootNewLocation);
                        proc_app2(callback, p2, versionStr);
                        p2.landmarks.clear();
                    }
                    vector<MyMarker*> inputswc = readSWC_file(poutswc_file.toStdString());
                    qDebug()<<"ran app2";
                    for(V3DLONG d = 0; d < inputswc.size(); d++)
                    {
                        tileswc_file.push_back(inputswc[d]);
                    }
                }
            }
            vector<MyMarker*> tileswc_file_total = readSWC_file(swcString.toStdString());
            for(V3DLONG d = 0; d < tileswc_file.size(); d++)
            {
                tileswc_file_total.push_back(tileswc_file[d]);
            }

            saveSWC_file(swcString.toStdString().c_str(), tileswc_file_total);
            saveSWC_file(swcString_2nd.toStdString().c_str(), tileswc_file);
            nt = readSWC_file(swcString_2nd);
        }

    }

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
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                V3DLONG node_pn = getParent(i,nt);
                V3DLONG node_pn_2nd;
                if( list.at(node_pn).pn < 0)
                {
                    node_pn_2nd = node_pn;
                }
                else
                {
                    node_pn_2nd = getParent(node_pn,nt);
                }

                newTip.x = list.at(node_pn_2nd).x + total4DImage->getOriginX();
                newTip.y = list.at(node_pn_2nd).y + total4DImage->getOriginY();
                newTip.z = list.at(node_pn_2nd).z + total4DImage->getOriginZ();
            }
            if( curr.x < 0.05* total4DImage->getXDim())
            {
                tip_left.push_back(newTip);
            }else if (curr.x > 0.95 * total4DImage->getXDim())
            {
                tip_right.push_back(newTip);
            }else if (curr.y < 0.05 * total4DImage->getYDim())
            {
                tip_up.push_back(newTip);
            }else if (curr.y > 0.95*total4DImage->getYDim())
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
        newTarget.x = -floor(P.block_size*(1.0-overlap)) + tileLocation.x;
        newTarget.y = total4DImage->getOriginY();
        newTarget.z = total4DImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_right.size()>0)
    {
        newTipsList->push_back(tip_right);
        newTarget.x = floor(P.block_size*(1.0-overlap)) + tileLocation.x;
        newTarget.y = total4DImage->getOriginY();
        newTarget.z = total4DImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_up.size()>0)
    {
        newTipsList->push_back(tip_up);
        newTarget.x = total4DImage->getOriginX();
        newTarget.y = -floor(P.block_size*(1.0-overlap)) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_down.size()>0)
    {
        newTipsList->push_back(tip_down);
        newTarget.x = total4DImage->getOriginX();
        newTarget.y = floor(P.block_size*(1.0-overlap)) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    total4DImage->deleteRawDataAndSetPointerToNull();

    return true;
}

bool app_tracing_ada_win(V3DPluginCallback2 &callback,APP_LS_PARA &P,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList)
{
    QString saveDirString;
    if(P.visible_thresh)
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_APP1");
    else
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_APP2");

    QString imageSaveString = saveDirString;

    V3DLONG start_x,start_y,end_x,end_y;
    start_x = (tileLocation.x < 0)?  0 : tileLocation.x;
    start_y = (tileLocation.y < 0)?  0 : tileLocation.y;

    end_x = tileLocation.x+tileLocation.radius;
    end_y = tileLocation.y+tileLocation.radius;
    if(end_x > P.in_sz[0]) end_x = P.in_sz[0];
    if(end_y > P.in_sz[1]) end_y = P.in_sz[1];

    if(tileLocation.x >= P.in_sz[0] - 1 || tileLocation.y >= P.in_sz[1] - 1 || end_x <= 0 || end_y <= 0 )
    {
        printf("hit the boundary");
        return true;
    }

    unsigned char * total1dData = 0;
    V3DLONG *in_sz = 0;

    if(P.image)
    {
        in_sz = new V3DLONG[4];
        in_sz[0] = end_x - start_x;
        in_sz[1] = end_y - start_y;
        in_sz[2] = P.in_sz[2];
        V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
        try {total1dData = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for loading the region.",0); return false;}
        V3DLONG i = 0;
        for(V3DLONG iz = 0; iz < P.in_sz[2]; iz++)
        {
            V3DLONG offsetk = iz*P.in_sz[1]*P.in_sz[0];
            for(V3DLONG iy = start_y; iy < end_y; iy++)
            {
                V3DLONG offsetj = iy*P.in_sz[0];
                for(V3DLONG ix = start_x; ix < end_x; ix++)
                {
                    total1dData[i] = P.image->getRawData()[offsetk + offsetj + ix];
                    i++;
                }
            }
        }
    }else
    {
        V3DLONG *in_zz = 0;
        int datatype;
        if (!loadRawRegion(const_cast<char *>(P.inimg_file.toStdString().c_str()), total1dData, in_zz, in_sz,datatype,start_x,start_y,tileLocation.z,
                           end_x,end_y,tileLocation.z + P.in_sz[2]))
        {
            printf("can not load the region");
            if(total1dData) {delete []total1dData; total1dData = 0;}
            return false;
        }
    }

    Image4DSimple* total4DImage = new Image4DSimple;
    total4DImage->setData((unsigned char*)total1dData, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
    total4DImage->setOriginX(start_x);
    total4DImage->setOriginY(start_y);
    total4DImage->setOriginZ(tileLocation.z);

    V3DLONG mysz[4];
    mysz[0] = total4DImage->getXDim();
    mysz[1] = total4DImage->getYDim();
    mysz[2] = total4DImage->getZDim();
    mysz[3] = total4DImage->getCDim();

    imageSaveString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y).append(".v3draw"));

    QString scanDataFileString = saveDirString;
    scanDataFileString.append("/").append("scanData.txt");
    QString swcString = saveDirString;
    swcString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".swc");

    PARA_APP1 p1;
    PARA_APP2 p2;
    QString versionStr = "v0.001";

    if(P.visible_thresh)
    {
        p1.bkg_thresh = P.bkg_thresh;
        p1.channel = P.channel-1;
        p1.b_256cube = P.b_256cube;
        p1.visible_thresh = P.visible_thresh;

        p1.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p1.p4dImage = total4DImage;
        p1.xc0 = p1.yc0 = p1.zc0 = 0;
        p1.xc1 = p1.p4dImage->getXDim()-1;
        p1.yc1 = p1.p4dImage->getYDim()-1;
        p1.zc1 = p1.p4dImage->getZDim()-1;
        qDebug()<<"starting app1";
    }
    else
    {
        p2.is_gsdt = P.is_gsdt;
        p2.is_coverage_prune = true;
        p2.is_break_accept = false;
        p2.bkg_thresh = P.bkg_thresh;
        p2.length_thresh = P.length_thresh;
        p2.cnn_type = 2;
        p2.channel = 0;
        p2.SR_ratio = 3.0/9.9;
        p2.b_256cube = P.b_256cube;
        p2.b_RadiusFrom2D = true;
        p2.b_resample = 1;
        p2.b_intensity = 0;
        p2.b_brightfiled = 0;
        p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p2.p4dImage = total4DImage;
        p2.xc0 = p2.yc0 = p2.zc0 = 0;
        p2.xc1 = p2.p4dImage->getXDim()-1;
        p2.yc1 = p2.p4dImage->getYDim()-1;
        p2.zc1 = p2.p4dImage->getZDim()-1;
    }

    NeuronTree nt;
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

    simple_saveimage_wrapper(callback, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, total4DImage->getDatatype());

    QString finaloutputswc = P.inimg_file + ("_nc_app2_adp.swc");
    ifstream ifs_swc(finaloutputswc.toStdString().c_str());
    vector<MyMarker*> finalswc;

    if(ifs_swc)
       finalswc = readSWC_file(finaloutputswc.toStdString());

    if(P.visible_thresh)
        qDebug()<<"starting app1";
    else
        qDebug()<<"starting app2";
    qDebug()<<"rootlist size "<<QString::number(inputRootList.size());

    vector<MyMarker*> tileswc_file;

    if(inputRootList.size() <1)
    {
        if(P.visible_thresh)
        {
            p1.outswc_file =swcString;
            proc_app1(callback, p1, versionStr);
        }
        else
        {
            p2.outswc_file =swcString;
            proc_app2(callback, p2, versionStr);
        }
    }
    else
    {
        for(int i = 0; i < inputRootList.size(); i++)
        {
            QString poutswc_file = swcString + (QString::number(i)) + (".swc");
            if(P.visible_thresh)
                p1.outswc_file =poutswc_file;
            else
                p2.outswc_file =poutswc_file;

            bool flag = false;
            LocationSimple RootNewLocation;
            RootNewLocation.x = inputRootList.at(i).x - total4DImage->getOriginX();
            RootNewLocation.y = inputRootList.at(i).y - total4DImage->getOriginY();
            RootNewLocation.z = inputRootList.at(i).z - total4DImage->getOriginZ();

            if(tileswc_file.size()>0)
            {
                for(V3DLONG dd = 0; dd < tileswc_file.size();dd++)
                {
                    double dis = sqrt(pow2(RootNewLocation.x - tileswc_file.at(dd)->x) + pow2(RootNewLocation.y - tileswc_file.at(dd)->y) + pow2(RootNewLocation.z - tileswc_file.at(dd)->z));
                    if(dis < 10.0)
                    {
                        flag = true;
                        break;
                    }
                }
            }

            if(!flag)
            {
                if(P.visible_thresh)
                {
                    p1.landmarks.push_back(RootNewLocation);
                    proc_app1(callback, p1, versionStr);
                    p1.landmarks.clear();
                }else
                {
                    p2.landmarks.push_back(RootNewLocation);
                    proc_app2(callback, p2, versionStr);
                    p2.landmarks.clear();
                }

                vector<MyMarker*> inputswc = readSWC_file(poutswc_file.toStdString());

                for(V3DLONG d = 0; d < inputswc.size(); d++)
                {
                    tileswc_file.push_back(inputswc[d]);
                }
            }
        }
        saveSWC_file(swcString.toStdString().c_str(), tileswc_file);
        nt = readSWC_file(swcString);
    }

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
            bool check_tip = false;
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                V3DLONG node_pn = getParent(i,nt);
                V3DLONG node_pn_2nd;
                if( list.at(node_pn).pn < 0)
                {
                    node_pn_2nd = node_pn;
                }
                else
                {
                    node_pn_2nd = getParent(node_pn,nt);
                }

                newTip.x = list.at(node_pn_2nd).x + total4DImage->getOriginX();
                newTip.y = list.at(node_pn_2nd).y + total4DImage->getOriginY();
                newTip.z = list.at(node_pn_2nd).z + total4DImage->getOriginZ();

//                newTip.x = curr.x + total4DImage->getOriginX();
//                newTip.y = curr.y + total4DImage->getOriginY();
//                newTip.z = curr.z + total4DImage->getOriginZ();

                for(V3DLONG j = 0; j < finalswc.size(); j++ )
                {
                    double dis = sqrt(pow2(newTip.x - finalswc.at(j)->x) + pow2(newTip.y - finalswc.at(j)->y) + pow2(newTip.z - finalswc.at(j)->z));
                    if(dis < 2*finalswc.at(j)->radius || dis < 20)
                    {
                        check_tip = true;
                        break;
                    }
                }
            }
            if(check_tip) continue;
            if( curr.x < 0.05* total4DImage->getXDim())
            {
                tip_left.push_back(newTip);
            }else if (curr.x > 0.95 * total4DImage->getXDim())
            {
                tip_right.push_back(newTip);
            }else if (curr.y < 0.05 * total4DImage->getYDim())
            {
                tip_up.push_back(newTip);
            }else if (curr.y > 0.95*total4DImage->getYDim())
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
        float min_y = INF, max_y = 0;
        for(int i = 0; i<tip_left.size();i++)
        {
            if(tip_left.at(i).y <= min_y) min_y = tip_left.at(i).y;
            if(tip_left.at(i).y >= max_y) max_y = tip_left.at(i).y;
        }
        double block_size = (max_y - min_y)*1.2;
        if(block_size <= 256) block_size = 256;
        if(block_size >= P.block_size) block_size = P.block_size;
        newTarget.x = -floor(block_size*(1.0-overlap)) + tileLocation.x;
        newTarget.y = floor((min_y + max_y - block_size)/2 - total4DImage->getOriginY()) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTarget.radius = block_size;
        newTargetList->push_back(newTarget);
   }
    if(tip_right.size()>0)
    {
        newTipsList->push_back(tip_right);
        float min_y = INF, max_y = 0;
        for(int i = 0; i<tip_right.size();i++)
        {
            if(tip_right.at(i).y <= min_y) min_y = tip_right.at(i).y;
            if(tip_right.at(i).y >= max_y) max_y = tip_right.at(i).y;
        }
        double block_size = (max_y - min_y)*1.2;
        if(block_size <= 256) block_size = 256;
        if(block_size >= P.block_size) block_size = P.block_size;
        newTarget.x = tileLocation.x + tileLocation.radius - floor(block_size*overlap);
        newTarget.y = floor((min_y + max_y - block_size)/2 - total4DImage->getOriginY()) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTarget.radius = block_size;
        newTargetList->push_back(newTarget);
    }
    if(tip_up.size()>0)
    {
        newTipsList->push_back(tip_up);
        float min_x = INF, max_x = 0;
        for(int i = 0; i<tip_up.size();i++)
        {
            if(tip_up.at(i).x <= min_x) min_x = tip_up.at(i).x;
            if(tip_up.at(i).x >= max_x) max_x = tip_up.at(i).x;
        }
        double block_size = (max_x - min_x)*1.2;
        if(block_size <= 256) block_size = 256;
        if(block_size >= P.block_size) block_size = P.block_size;
        newTarget.x = floor((min_x + max_x - block_size)/2) - total4DImage->getOriginX() + tileLocation.x;
        newTarget.y = -floor(block_size*(1.0-overlap)) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTarget.radius = block_size;
        newTargetList->push_back(newTarget);
    }
    if(tip_down.size()>0)
    {
        newTipsList->push_back(tip_down);
        float min_x = INF, max_x = 0;
        for(int i = 0; i<tip_down.size();i++)
        {
            if(tip_down.at(i).x <= min_x) min_x = tip_down.at(i).x;
            if(tip_down.at(i).x >= max_x) max_x = tip_down.at(i).x;
        }
        double block_size = (max_x - min_x)*1.2;
        if(block_size <= 256) block_size = 256;
        if(block_size >= P.block_size) block_size = P.block_size;

        newTarget.x = floor((min_x + max_x - block_size)/2) - total4DImage->getOriginX() + tileLocation.x;
        newTarget.y = tileLocation.y + tileLocation.radius - floor(block_size*overlap);
        newTarget.z = total4DImage->getOriginZ();
        newTarget.radius = block_size;
        newTargetList->push_back(newTarget);
    }

    if(ifs_swc)
    {
        for(V3DLONG i = 0; i < tileswc_file.size(); i++)
        {
            tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
            tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
            tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();

            finalswc.push_back(tileswc_file[i]);
        }
        saveSWC_file(finaloutputswc.toStdString().c_str(), finalswc);
    }
    else
    {
        for(V3DLONG i = 0; i < tileswc_file.size(); i++)
        {
            tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
            tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
            tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();
        }
        saveSWC_file(finaloutputswc.toStdString().c_str(), tileswc_file);
    }

    total4DImage->deleteRawDataAndSetPointerToNull();

    return true;
}

void processSmartScan(V3DPluginCallback2 &callback, list<string> & infostring, QString fileWithData)
{
    ifstream ifs(fileWithData.toLatin1());
    string info_swc;
    int offsetX, offsetY;
    string swcfilepath;
    vector<MyMarker*> outswc;
    int node_type = 1;
    int offsetX_min = 10000000,offsetY_min = 10000000,offsetX_max = -10000000,offsetY_max =-10000000;
    int origin_x,origin_y;

    QString folderpath = QFileInfo(fileWithData).absolutePath();
    V3DLONG in_sz[4];

    while(ifs && getline(ifs, info_swc))
    {
        std::istringstream iss(info_swc);
        iss >> offsetX >> offsetY >> swcfilepath;
        if(offsetX < offsetX_min) offsetX_min = offsetX;
        if(offsetY < offsetY_min) offsetY_min = offsetY;
        if(offsetX > offsetX_max) offsetX_max = offsetX;
        if(offsetY > offsetY_max) offsetY_max = offsetY;

        if(node_type == 1)
        {
            origin_x = offsetX;
            origin_y = offsetY;

            QString firstImagepath = folderpath + "/" +   QFileInfo(QString::fromStdString(swcfilepath)).baseName().append(".v3draw");
            unsigned char * data1d = 0;
            int datatype;
            if(!simple_loadimage_wrapper(callback, firstImagepath.toStdString().c_str(), data1d, in_sz, datatype))
            {
                cerr<<"load image "<<firstImagepath.toStdString()<<" error!"<<endl;
                return;
            }
            if(data1d) {delete []data1d; data1d=0;}

        }
        vector<MyMarker*> inputswc = readSWC_file(swcfilepath);;
        for(V3DLONG d = 0; d < inputswc.size(); d++)
        {
            inputswc[d]->x = inputswc[d]->x + offsetX;
            inputswc[d]->y = inputswc[d]->y + offsetY;
            inputswc[d]->type = node_type;
            outswc.push_back(inputswc[d]);
        }
        node_type++;
    }
    ifs.close();


    QString fileSaveName = fileWithData + ".swc";
    for(V3DLONG i = 0; i < outswc.size(); i++)
    {
        outswc[i]->x = outswc[i]->x - offsetX_min;
        outswc[i]->y = outswc[i]->y - offsetY_min;
    }

    //export_list2file(outswc, fileSaveName,QString::fromStdString(swcfilepath));

    saveSWC_file(fileSaveName.toStdString().c_str(), outswc,infostring);

    //write tc file

    QString tc_name = fileWithData + ".tc";
    ofstream myfile;
    myfile.open(tc_name.toStdString().c_str(), ios::in);
    if (myfile.is_open()==true)
    {
        qDebug()<<"initial file can be opened for reading and will be removed";

        myfile.close();
        remove(tc_name.toStdString().c_str());
    }
    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
    if (!myfile.is_open()){
        qDebug()<<"file's not really open...";
        return;
    }

    myfile << "# thumbnail file \n";
    myfile << "NULL \n\n";
    myfile << "# tiles \n";
    myfile << node_type-1 << " \n\n";
    myfile << "# dimensions (XYZC) \n";
    myfile << in_sz[0] + offsetX_max - offsetX_min << " " << in_sz[1] + offsetY_max - offsetY_min << " " << in_sz[2] << " " << 1 << " ";
    myfile << "\n\n";
    myfile << "# origin (XYZ) \n";
    myfile << "0.000000 0.000000 0.000000 \n\n";
    myfile << "# resolution (XYZ) \n";
    myfile << "1.000000 1.000000 1.000000 \n\n";
    myfile << "# image coordinates look up table \n";

    ifstream ifs_2nd(fileWithData.toLatin1());
    while(ifs_2nd && getline(ifs_2nd, info_swc))
    {
        std::istringstream iss(info_swc);
        iss >> offsetX >> offsetY >> swcfilepath;
        QString imagename= QFileInfo(QString::fromStdString(swcfilepath)).completeBaseName() + ".v3draw";
        QString imagefilepath= folderpath + "/" + imagename;


        unsigned char * data1d = 0;
        int datatype;
        if(!simple_loadimage_wrapper(callback, imagefilepath.toStdString().c_str(), data1d, in_sz, datatype))
        {
            cerr<<"load image "<<imagefilepath.toStdString()<<" error!"<<endl;
            return;
        }
        if(data1d) {delete []data1d; data1d=0;}
        imagename.append(QString("   ( %1, %2, 0) ( %3, %4, %5)").arg(offsetX - origin_x).arg(offsetY- origin_y).arg(in_sz[0]-1 + offsetX - origin_x).arg(in_sz[1]-1 + offsetY - origin_y).arg(in_sz[2]-1));
        myfile << imagename.toStdString();
        myfile << "\n";
    }
    myfile.flush();
    myfile.close();
    ifs_2nd.close();
}

bool crawler_raw_all(V3DPluginCallback2 &callback, QWidget *parent,ALL_LS_PARA &P,bool bmenu, int method)
{
    QElapsedTimer timer1;
    timer1.start();

    QString fileOpenName = P.inimg_file;

    if(P.image)
    {
        P.in_sz[0] = P.image->getXDim();
        P.in_sz[1] = P.image->getYDim();
        P.in_sz[2] = P.image->getZDim();
    }else
    {
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
        LocationSimple t;
        for(int i = 0; i < file_inmarkers.size(); i++)
        {
            t.x = file_inmarkers[i].x + 1;
            t.y = file_inmarkers[i].y + 1;
            t.z = file_inmarkers[i].z + 1;
            P.listLandmarks.push_back(t);
        }
    }

    LandmarkList allTargetList;
    QList<LandmarkList> allTipsList;

    LocationSimple tileLocation;
    tileLocation.x = P.listLandmarks[0].x;
    tileLocation.y = P.listLandmarks[0].y;
    tileLocation.z = P.listLandmarks[0].z;

    LandmarkList inputRootList;
    inputRootList.push_back(tileLocation);
    allTipsList.push_back(inputRootList);

    tileLocation.x = tileLocation.x -int(P.block_size/2);
    tileLocation.y = tileLocation.y -int(P.block_size/2);
    tileLocation.z = 0;
    tileLocation.radius = P.block_size;

    allTargetList.push_back(tileLocation);

    QString tmpfolder;
    if(method ==1)
       tmpfolder = QFileInfo(fileOpenName).path()+("/tmp_NEUTUBE");
    else if(method ==2)
       tmpfolder = QFileInfo(fileOpenName).path()+("/tmp_SNAKE");
    else if(method ==3)
       tmpfolder = QFileInfo(fileOpenName).path()+("/tmp_MOST");

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
        if(P.adap_win)
            all_tracing_ada_win(callback,P,allTipsList.at(0),allTargetList.at(0),&newTargetList,&newTipsList,method);
        else
            all_tracing(callback,P,allTipsList.at(0),allTargetList.at(0),&newTargetList,&newTipsList,method);
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
    qint64 etime1 = timer1.elapsed();

    list<string> infostring;
    string tmpstr; QString qtstr;
    if(method ==1)
    {
        tmpstr =  qPrintable( qtstr.prepend("## NeuronCrawler_NEUTUBE")); infostring.push_back(tmpstr);
    }else if(method ==2)
    {
        tmpstr =  qPrintable( qtstr.prepend("## NeuronCrawler_SNAKE")); infostring.push_back(tmpstr);
    }else if(method ==3)
    {
        tmpstr =  qPrintable( qtstr.prepend("## NeuronCrawler_MOST")); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.channel).prepend("#channel = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);

        tmpstr =  qPrintable( qtstr.setNum(P.seed_win).prepend("#seed_win = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(P.slip_win).prepend("#slip_win = ") ); infostring.push_back(tmpstr);
    }

    tmpstr =  qPrintable( qtstr.setNum(P.block_size).prepend("#block_size = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(etime1).prepend("#neuron preprocessing time (milliseconds) = ") ); infostring.push_back(tmpstr);


    processSmartScan(callback,infostring,tmpfolder +"/scanData.txt");


    v3d_msg(QString("The tracing uses %1 for tracing. Now you can drag and drop the generated swc fle [%2] into Vaa3D."
                    ).arg(etime1).arg(tmpfolder +"/scanData.txt.swc"), 1);

    return true;
}

bool all_tracing(V3DPluginCallback2 &callback,ALL_LS_PARA &P,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList, int method)
{

    QString saveDirString;
    if(method ==1)
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_NEUTUBE");
    else if (method ==2)
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_SNAKE");
    else if (method ==3)
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_MOST");

    QString imageSaveString = saveDirString;

    V3DLONG start_x,start_y,end_x,end_y;
    start_x = (tileLocation.x < 0)?  0 : tileLocation.x;
    start_y = (tileLocation.y < 0)?  0 : tileLocation.y;

    end_x = tileLocation.x+P.block_size;
    end_y = tileLocation.y+P.block_size;
    if(end_x > P.in_sz[0]) end_x = P.in_sz[0];
    if(end_y > P.in_sz[1]) end_y = P.in_sz[1];

    if(tileLocation.x >= P.in_sz[0] - 1 || tileLocation.y >= P.in_sz[1] - 1 || end_x <= 0 || end_y <= 0 )
    {
        printf("hit the boundary");
        return true;
    }

    unsigned char * total1dData = 0;
    V3DLONG *in_sz = 0;

    if(P.image)
    {
        in_sz = new V3DLONG[4];
        in_sz[0] = end_x - start_x;
        in_sz[1] = end_y - start_y;
        in_sz[2] = P.in_sz[2];
        V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
        try {total1dData = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for loading the region.",0); return false;}
        V3DLONG i = 0;
        for(V3DLONG iz = 0; iz < P.in_sz[2]; iz++)
        {
            V3DLONG offsetk = iz*P.in_sz[1]*P.in_sz[0];
            for(V3DLONG iy = start_y; iy < end_y; iy++)
            {
                V3DLONG offsetj = iy*P.in_sz[0];
                for(V3DLONG ix = start_x; ix < end_x; ix++)
                {
                    total1dData[i] = P.image->getRawData()[offsetk + offsetj + ix];
                    i++;
                }
            }
        }
    }else
    {
        V3DLONG *in_zz = 0;
        int datatype;
        if (!loadRawRegion(const_cast<char *>(P.inimg_file.toStdString().c_str()), total1dData, in_zz, in_sz,datatype,start_x,start_y,tileLocation.z,
                           end_x,end_y,tileLocation.z + P.in_sz[2]))
        {
            printf("can not load the region");
            if(total1dData) {delete []total1dData; total1dData = 0;}
            return false;
        }
    }

    Image4DSimple* total4DImage = new Image4DSimple;
    total4DImage->setData((unsigned char*)total1dData, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
    total4DImage->setOriginX(start_x);
    total4DImage->setOriginY(start_y);
    total4DImage->setOriginZ(tileLocation.z);

    V3DLONG mysz[4];
    mysz[0] = total4DImage->getXDim();
    mysz[1] = total4DImage->getYDim();
    mysz[2] = total4DImage->getZDim();
    mysz[3] = total4DImage->getCDim();

    imageSaveString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y).append(".v3draw"));

    QString scanDataFileString = saveDirString;
    scanDataFileString.append("/").append("scanData.txt");
    QString swcString = saveDirString;
    swcString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".swc");

    ifstream ifs_image(imageSaveString.toStdString().c_str());
    if(!ifs_image)
    {
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

        simple_saveimage_wrapper(callback, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, total4DImage->getDatatype());

        V3DPluginArgItem arg;
        V3DPluginArgList input;
        V3DPluginArgList output;

        QString full_plugin_name;
        QString func_name;

        arg.type = "random";std::vector<char*> arg_input;
        std:: string fileName_Qstring(imageSaveString.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
        arg_input.push_back(fileName_string);
        arg.p = (void *) & arg_input; input<< arg;

        char* char_swcout =  new char[swcString.length() + 1];strcpy(char_swcout, swcString.toStdString().c_str());
        arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(char_swcout); arg.p = (void *) & arg_output; output<< arg;

        arg.type = "random";
        std::vector<char*> arg_para;

        if(method ==1)
        {
            arg_para.push_back("1");
            arg_para.push_back("1");
            full_plugin_name = "neuTube";
            func_name =  "neutube_trace";
        }else if(method ==2)
        {
            arg_para.push_back("1");
            arg_para.push_back("1");
            full_plugin_name = "snake";
            func_name =  "snake_trace";
        }else if(method ==3)
        {
            string S_channel = boost::lexical_cast<string>(P.channel);
            char* C_channel = new char[S_channel.length() + 1];
            strcpy(C_channel,S_channel.c_str());
            arg_para.push_back(C_channel);

            string S_background_th = boost::lexical_cast<string>(P.bkg_thresh);
            char* C_background_th = new char[S_background_th.length() + 1];
            strcpy(C_background_th,S_background_th.c_str());
            arg_para.push_back(C_background_th);

            string S_seed_win = boost::lexical_cast<string>(P.seed_win);
            char* C_seed_win = new char[S_seed_win.length() + 1];
            strcpy(C_seed_win,S_seed_win.c_str());
            arg_para.push_back(C_seed_win);

            string S_slip_win = boost::lexical_cast<string>(P.slip_win);
            char* C_slip_win = new char[S_slip_win.length() + 1];
            strcpy(C_slip_win,S_slip_win.c_str());
            arg_para.push_back(C_slip_win);

            full_plugin_name = "mostVesselTracer";
            func_name =  "MOST_trace";
        }

        arg.p = (void *) & arg_para; input << arg;

        if(!callback.callPluginFunc(full_plugin_name,func_name,input,output))
        {

            printf("Can not find the tracing plugin!\n");
            return false;
        }
    }


    NeuronTree nt_neutube;
    QString swcNEUTUBE = saveDirString;
    if(method ==1)
        swcNEUTUBE.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".v3draw_neutube.swc");
    else if (method ==2)
        swcNEUTUBE.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".v3draw_snake.swc");
    else if (method ==3)
        swcNEUTUBE.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".v3draw_MOST.swc");

    nt_neutube = readSWC_file(swcNEUTUBE);

    NeuronTree nt;
    ifstream ifs_swcString(swcString.toStdString().c_str());
    if(!ifs_swcString)
    {
        nt = sort_eliminate_swc(nt_neutube,inputRootList,total4DImage);
        export_list2file(nt.listNeuron, swcString,swcNEUTUBE);

    }else
    {
        NeuronTree nt_tile = readSWC_file(swcString);
        LandmarkList inputRootList_pruned = eliminate_seed(nt_tile,inputRootList,total4DImage);
        if(inputRootList_pruned.size()<1)
            return true;
        else
        {
            nt = sort_eliminate_swc(nt_neutube,inputRootList_pruned,total4DImage);
            combine_list2file(nt.listNeuron, swcString);

        }
    }

    LandmarkList tip_left;
    LandmarkList tip_right;
    LandmarkList tip_up ;
    LandmarkList tip_down;
    QList<NeuronSWC> list = nt.listNeuron;
    for (V3DLONG i=0;i<list.size();i++)
    {
            NeuronSWC curr = list.at(i);
            LocationSimple newTip;
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                newTip.x = curr.x + total4DImage->getOriginX();
                newTip.y = curr.y + total4DImage->getOriginY();
                newTip.z = curr.z + total4DImage->getOriginZ();
            }
            if( curr.x < 0.05* total4DImage->getXDim())
            {
                tip_left.push_back(newTip);
            }else if (curr.x > 0.95 * total4DImage->getXDim())
            {
                tip_right.push_back(newTip);
            }else if (curr.y < 0.05 * total4DImage->getYDim())
            {
                tip_up.push_back(newTip);
            }else if (curr.y > 0.95*total4DImage->getYDim())
            {
                tip_down.push_back(newTip);
            }
    }

    double overlap = 0.1;
    LocationSimple newTarget;
    if(tip_left.size()>0)
    {
        newTipsList->push_back(tip_left);
        newTarget.x = -floor(P.block_size*(1.0-overlap)) + tileLocation.x;
        newTarget.y = total4DImage->getOriginY();
        newTarget.z = total4DImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_right.size()>0)
    {
        newTipsList->push_back(tip_right);
        newTarget.x = floor(P.block_size*(1.0-overlap)) + tileLocation.x;
        newTarget.y = total4DImage->getOriginY();
        newTarget.z = total4DImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_up.size()>0)
    {
        newTipsList->push_back(tip_up);
        newTarget.x = total4DImage->getOriginX();
        newTarget.y = -floor(P.block_size*(1.0-overlap)) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_down.size()>0)
    {
        newTipsList->push_back(tip_down);
        newTarget.x = total4DImage->getOriginX();
        newTarget.y = floor(P.block_size*(1.0-overlap)) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }

    total4DImage->deleteRawDataAndSetPointerToNull();
    return true;
}

bool all_tracing_ada_win(V3DPluginCallback2 &callback,ALL_LS_PARA &P,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList, int method)
{

    QString saveDirString;
    if(method ==1)
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_NEUTUBE");
    else if (method ==2)
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_SNAKE");
    else if (method ==3)
        saveDirString = QFileInfo(P.inimg_file).path().append("/tmp_MOST");

    QString imageSaveString = saveDirString;

    V3DLONG start_x,start_y,end_x,end_y;
    start_x = (tileLocation.x < 0)?  0 : tileLocation.x;
    start_y = (tileLocation.y < 0)?  0 : tileLocation.y;

    end_x = tileLocation.x+tileLocation.radius;
    end_y = tileLocation.y+tileLocation.radius;
    if(end_x > P.in_sz[0]) end_x = P.in_sz[0];
    if(end_y > P.in_sz[1]) end_y = P.in_sz[1];

    if(tileLocation.x >= P.in_sz[0] - 1 || tileLocation.y >= P.in_sz[1] - 1 || end_x <= 0 || end_y <= 0 )
    {
        printf("hit the boundary");
        return true;
    }

    unsigned char * total1dData = 0;
    V3DLONG *in_sz = 0;

    if(P.image)
    {
        in_sz = new V3DLONG[4];
        in_sz[0] = end_x - start_x;
        in_sz[1] = end_y - start_y;
        in_sz[2] = P.in_sz[2];
        V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
        try {total1dData = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for loading the region.",0); return false;}
        V3DLONG i = 0;
        for(V3DLONG iz = 0; iz < P.in_sz[2]; iz++)
        {
            V3DLONG offsetk = iz*P.in_sz[1]*P.in_sz[0];
            for(V3DLONG iy = start_y; iy < end_y; iy++)
            {
                V3DLONG offsetj = iy*P.in_sz[0];
                for(V3DLONG ix = start_x; ix < end_x; ix++)
                {
                    total1dData[i] = P.image->getRawData()[offsetk + offsetj + ix];
                    i++;
                }
            }
        }
    }else
    {
        V3DLONG *in_zz = 0;
        int datatype;
        if (!loadRawRegion(const_cast<char *>(P.inimg_file.toStdString().c_str()), total1dData, in_zz, in_sz,datatype,start_x,start_y,tileLocation.z,
                           end_x,end_y,tileLocation.z + P.in_sz[2]))
        {
            printf("can not load the region");
            if(total1dData) {delete []total1dData; total1dData = 0;}
            return false;
        }
    }

    Image4DSimple* total4DImage = new Image4DSimple;
    total4DImage->setData((unsigned char*)total1dData, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
    total4DImage->setOriginX(start_x);
    total4DImage->setOriginY(start_y);
    total4DImage->setOriginZ(tileLocation.z);

    V3DLONG mysz[4];
    mysz[0] = total4DImage->getXDim();
    mysz[1] = total4DImage->getYDim();
    mysz[2] = total4DImage->getZDim();
    mysz[3] = total4DImage->getCDim();

    imageSaveString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y).append(".v3draw"));

    QString scanDataFileString = saveDirString;
    scanDataFileString.append("/").append("scanData.txt");
    QString swcString = saveDirString;
    swcString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".swc");

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

    simple_saveimage_wrapper(callback, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, total4DImage->getDatatype());


    QString finaloutputswc = P.inimg_file + ("_nc_neutube_adp.swc");
    ifstream ifs_swc(finaloutputswc.toStdString().c_str());
    vector<MyMarker*> finalswc;

    if(ifs_swc)
       finalswc = readSWC_file(finaloutputswc.toStdString());

    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    QString full_plugin_name;
    QString func_name;

    arg.type = "random";std::vector<char*> arg_input;
    std:: string fileName_Qstring(imageSaveString.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
    arg_input.push_back(fileName_string);
    arg.p = (void *) & arg_input; input<< arg;

    char* char_swcout =  new char[swcString.length() + 1];strcpy(char_swcout, swcString.toStdString().c_str());
    arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(char_swcout); arg.p = (void *) & arg_output; output<< arg;

    arg.type = "random";
    std::vector<char*> arg_para;

    if(method ==1)
    {
        arg_para.push_back("1");
        arg_para.push_back("1");
        full_plugin_name = "neuTube";
        func_name =  "neutube_trace";
    }else if(method ==2)
    {
        arg_para.push_back("1");
        arg_para.push_back("1");
        full_plugin_name = "snake";
        func_name =  "snake_trace";
    }else if(method ==3)
    {
        string S_channel = boost::lexical_cast<string>(P.channel);
        char* C_channel = new char[S_channel.length() + 1];
        strcpy(C_channel,S_channel.c_str());
        arg_para.push_back(C_channel);

        string S_background_th = boost::lexical_cast<string>(P.bkg_thresh);
        char* C_background_th = new char[S_background_th.length() + 1];
        strcpy(C_background_th,S_background_th.c_str());
        arg_para.push_back(C_background_th);

        string S_seed_win = boost::lexical_cast<string>(P.seed_win);
        char* C_seed_win = new char[S_seed_win.length() + 1];
        strcpy(C_seed_win,S_seed_win.c_str());
        arg_para.push_back(C_seed_win);

        string S_slip_win = boost::lexical_cast<string>(P.slip_win);
        char* C_slip_win = new char[S_slip_win.length() + 1];
        strcpy(C_slip_win,S_slip_win.c_str());
        arg_para.push_back(C_slip_win);

        full_plugin_name = "mostVesselTracer";
        func_name =  "MOST_trace";
    }

    arg.p = (void *) & arg_para; input << arg;

    if(!callback.callPluginFunc(full_plugin_name,func_name,input,output))
    {

        printf("Can not find the tracing plugin!\n");
        return false;
    }


    NeuronTree nt_neutube;
    QString swcNEUTUBE = saveDirString;
    if(method ==1)
        swcNEUTUBE.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".v3draw_neutube.swc");
    else if (method ==2)
        swcNEUTUBE.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".v3draw_snake.swc");
    else if (method ==3)
        swcNEUTUBE.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append(".v3draw_MOST.swc");

    nt_neutube = readSWC_file(swcNEUTUBE);

    NeuronTree nt;
    ifstream ifs_swcString(swcString.toStdString().c_str());
    if(!ifs_swcString)
    {
        nt = sort_eliminate_swc(nt_neutube,inputRootList,total4DImage);
        export_list2file(nt.listNeuron, swcString,swcNEUTUBE);

    }else
    {
        NeuronTree nt_tile = readSWC_file(swcString);
        LandmarkList inputRootList_pruned = eliminate_seed(nt_tile,inputRootList,total4DImage);
        if(inputRootList_pruned.size()<1)
            return true;
        else
        {
            nt = sort_eliminate_swc(nt_neutube,inputRootList_pruned,total4DImage);
            combine_list2file(nt.listNeuron, swcString);

        }
    }

    LandmarkList tip_left;
    LandmarkList tip_right;
    LandmarkList tip_up ;
    LandmarkList tip_down;
    QList<NeuronSWC> list = nt.listNeuron;
    for (V3DLONG i=0;i<list.size();i++)
    {
            NeuronSWC curr = list.at(i);
            LocationSimple newTip;
            bool check_tip = false;

            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                newTip.x = curr.x + total4DImage->getOriginX();
                newTip.y = curr.y + total4DImage->getOriginY();
                newTip.z = curr.z + total4DImage->getOriginZ();

                for(V3DLONG j = 0; j < finalswc.size(); j++ )
                {
                    double dis = sqrt(pow2(newTip.x - finalswc.at(j)->x) + pow2(newTip.y - finalswc.at(j)->y) + pow2(newTip.z - finalswc.at(j)->z));
                    if(dis < 20)
                    {
                        check_tip = true;
                        break;
                    }
                }
            }
            if(check_tip) continue;
            if( curr.x < 0.05* total4DImage->getXDim())
            {
                tip_left.push_back(newTip);
            }else if (curr.x > 0.95 * total4DImage->getXDim())
            {
                tip_right.push_back(newTip);
            }else if (curr.y < 0.05 * total4DImage->getYDim())
            {
                tip_up.push_back(newTip);
            }else if (curr.y > 0.95*total4DImage->getYDim())
            {
                tip_down.push_back(newTip);
            }
    }

    double overlap = 0.1;
    LocationSimple newTarget;
    if(tip_left.size()>0)
    {
        newTipsList->push_back(tip_left);
        float min_y = INF, max_y = 0;
        for(int i = 0; i<tip_left.size();i++)
        {
            if(tip_left.at(i).y <= min_y) min_y = tip_left.at(i).y;
            if(tip_left.at(i).y >= max_y) max_y = tip_left.at(i).y;
        }
        double block_size = (max_y - min_y)*1.2;
        if(block_size <= 256) block_size = 256;
        if(block_size >= P.block_size) block_size = P.block_size;
        newTarget.x = -floor(block_size*(1.0-overlap)) + tileLocation.x;
        newTarget.y = floor((min_y + max_y - block_size)/2 - total4DImage->getOriginY()) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTarget.radius = block_size;
        newTargetList->push_back(newTarget);
   }
    if(tip_right.size()>0)
    {
        newTipsList->push_back(tip_right);
        float min_y = INF, max_y = 0;
        for(int i = 0; i<tip_right.size();i++)
        {
            if(tip_right.at(i).y <= min_y) min_y = tip_right.at(i).y;
            if(tip_right.at(i).y >= max_y) max_y = tip_right.at(i).y;
        }
        double block_size = (max_y - min_y)*1.2;
        if(block_size <= 256) block_size = 256;
        if(block_size >= P.block_size) block_size = P.block_size;
        newTarget.x = tileLocation.x + tileLocation.radius - floor(block_size*overlap);
        newTarget.y = floor((min_y + max_y - block_size)/2 - total4DImage->getOriginY()) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTarget.radius = block_size;
        newTargetList->push_back(newTarget);
    }
    if(tip_up.size()>0)
    {
        newTipsList->push_back(tip_up);
        float min_x = INF, max_x = 0;
        for(int i = 0; i<tip_up.size();i++)
        {
            if(tip_up.at(i).x <= min_x) min_x = tip_up.at(i).x;
            if(tip_up.at(i).x >= max_x) max_x = tip_up.at(i).x;
        }
        double block_size = (max_x - min_x)*1.2;
        if(block_size <= 256) block_size = 256;
        if(block_size >= P.block_size) block_size = P.block_size;
        newTarget.x = floor((min_x + max_x - block_size)/2) - total4DImage->getOriginX() + tileLocation.x;
        newTarget.y = -floor(block_size*(1.0-overlap)) + tileLocation.y;
        newTarget.z = total4DImage->getOriginZ();
        newTarget.radius = block_size;
        newTargetList->push_back(newTarget);
    }
    if(tip_down.size()>0)
    {
        newTipsList->push_back(tip_down);
        float min_x = INF, max_x = 0;
        for(int i = 0; i<tip_down.size();i++)
        {
            if(tip_down.at(i).x <= min_x) min_x = tip_down.at(i).x;
            if(tip_down.at(i).x >= max_x) max_x = tip_down.at(i).x;
        }
        double block_size = (max_x - min_x)*1.2;
        if(block_size <= 256) block_size = 256;
        if(block_size >= P.block_size) block_size = P.block_size;

        newTarget.x = floor((min_x + max_x - block_size)/2) - total4DImage->getOriginX() + tileLocation.x;
        newTarget.y = tileLocation.y + tileLocation.radius - floor(block_size*overlap);
        newTarget.z = total4DImage->getOriginZ();
        newTarget.radius = block_size;
        newTargetList->push_back(newTarget);
    }

    vector<MyMarker*> tileswc_file = readSWC_file(swcString.toStdString());

    if(ifs_swc)
    {
        for(V3DLONG i = 0; i < tileswc_file.size(); i++)
        {
            tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
            tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
            tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();

            finalswc.push_back(tileswc_file[i]);
        }
        saveSWC_file(finaloutputswc.toStdString().c_str(), finalswc);
    }
    else
    {
        for(V3DLONG i = 0; i < tileswc_file.size(); i++)
        {
            tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
            tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
            tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();
        }
        saveSWC_file(finaloutputswc.toStdString().c_str(), tileswc_file);
    }


    total4DImage->deleteRawDataAndSetPointerToNull();
    return true;
}


NeuronTree sort_eliminate_swc(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage)
{
    NeuronTree nt_result;
    QList<NeuronSWC> neuron_sorted;

    if (!SortSWC(nt.listNeuron, neuron_sorted,VOID, 10))
    {
        v3d_msg("fail to call swc sorting function.",0);
        return nt_result;
    }

    V3DLONG neuronNum = neuron_sorted.size();
    V3DLONG *flag = new V3DLONG[neuronNum];
    if(inputRootList.size() == 1 && int(inputRootList.at(0).x - total4DImage->getOriginX()) == int(inputRootList.at(0).y - total4DImage->getOriginY()))
    {
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            flag[i] = 1;
        }
    }
    else
    {
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            flag[i] = 0;
        }

        int root_index = -1;
        for(V3DLONG i = 0; i<inputRootList.size();i++)
        {
            int marker_x = inputRootList.at(i).x - total4DImage->getOriginX();
            int marker_y = inputRootList.at(i).y - total4DImage->getOriginY();
            int marker_z = inputRootList.at(i).z - total4DImage->getOriginZ();

            for(V3DLONG j = 0; j<neuronNum;j++)
            {
                NeuronSWC curr = neuron_sorted.at(j);
                if(curr.pn < 0) root_index = j;
                double dis = sqrt(pow2(marker_x - curr.x) + pow2(marker_y - curr.y) + pow2(marker_z - curr.z));

                if(dis < 20 && flag[j] ==0)
                {
                    flag[root_index] = 1;
                    V3DLONG d;
                    for( d = root_index+1; d < neuronNum; d++)
                    {
                        if(neuron_sorted.at(d).pn < 0)
                            break;
                        else
                            flag[d] = 1;

                    }
                    break;
                }

            }
        }
    }

    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    //set node
    NeuronSWC S;
    for (V3DLONG i=0;i<neuron_sorted.size();i++)
    {
        if(flag[i] == 1)
        {
            NeuronSWC curr = neuron_sorted.at(i);
            S.n 	= curr.n;
            S.type 	= curr.type;
            S.x 	= curr.x;
            S.y 	= curr.y;
            S.z 	= curr.z;
            S.r 	= curr.r;
            S.pn 	= curr.pn;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }
    nt_result.n = -1;
    nt_result.on = true;
    nt_result.listNeuron = listNeuron;
    nt_result.hashNeuron = hashNeuron;
    if(flag) {delete[] flag; flag = 0;}

    return nt_result;
}

LandmarkList eliminate_seed(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage)
{
    LandmarkList inputRootList_pruned;
    V3DLONG neuronNum = nt.listNeuron.size();


    for(V3DLONG i = 0; i<inputRootList.size();i++)
    {
        int marker_x = inputRootList.at(i).x - total4DImage->getOriginX();
        int marker_y = inputRootList.at(i).y - total4DImage->getOriginY();
        int marker_z = inputRootList.at(i).z - total4DImage->getOriginZ();

        bool flag = false;
        for(V3DLONG j = 0; j<neuronNum;j++)
        {
            NeuronSWC curr = nt.listNeuron.at(j);
            double dis = sqrt(pow2(marker_x - curr.x) + pow2(marker_y - curr.y) + pow2(marker_z - curr.z));

            if(dis < 20)
            {
                flag = true;
                break;
            }

        }
        if(!flag)
            inputRootList_pruned.push_back(inputRootList.at(i));

    }
    return inputRootList_pruned;
}

bool combine_list2file(QList<NeuronSWC> & lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::Text|QIODevice::Append))
        return false;
    QTextStream myfile(&file);
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};
