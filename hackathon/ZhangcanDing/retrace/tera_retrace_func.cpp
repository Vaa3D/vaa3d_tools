#include <v3d_interface.h>
#include "tera_retrace_func.h"
#include "../../../v3d_main/jba/c++/convert_type2uint8.h"
//#include "profile_swc.h"
#include "vn_app2.h"
#include "app2/my_surf_objs.h"
#include "mean_shift_fun.h"
#include "topology_analysis.h"
#include <math.h>

#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(NTDIS(a,b)*NTDIS(a,c)))*180.0/3.14159265359)


using namespace std;

template <class T>
void BinaryProcess(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d)
{
    V3DLONG i, j,k,n,count;
    double t, temp;

    V3DLONG mCount = iImageHeight * iImageWidth;
    for (i=0; i<iImageLayer; i++)
    {
        for (j=0; j<iImageHeight; j++)
        {
            for (k=0; k<iImageWidth; k++)
            {
                V3DLONG curpos = i * mCount + j*iImageWidth + k;
                V3DLONG curpos1 = i* mCount + j*iImageWidth;
                V3DLONG curpos2 = j* iImageWidth + k;
                temp = 0;
                count = 0;
                for(n =1 ; n <= d  ;n++)
                {
                    if (k>h*n) {temp += apsInput[curpos1 + k-(h*n)]; count++;}
                    if (k+(h*n)< iImageWidth) { temp += apsInput[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += apsInput[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//
                    if (j+(h*n)<iImageHeight) {temp += apsInput[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
                    if (i>(h*n)) {temp += apsInput[(i-(h*n))* mCount + curpos2]; count++;}//
                    if (i+(h*n)< iImageLayer) {temp += apsInput[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
                }
                t =  apsInput[curpos]-temp/(count);
                aspOutput[curpos]= (t > 0)? t : 0;
            }
        }
    }
}
template <class T> T pow2(T a)
{
    return a*a;

}

vector<MyMarker *>nt2markers(NeuronTree nt)
{
    vector<MyMarker *> swc;
    map <int, MyMarker *> marker_map;
    map <MyMarker*, int> parid_map;
    for(int i=0; i<nt.listNeuron.size();i++)
    {
        MyMarker *pmarker = new MyMarker;
        pmarker->x= nt.listNeuron.at(i).x;
        pmarker->y= nt.listNeuron.at(i).y;
        pmarker->z= nt.listNeuron.at(i).z;
        pmarker->type= nt.listNeuron.at(i).type;
        pmarker->radius= nt.listNeuron.at(i).radius;
        marker_map[nt.listNeuron.at(i).n]=pmarker;
        parid_map[pmarker]=nt.listNeuron.at(i).parent;
        //pmarker->parent = nt.listNeuron.at(i).parent;
        swc.push_back(pmarker);
    }
    vector<MyMarker*>::iterator it=swc.begin();
    while (it !=swc.end())
    {
        MyMarker *pmarker= *it;
        pmarker->parent=marker_map[parid_map[pmarker]];
        it++;
    }



    return swc;

}

bool crawler_raw_app(V3DPluginCallback2 &callback, QWidget *parent,TRACE_LS_PARA &P,bool bmenu)
{
    QElapsedTimer timer1;
    timer1.start();

    P.listLandmarks.clear();

    QString fileOpenName = P.inimg_file;


    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(fileOpenName.toStdString(),in_zz))
    {
        return false;
    }
    P.in_sz[0] = in_zz[0];
    P.in_sz[1] = in_zz[1];
    P.in_sz[2] = in_zz[2];

    LandmarkList allTargetList;
    QList<LandmarkList> allTipsList;

    LocationSimple tileLocation;
    tileLocation.x = P.listLandmarks[0].x;
    tileLocation.y = P.listLandmarks[0].y;
    tileLocation.z = P.listLandmarks[0].z;

    LandmarkList inputRootList;
    if(P.method != gd )inputRootList.push_back(tileLocation);

    allTipsList.push_back(inputRootList);

    tileLocation.x = tileLocation.x -int(P.block_size/2);
    tileLocation.y = tileLocation.y -int(P.block_size/2);

    //tracing_3D==true
    tileLocation.z = tileLocation.z -int(P.block_size/8);


    tileLocation.ev_pc1 = P.block_size;
    tileLocation.ev_pc2 = P.block_size;
    tileLocation.ev_pc3 = P.block_size/4;

    P.block_size = P.block_size/2;

    tileLocation.category = 1;
    allTargetList.push_back(tileLocation);


    P.output_folder = QFileInfo(P.markerfilename).path();
    QString tmpfolder;
    tmpfolder= P.markerfilename+QString("_tmp_APP2");


    if(!tmpfolder.isEmpty() && !P.resume)
       system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));

    system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));
    if(tmpfolder.isEmpty())
    {
        printf("Can not create a tmp folder!\n");
        return false;
    }

    QString finaloutputswc;
    if(P.global_name)
        finaloutputswc = P.markerfilename+QString("_nc_APP2_GD.swc");
    if(QFileInfo(finaloutputswc).exists() && !P.resume)
        system(qPrintable(QString("rm %1").arg(finaloutputswc.toStdString().c_str())));

    LandmarkList newTargetList;
    QList<LandmarkList> newTipsList;
    bool flag = true;
    while(allTargetList.size()>0)
    {
        newTargetList.clear();
        newTipsList.clear();

           app_tracing_ada_win_3D(callback,P,allTipsList.at(0),allTargetList.at(0),&newTargetList,&newTipsList);


        allTipsList.removeAt(0);
        allTargetList.removeAt(0);
        if(newTipsList.size()>0)
        {
            for(int i = 0; i < newTipsList.size(); i++)
            {
                allTargetList.push_back(newTargetList.at(i));
                allTipsList.push_back(newTipsList.at(i));
            }

            for(int i = 0; i < allTargetList.size();i++)
            {
                for(int j = 0; j < allTargetList.size();j++)
                {
                    if(allTargetList.at(i).radius > allTargetList.at(j).radius)
                    {
                        allTargetList.swap(i,j);
                        allTipsList.swap(i,j);
                    }
                }
            }
        }
    }

return true;
}




bool app_tracing_ada_win_3D(V3DPluginCallback2 &callback,TRACE_LS_PARA &P,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList)
{
    QString saveDirString;
    QString finaloutputswc;


    //if(P.method == app2)
    //{
        saveDirString = P.markerfilename+ QString("_tmp_APP2");
        //finaloutputswc = P.markerfilename + QString("x_%1_y_%2_z_%3_nc_app2_adp_3D.swc").arg(P.listLandmarks[0].x).arg(P.listLandmarks[0].y).arg(P.listLandmarks[0].z);
    //}


    //if(P.global_name)
        finaloutputswc = P.markerfilename+QString("_nc_APP2_GD.swc");

    QString imageSaveString = saveDirString;

    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    start_x = (tileLocation.x < 0)?  0 : tileLocation.x;
    start_y = (tileLocation.y < 0)?  0 : tileLocation.y;
    start_z = (tileLocation.z < 0)?  0 : tileLocation.z;


    end_x = tileLocation.x+tileLocation.ev_pc1;
    end_y = tileLocation.y+tileLocation.ev_pc2;
    end_z = tileLocation.z+tileLocation.ev_pc3;

    if(end_x > P.in_sz[0]) end_x = P.in_sz[0];
    if(end_y > P.in_sz[1]) end_y = P.in_sz[1];
    if(end_z > P.in_sz[2]) end_z = P.in_sz[2];

    if(tileLocation.x >= P.in_sz[0] - 1 || tileLocation.y >= P.in_sz[1] - 1 || tileLocation.z >= P.in_sz[2] - 1 || end_x <= 0 || end_y <= 0 || end_z <= 0)
    {
        printf("hit the boundary");
        return true;
    }

    QString scanDataFileString = saveDirString;
    scanDataFileString.append("/").append("scanData.txt");
    if(QFileInfo(
                ).exists() && !QFileInfo(scanDataFileString).exists() && !P.global_name)
        system(qPrintable(QString("rm -rf %1").arg(finaloutputswc.toStdString().c_str())));
    unsigned char * total1dData = 0;
    V3DLONG *in_sz = 0;


   //P.ini_img;

    in_sz = new V3DLONG[4];
    in_sz[0] = end_x - start_x;
    in_sz[1] = end_y - start_y;
    in_sz[2] = end_z - start_z;

    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(P.inimg_file.toStdString(),in_zz))
    {
        return false;
    }

    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    try {total1dData = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for loading the region.",0); return false;}
    if(P.channel > in_zz[3])
        P.channel = 1;
    unsigned char * total1dDataTerafly = 0;
    total1dDataTerafly = callback.getSubVolumeTeraFly(P.inimg_file.toStdString(),start_x,end_x,
                                                      start_y,end_y,start_z,end_z);

    for(V3DLONG i=0; i<pagesz; i++)
    {
        total1dData[i] = total1dDataTerafly[pagesz*(P.channel-1)+i];
    }
    if(total1dDataTerafly) {delete []total1dDataTerafly; total1dDataTerafly = 0;}



    Image4DSimple* total4DImage = new Image4DSimple;
    V3DLONG pagesz_vim = in_sz[0]*in_sz[1]*in_sz[2];
 //   unsigned char* total1dData_apa = 0;

    if(P.global_name)
    {
        double min,max;
        unsigned char * total1dData_scaled = 0;
        total1dData_scaled = new unsigned char [pagesz_vim];
        rescale_to_0_255_and_copy(total1dData,pagesz_vim,min,max,total1dData_scaled);
        total4DImage->setData((unsigned char*)total1dData_scaled, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
    }

    total4DImage->setOriginX(start_x);
    total4DImage->setOriginY(start_y);
    total4DImage->setOriginZ(start_z);

    V3DLONG mysz[4];
    mysz[0] = total4DImage->getXDim();
    mysz[1] = total4DImage->getYDim();
    mysz[2] = total4DImage->getZDim();
    mysz[3] = total4DImage->getCDim();

    total4DImage->setRezZ(3.0);//set the flg for 3d crawler

    imageSaveString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append("_z_").append(QString::number(start_z)).append(".v3draw");

    QString swcString = saveDirString;
    swcString.append("/x_").append(QString::number(start_x)).append("_y_").append(QString::number(start_y)).append("_z_").append(QString::number(start_z)).append(".swc");

    qDebug()<<scanDataFileString;
    QFile saveTextFile;
    saveTextFile.setFileName(scanDataFileString);// add currentScanFile
    if (!saveTextFile.isOpen()){
        if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
            qDebug()<<"unable to save file!";
            return false;}     }
    QTextStream outputStream;
    outputStream.setDevice(&saveTextFile);
    outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<< (int) total4DImage->getOriginZ()<<" "<<swcString<<" "<< (int) in_sz[0]<<" "<< (int) in_sz[1]<<" "<< (int) in_sz[2]<<"\n";
    saveTextFile.close();

    simple_saveimage_wrapper(callback, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, total4DImage->getDatatype());

    ifstream ifs_swc(finaloutputswc.toStdString().c_str());

    P.length_thresh = 5;

    PARA_APP2 p2;
    QString versionStr = "v0.001";


    p2.is_gsdt = P.is_gsdt;
    p2.is_coverage_prune = true;
    p2.is_break_accept = P.is_break_accept;
    p2.bkg_thresh = -1;//P.bkg_thresh;
    p2.length_thresh = P.length_thresh;
    p2.cnn_type = 2;
    p2.channel = 0;
    p2.SR_ratio = 3.0/9.9;
    p2.b_256cube = P.b_256cube;
    p2.b_RadiusFrom2D = P.b_RadiusFrom2D;
    p2.b_resample = 0;
    p2.b_intensity = 0;
    p2.b_brightfiled = 0;
    p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

    p2.p4dImage = total4DImage;
    p2.p4dImage->setFileName(imageSaveString.toStdString().c_str());
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = p2.p4dImage->getXDim()-1;
    p2.yc1 = p2.p4dImage->getYDim()-1;
    p2.zc1 = p2.p4dImage->getZDim()-1;


    NeuronTree nt;
    vector<MyMarker*> finalswc;

    if(ifs_swc)
       //finalswc = readSWC_file(finaloutputswc.toStdString());
    {
        NeuronTree swcex= callback.getSWCTeraFly();
        finalswc = nt2markers(swcex);
    }




    //meanshift first before tracing

    mean_shift_fun fun_obj;
    fun_obj.pushNewData<unsigned char>((unsigned char*)total1dData, mysz);

    vector<MyMarker*> tileswc_file;


    if( P.method==app2)
    {
        qDebug()<<"starting app2";
        qDebug()<<"rootlist size "<<QString::number(inputRootList.size());

        if(inputRootList.size() <1)
        {
              p2.outswc_file =swcString;
              proc_app2(callback, p2, versionStr);

        }
        else
        {
            for(int i = 0; i < inputRootList.size(); i++)
            {
                QString poutswc_file = swcString + (QString::number(i)) + (".swc");

                p2.outswc_file =poutswc_file;
                bool flag = false;
                LocationSimple RootNewLocation;
                RootNewLocation.x = inputRootList.at(i).x - total4DImage->getOriginX();
                RootNewLocation.y = inputRootList.at(i).y - total4DImage->getOriginY();
                RootNewLocation.z = inputRootList.at(i).z - total4DImage->getOriginZ();

                if(!ifs_swc )// used to be !ifs_swc&& P.soma
                {
                    LandmarkList marklist_tmp;
                    marklist_tmp.push_back(RootNewLocation);


                    ImageMarker outputMarker;
                    QList<ImageMarker> seedsToSave;
                    outputMarker.x = RootNewLocation.x;
                    outputMarker.y = RootNewLocation.y;
                    outputMarker.z = RootNewLocation.z;
                    seedsToSave.append(outputMarker);

                    vector<V3DLONG> poss_landmark;
                    double windowradius = 10;

                    poss_landmark=landMarkList2poss(marklist_tmp, mysz[0], mysz[0]*mysz[1]);
                    marklist_tmp.clear();
                    vector<float> mass_center=fun_obj.mean_shift_center_mass(poss_landmark[0],windowradius);
                    RootNewLocation.x = mass_center[0]+1;
                    RootNewLocation.y = mass_center[1]+1;
                    RootNewLocation.z = mass_center[2]+1;

                    outputMarker.x = RootNewLocation.x;
                    outputMarker.y = RootNewLocation.y;
                    outputMarker.z = RootNewLocation.z;
                    seedsToSave.append(outputMarker);

                    QString marker_name = imageSaveString + ".marker";
                    writeMarker_file(marker_name, seedsToSave);

                }

                const float dd = 0.5;

                if(RootNewLocation.x<-dd || RootNewLocation.x>total4DImage->getXDim()-1+dd || RootNewLocation.y<-dd || RootNewLocation.y>total4DImage->getYDim()-1+dd || RootNewLocation.z<-dd || RootNewLocation.z>total4DImage->getZDim()-1+dd)
                    continue;

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
                    vector<MyMarker*> inputswc;
                    bool soma_tile;
                    if(0)
                    {

//                        p1.landmarks.push_back(RootNewLocation);
//                        proc_app1(callback, p1, versionStr);
//                        p1.landmarks.clear();
                    }else
                    {

                        v3d_msg(QString("root is (%1,%2,%3").arg(RootNewLocation.x).arg(RootNewLocation.y).arg(RootNewLocation.z),0);
                        V3DLONG num_tips = 100;
                        double tips_th;
                        if(P.global_name)
                        {
                            tips_th = 10;
                            p2.is_break_accept = false;
                        }
                        else
                            tips_th = p2.p4dImage->getXDim()*100/512;
                       // p2.bkg_thresh = -1;//P.bkg_thresh;
                        double imgAve, imgStd;
                        mean_and_std(p2.p4dImage->getRawDataAtChannel(0), p2.p4dImage->getTotalUnitNumberPerChannel(), imgAve, imgStd);
                        double td= (imgStd<10)? 10: imgStd;
                        p2.bkg_thresh = imgAve +0.7*td ;

                        p2.landmarks.push_back(RootNewLocation);

                       // if(P.global_name && ifs_swc)
                        if(0)
                        {
                            double imgAve, imgStd;
                            mean_and_std(p2.p4dImage->getRawDataAtChannel(0), p2.p4dImage->getTotalUnitNumberPerChannel(), imgAve, imgStd);
                            double td= (imgStd<10)? 10: imgStd;
                            p2.bkg_thresh = imgAve +0.5*td ;
                            bool flag_high = false;
                            bool flag_low = false;
                            do
                            {
                                double fore_count = 0;
                                for(V3DLONG i = 0 ; i < p2.p4dImage->getTotalUnitNumberPerChannel(); i++)
                                {
                                    if(p2.p4dImage->getRawDataAtChannel(0)[i] > p2.bkg_thresh)
                                        fore_count++;
                                }

                                double fore_ratio = fore_count/p2.p4dImage->getTotalUnitNumberPerChannel();
                                if(fore_ratio > 0.05 && !flag_low)
                                {
                                    p2.bkg_thresh++;
                                    flag_high = true;
                                }else if (fore_ratio < 0.01 && !flag_high)
                                {
                                    p2.bkg_thresh--;
                                    flag_low = true;
                                }else
                                    break;
                            } while(1);
                        }

                        int count=0;
                        do
                        {
                            if(count>4)
                            {
                                inputswc.clear();
                                break;
                            }

                            count++;
                            soma_tile = false;
                            num_tips = 0;
                            proc_app2(callback, p2, versionStr);
    //                        if(ifs_swc)
    //                        {
    //                            NeuronTree nt_app2 = readSWC_file(poutswc_file);
    //                            NeuronTree nt_app2_pruned = pruning_cross_swc(nt_app2);
    //                            writeSWC_file(poutswc_file,nt_app2_pruned);
    //                        }

                            inputswc = readSWC_file(poutswc_file.toStdString());
                            NeuronTree inputswc_nt = readSWC_file(poutswc_file);
                            QVector<QVector<V3DLONG> > childs;
                            V3DLONG neuronNum = inputswc_nt.listNeuron.size();
                            childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
                            for (V3DLONG i=0;i<neuronNum;i++)
                            {
                                V3DLONG par = inputswc_nt.listNeuron[i].pn;
                                if (par<0) continue;
                                childs[inputswc_nt.hashNeuron.value(par)].push_back(i);
                            }

                            for(V3DLONG d = 0; d < inputswc_nt.listNeuron.size(); d++)
                            {
                                if(childs[d].size() == 0)
                                    num_tips++;
                                if(ifs_swc && inputswc[d]->radius >= 8) //was 8
                                {
                                    soma_tile = true;
                                }

                            }
                            if (num_tips>=tips_th  && ifs_swc) //add <=20 and #tips>=100 constraints by PHC 20170801
                            {
                                unsigned char* total1dData_apa = 0;
                                total1dData_apa = new unsigned char [pagesz_vim];
                                BinaryProcess(total1dData, total1dData_apa,in_sz[0],in_sz[1], in_sz[2], 5, 3);
                                p2.p4dImage->setData((unsigned char*)total1dData_apa, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
                                p2.bkg_thresh =-1;
                            }
                            else
                                break;
                        } while (1);

                        while(0 && inputswc.size()<=0) //by PHC 20170523
                        {
                            soma_tile = false;
                            num_tips = 0;
                            p2.bkg_thresh -=2;
                            proc_app2(callback, p2, versionStr);
                            inputswc = readSWC_file(poutswc_file.toStdString());
                            for(V3DLONG d = 0; d < inputswc.size(); d++)
                            {
                                if(ifs_swc && inputswc[d]->radius >= 20) //was 8
                                {
                                    soma_tile = true;
                                    break;
                                }
                            }
                        }

                        p2.landmarks.clear();
                    }

                    for(V3DLONG d = 0; d < inputswc.size(); d++)
                    {
                        if(soma_tile)   inputswc[d]->type = 0;
                        tileswc_file.push_back(inputswc[d]);
                    }
                }
            }
//            saveSWC_file(swcString.toStdString().c_str(), tileswc_file);
//            nt = readSWC_file(swcString);
        }
    }

    if(in_sz) {delete []in_sz; in_sz =0;}

    if(ifs_swc)
    {

        map<MyMarker*, double> score_map;
    //    vector<MyMarker *> neuronTree = readSWC_file(swcString.toStdString());
        topology_analysis_perturb_intense(total4DImage->getRawData(), tileswc_file, score_map, 1, p2.p4dImage->getXDim(), p2.p4dImage->getYDim(), p2.p4dImage->getZDim(), 1);

        for(V3DLONG i = 0; i<tileswc_file.size(); i++){
            MyMarker * marker = tileswc_file[i];
            double tmp = score_map[marker] * 120 +19;
            marker->type = (tmp > 255 || marker->type ==0) ? 255 : tmp;
        }
    }else
    {
        for(V3DLONG i = 0; i<tileswc_file.size(); i++){
            MyMarker * marker = tileswc_file[i];
            marker->type = 3;
        }
    }

    saveSWC_file(swcString.toStdString(),tileswc_file);
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

    int conf_th = 130;
    //assign all sub_trees
    QVector<int> visit(nt.listNeuron.size(),0);

    if(!ifs_swc)
    {
        for(int i=1; i<nt.listNeuron.size();i++)
        {
            if(NTDIS(nt.listNeuron.at(i),nt.listNeuron.at(0))<=40)
                visit[i]=1;

        }
        for(int i=0; i<nt.listNeuron.size();i++)
        {
            if(nt.listNeuron[i].radius>=8 && visit[i]==0)
            {
                QQueue<int> q;
                visit[i]=1;
                q.push_back(i);
                while(!q.empty())
                {
                    int current = q.front(); q.pop_front();
                    nt.listNeuron[current].type = 255;
                    for(int j=0; j<childs[current].size();j++)
                    {
                        if(visit[childs[current].at(j)]==0)
                        {
                            visit[childs[current].at(j)]=1;
                            q.push_back(childs[current].at(j));
                        }
                    }
                }
                q.clear();
            }
        }
    }
    else{
        for(int i=0; i<nt.listNeuron.size();i++)
        {
            if(nt.listNeuron[i].type>conf_th && nt.listNeuron[i].pn ==-1 && visit[i]==0)  //was 130
            {
                QQueue<int> q;
                visit[i]=1;
                q.push_back(i);
                while(!q.empty())
                {
                    int current = q.front(); q.pop_front();
                    for(int j=0; j<childs[current].size();j++)
                    {
                        int current_child = childs[current].at(j);
                        if(visit[current_child]==0)
                        {
                            visit[current_child]=1;
                            if(childs[current_child].size()<2)
                                q.push_back(current_child);
                        }
                    }
                }
                q.clear();
            }

            int diff_radius;
            if(nt.listNeuron[i].type<=conf_th && nt.listNeuron[i].radius<2 && visit[i]==0)
            {
                int count=0;
                int pre_sum=0;
                int next_sum=0;
                int pre=i, next=i;
                while(count<15 && childs[next].size()>0 && nt.listNeuron[pre].pn>0)
                {
                    pre_sum += nt.listNeuron[pre].radius;
                    next_sum += nt.listNeuron[next].radius;
                    pre = nt.hashNeuron.value(nt.listNeuron[pre].pn);
                    next = childs[next].at(0);
                    count++;
                }

                if(count==15)
                {
                    diff_radius = (next_sum-pre_sum);
                    double node_angle = angle(nt.listNeuron[i], nt.listNeuron[pre], nt.listNeuron[next]);

                    if(diff_radius>15 && node_angle<120) nt.listNeuron[i].type=255;
                }
            }

            if(nt.listNeuron[i].type>conf_th && visit[i]==0 && childs[i].size()==1)
            {
                QQueue<int> q;
                visit[i]=1;
                q.push_back(i);
                while(!q.empty())
                {
                    int current = q.front(); q.pop_front();
                    nt.listNeuron[current].type = 255;
                    for(int j=0; j<childs[current].size();j++)
                    {
                        if(visit[childs[current].at(j)]==0)
                        {
                            visit[childs[current].at(j)]=1;
                            q.push_back(childs[current].at(j));
                        }
                    }
                }
                q.clear();
            }
        }
    }

    LandmarkList tip_left;
    LandmarkList tip_right;
    LandmarkList tip_up ;
    LandmarkList tip_down;
    LandmarkList tip_out;
    LandmarkList tip_in;


    double overlap_ratio;
    if(P.method == gd)
        overlap_ratio = 0.5;
    else
        overlap_ratio = 0.05;

    QList<NeuronSWC> list = nt.listNeuron;
    if(list.size()<3) return true;
    for (V3DLONG i=0;i<list.size();i++)
    {
        if (childs[i].size()==0 || P.method != gd)
        {
            NeuronSWC curr = list.at(i);
            if(curr.type >conf_th) continue;
            LocationSimple newTip;
            bool check_tip = false;
            if( curr.x < overlap_ratio*  total4DImage->getXDim() || curr.x > (1-overlap_ratio) *  total4DImage->getXDim() || curr.y < overlap_ratio * total4DImage->getYDim() || curr.y > (1-overlap_ratio)* total4DImage->getYDim()
                    || curr.z < overlap_ratio*  total4DImage->getZDim() || curr.z > (1-overlap_ratio) *  total4DImage->getZDim())
            {
//                V3DLONG node_pn = getParent(i,nt);
//                V3DLONG node_pn_2nd;
//                if( list.at(node_pn).pn < 0)
//                {
//                    node_pn_2nd = node_pn;
//                }
//                else
//                {
//                    node_pn_2nd = getParent(node_pn,nt);
//                }

//                newTip.x = list.at(node_pn_2nd).x + total4DImage->getOriginX();
//                newTip.y = list.at(node_pn_2nd).y + total4DImage->getOriginY();
//                newTip.z = list.at(node_pn_2nd).z + total4DImage->getOriginZ();

//                newTip.radius = list.at(node_pn_2nd).r;

                newTip.x = curr.x + total4DImage->getOriginX();
                newTip.y = curr.y + total4DImage->getOriginY();
                newTip.z = curr.z + total4DImage->getOriginZ();
                newTip.radius = curr.r;

                for(V3DLONG j = 0; j < finalswc.size(); j++ )
                {
                    double dis = sqrt(pow2(newTip.x - finalswc.at(j)->x) + pow2(newTip.y - finalswc.at(j)->y) + pow2(newTip.z - finalswc.at(j)->z));
                    if(dis < 2*finalswc.at(j)->radius || dis < 10 || curr.type ==0)  //was 5
                   // if(dis < 10)
                    {
                        check_tip = true;
                        break;
                    }
                }
            }
            if(check_tip) continue;

            if( P.method == gd )
            {
                double x_diff = curr.x - list.at(0).x;
                double y_diff = curr.y - list.at(0).y;
                double z_diff = curr.z - list.at(0).z;

                if(fabs(x_diff) >= fabs(y_diff) && fabs(x_diff) >= fabs(z_diff))
                {
                    if(x_diff < 0)
                        tip_left.push_back(newTip);
                    else
                        tip_right.push_back(newTip);

                }else if(fabs(y_diff) >= fabs(x_diff) && fabs(y_diff) >= fabs(z_diff))
                {
                    if(y_diff < 0)
                        tip_up.push_back(newTip);
                    else
                        tip_down.push_back(newTip);

                }else if(fabs(z_diff) >= fabs(x_diff) && fabs(z_diff) >= fabs(y_diff))
                {
                    if(z_diff < 0)
                        tip_in.push_back(newTip);
                    else
                        tip_out.push_back(newTip);
                }

            }else
            {

                if( curr.x < overlap_ratio* total4DImage->getXDim())
                {
                    tip_left.push_back(newTip);
                }else if (curr.x > (1-overlap_ratio) * total4DImage->getXDim())
                {
                    tip_right.push_back(newTip);
                }else if (curr.y < overlap_ratio * total4DImage->getYDim())
                {
                    tip_up.push_back(newTip);
                }else if (curr.y > (1-overlap_ratio)*total4DImage->getYDim())
                {
                    tip_down.push_back(newTip);
                }else if (curr.z < overlap_ratio * total4DImage->getZDim())
                {
                    tip_out.push_back(newTip);
                }else if (curr.z > (1-overlap_ratio)*total4DImage->getZDim())
                {
                    tip_in.push_back(newTip);
                }
            }
        }
    }

    if(tip_left.size()>0)
    {
        QList<LandmarkList> group_tips_left = group_tips(tip_left,512,1);
        for(int i = 0; i < group_tips_left.size();i++)
        {
            if(P.method == gd)
               {} //ada_win_finding_3D_GD(group_tips_left.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,1);
            else
                ada_win_finding_3D(group_tips_left.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,1);
        }
    }
    if(tip_right.size()>0)
    {
        QList<LandmarkList> group_tips_right = group_tips(tip_right,512,2);
        for(int i = 0; i < group_tips_right.size();i++)
        {
            if(P.method == gd)
            {}
              //  ada_win_finding_3D_GD(group_tips_right.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,2);
            else
                ada_win_finding_3D(group_tips_right.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,2);
        }
    }
    if(tip_up.size()>0)
    {
        QList<LandmarkList> group_tips_up = group_tips(tip_up,512,3);
        for(int i = 0; i < group_tips_up.size();i++)
        {
            if(P.method == gd)
                {}//ada_win_finding_3D_GD(group_tips_up.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,3);
            else
                ada_win_finding_3D(group_tips_up.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,3);
        }
    }
    if(tip_down.size()>0)
    {
        QList<LandmarkList> group_tips_down = group_tips(tip_down,512,4);
        for(int i = 0; i < group_tips_down.size();i++)
        {
            if(P.method == gd)
             {}
                //ada_win_finding_3D_GD(group_tips_down.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,4);
            else
                ada_win_finding_3D(group_tips_down.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,4);
        }
    }

    if(tip_out.size()>0)
    {
        QList<LandmarkList> group_tips_out = group_tips(tip_out,512,5);
        for(int i = 0; i < group_tips_out.size();i++)
        {
            if(P.method == gd)
                {}//ada_win_finding_3D_GD(group_tips_out.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,5);
            else
                ada_win_finding_3D(group_tips_out.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,5);
        }
    }

    if(tip_in.size()>0)
    {
        QList<LandmarkList> group_tips_in = group_tips(tip_in,512,6);
        for(int i = 0; i < group_tips_in.size();i++)
        {
            if(P.method == gd)
                {}//ada_win_finding_3D_GD(group_tips_in.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,6);
            else
                ada_win_finding_3D(group_tips_in.at(i),tileLocation,newTargetList,newTipsList,total4DImage,P.block_size,6);
        }
    }

    if(P.method == gd)
        tileswc_file = readSWC_file(swcString.toStdString());

    if(ifs_swc)
    {
        for(V3DLONG i = 0; i < tileswc_file.size(); i++)
        {
            tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
            tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
            tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();

            finalswc.push_back(tileswc_file[i]);
        }
        //saveSWC_file(finaloutputswc.toStdString().c_str(), finalswc);
    }
    else
    {
        for(V3DLONG i = 0; i < tileswc_file.size(); i++)
        {
            tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
            tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
            tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();
        }
        //saveSWC_file(finaloutputswc.toStdString().c_str(), tileswc_file);
    }

    total4DImage->deleteRawDataAndSetPointerToNull();

    return true;
}

QList<LandmarkList> group_tips(LandmarkList tips,int block_size, int direction)
{
    QList<LandmarkList> groupTips;

   //bubble sort
   if(direction == 1 || direction == 2 || direction == 5 || direction == 6)
   {
       for(int i = 0; i < tips.size();i++)
       {
           for(int j = 0; j < tips.size();j++)
           {
               if(tips.at(i).y < tips.at(j).y)
                   tips.swap(i,j);
           }
       }

       LandmarkList eachGroupList;
       eachGroupList.push_back(tips.at(0));
       for(int d = 0; d < tips.size()-1; d++)
       {
           if(tips.at(d+1).y - tips.at(d).y < block_size)
           {
               eachGroupList.push_back(tips.at(d+1));
           }
           else
           {
               groupTips.push_back(eachGroupList);
               eachGroupList.erase(eachGroupList.begin(),eachGroupList.end());
               eachGroupList.push_back(tips.at(d+1));
           }
       }
       groupTips.push_back(eachGroupList);
   }else
   {
       for(int i = 0; i < tips.size();i++)
       {
           for(int j = 0; j < tips.size();j++)
           {
               if(tips.at(i).x < tips.at(j).x)
                   tips.swap(i,j);
           }
       }

       LandmarkList eachGroupList;
       eachGroupList.push_back(tips.at(0));
       for(int d = 0; d < tips.size()-1; d++)
       {
           if(tips.at(d+1).x - tips.at(d).x < block_size)
           {
               eachGroupList.push_back(tips.at(d+1));
           }
           else
           {
               groupTips.push_back(eachGroupList);
               eachGroupList.erase(eachGroupList.begin(),eachGroupList.end());
               eachGroupList.push_back(tips.at(d+1));

           }
       }
       groupTips.push_back(eachGroupList);
   }
   return groupTips;
}


bool ada_win_finding_3D(LandmarkList tips,LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList,Image4DSimple* total4DImage,int block_size,int direction)
{
    newTipsList->push_back(tips);
    double overlap = 0.1;

    float min_x = INF, max_x = -INF;
    float min_y = INF, max_y = -INF;
    float min_z = INF, max_z = -INF;


    double adaptive_size_x,adaptive_size_y,adaptive_size_z;
    double max_r = -INF;

    if(direction == 1 || direction == 2)
    {
        for(int i = 0; i<tips.size();i++)
        {
            if(tips.at(i).y <= min_y) min_y = tips.at(i).y;
            if(tips.at(i).y >= max_y) max_y = tips.at(i).y;

            if(tips.at(i).z <= min_z) min_z = tips.at(i).z;
            if(tips.at(i).z >= max_z) max_z = tips.at(i).z;

            if(tips.at(i).radius >= max_r) max_r = tips.at(i).radius;
        }

        adaptive_size_y = (max_y - min_y)*1.2;
        adaptive_size_z = (max_z - min_z)*1.2;
        adaptive_size_x = adaptive_size_y;

    }else if(direction == 3 || direction == 4)
    {
        for(int i = 0; i<tips.size();i++)
        {
            if(tips.at(i).x <= min_x) min_x = tips.at(i).x;
            if(tips.at(i).x >= max_x) max_x = tips.at(i).x;

            if(tips.at(i).z <= min_z) min_z = tips.at(i).z;
            if(tips.at(i).z >= max_z) max_z = tips.at(i).z;

            if(tips.at(i).radius >= max_r) max_r = tips.at(i).radius;
        }

        adaptive_size_x = (max_x - min_x)*1.2;
        adaptive_size_z = (max_z - min_z)*1.2;
        adaptive_size_y = adaptive_size_x;
    }else
    {
        for(int i = 0; i<tips.size();i++)
        {
            if(tips.at(i).x <= min_x) min_x = tips.at(i).x;
            if(tips.at(i).x >= max_x) max_x = tips.at(i).x;

            if(tips.at(i).y <= min_y) min_y = tips.at(i).y;
            if(tips.at(i).y >= max_y) max_y = tips.at(i).y;

            if(tips.at(i).radius >= max_r) max_r = tips.at(i).radius;
        }

        adaptive_size_x = (max_x - min_x)*1.2;
        adaptive_size_y = (max_y - min_y)*1.2;
        adaptive_size_z = adaptive_size_x;
    }

    adaptive_size_x = (adaptive_size_x <= 128) ? 128 : adaptive_size_x;
    adaptive_size_y = (adaptive_size_y <= 128) ? 128 : adaptive_size_y;
    adaptive_size_z = (adaptive_size_z <= 128) ? 128 : adaptive_size_z;


    adaptive_size_x = (adaptive_size_x >= block_size) ? block_size : adaptive_size_x;
    adaptive_size_y = (adaptive_size_y >= block_size) ? block_size : adaptive_size_y;
    adaptive_size_z = (adaptive_size_z >= block_size) ? block_size : adaptive_size_z;

    LocationSimple newTarget;
    if(direction == 1)
    {
        newTarget.x = -floor(adaptive_size_x*(1.0-overlap)) + tileLocation.x;
        newTarget.y = floor((min_y + max_y - adaptive_size_y)/2);
        newTarget.z = floor((min_z + max_z - adaptive_size_z)/2);
    }else if(direction == 2)
    {
        newTarget.x = tileLocation.x + tileLocation.ev_pc1 - floor(adaptive_size_x*overlap);
        newTarget.y = floor((min_y + max_y - adaptive_size_y)/2);
        newTarget.z = floor((min_z + max_z - adaptive_size_z)/2);

    }else if(direction == 3)
    {
        newTarget.x = floor((min_x + max_x - adaptive_size_x)/2);
        newTarget.y = -floor(adaptive_size_y*(1.0-overlap)) + tileLocation.y;
        newTarget.z = floor((min_z + max_z - adaptive_size_z)/2);
    }else if(direction == 4)
    {
        newTarget.x = floor((min_x + max_x - adaptive_size_x)/2);
        newTarget.y = tileLocation.y + tileLocation.ev_pc2 - floor(adaptive_size_y*overlap);
        newTarget.z = floor((min_z + max_z - adaptive_size_z)/2);
    }else if(direction == 5)
    {
        newTarget.x = floor((min_x + max_x - adaptive_size_x)/2);
        newTarget.y = floor((min_y + max_y - adaptive_size_y)/2);
        newTarget.z = -floor(adaptive_size_z*(1.0-overlap)) + tileLocation.z;
    }else if(direction == 6)
    {
        newTarget.x = floor((min_x + max_x - adaptive_size_x)/2);
        newTarget.y = floor((min_y + max_y - adaptive_size_y)/2);
        newTarget.z = tileLocation.z + tileLocation.ev_pc3 - floor(adaptive_size_z*overlap);
    }


   // v3d_msg(QString("zmin is %1, zmax is %2, z is %3, z_winsize is %4").arg(min_z).arg(max_z).arg(tileLocation.z).arg(adaptive_size_z));


    newTarget.ev_pc1 = adaptive_size_x;
    newTarget.ev_pc2 = adaptive_size_y;
    newTarget.ev_pc3 = adaptive_size_z;

    newTarget.radius = max_r;

    newTargetList->push_back(newTarget);
    return true;
}


