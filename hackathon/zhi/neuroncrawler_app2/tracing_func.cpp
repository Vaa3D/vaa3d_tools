#include <v3d_interface.h>
#include "v3d_message.h"
#include "tracing_func.h"
#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"


using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
template <class T> T pow2(T a)
{
    return a*a;

}

bool crawler_raw(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &P,bool bmenu)
{
    QElapsedTimer timer1;
    timer1.start();

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
    tileLocation.z = 0;
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

    processSmartScan(callback, P,tmpfolder +"/scanData.txt",timer1);

    return true;
}


bool app2_tracing(V3DPluginCallback2 &callback,APP2_LS_PARA &P,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList)
{

    QString saveDirString = QFileInfo(P.inimg_file).path().append("/tmp");
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

    imageSaveString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y).append(".v3draw"));

    ifstream ifs_image(imageSaveString.toStdString().c_str());
    if(ifs_image)
    {
        printf("the tile was scanned");
        return true;
    }

    unsigned char * total1dData = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;

    int datatype;
    if (!loadRawRegion(const_cast<char *>(P.inimg_file.toStdString().c_str()), total1dData, in_zz, in_sz,datatype,start_x,start_y,tileLocation.z,
                       end_x,end_y,tileLocation.z + P.in_sz[2]))
    {
        printf("can not load the region");
        if(total1dData) {delete []total1dData; total1dData = 0;}
        return false;
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

    simple_saveimage_wrapper(callback, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, total4DImage->getDatatype());

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
        newTarget.x = -floor(P.block_size*(1.0-overlap)) + tileLocation.x;
        newTarget.y = p.p4dImage->getOriginY();
        newTarget.z = p.p4dImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_right.size()>0)
    {
        newTipsList->push_back(tip_right);
        newTarget.x = floor(P.block_size*(1.0-overlap)) + tileLocation.x;
        newTarget.y = p.p4dImage->getOriginY();
        newTarget.z = p.p4dImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_up.size()>0)
    {
        newTipsList->push_back(tip_up);
        newTarget.x = p.p4dImage->getOriginX();
        newTarget.y = -floor(P.block_size*(1.0-overlap)) + tileLocation.y;
        newTarget.z = p.p4dImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    if(tip_down.size()>0)
    {
        newTipsList->push_back(tip_down);
        newTarget.x = p.p4dImage->getOriginX();
        newTarget.y = floor(P.block_size*(1.0-overlap)) + tileLocation.y;
        newTarget.z = p.p4dImage->getOriginZ();
        newTargetList->push_back(newTarget);
    }
    return true;
}

void processSmartScan(V3DPluginCallback2 &callback, APP2_LS_PARA &P,QString fileWithData,QElapsedTimer timer1)
{
    qint64 etime1 = timer1.elapsed();

    list<string> infostring;
    string tmpstr; QString qtstr;
    tmpstr =  qPrintable( qtstr.prepend("##Vaa3D-Neuron-APP2 for Large-scale ")); infostring.push_back(tmpstr);
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
