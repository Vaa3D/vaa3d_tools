//#include "mean_shift_fun.h"
#include "segment_profiling_main.h"
#include "segment_mean_shift.h"

bool segment_mean_shift(unsigned char* &data1d,LandmarkList &LList,V3DLONG im_cropped_sz[4],int i,vector<MyMarker*> &nt_marker2)
{
    cout<<"segment_mean_shift"<<endl;
    mean_shift_fun fun_obj;
    int windowradius=3;
    V3DLONG size_tmp=im_cropped_sz[0]*im_cropped_sz[1]*im_cropped_sz[2]*im_cropped_sz[3];
    cout<<im_cropped_sz[0]<<"  "<<im_cropped_sz[1]<<"  "<<im_cropped_sz[2]<<endl;

    fun_obj.pushNewData<unsigned char>((unsigned char*)data1d, im_cropped_sz);

    vector<V3DLONG> poss_landmark;
    poss_landmark=landMarkList2poss(LList, im_cropped_sz[0], im_cropped_sz[0]*im_cropped_sz[1]);
    LandmarkList LList_new_center;
    vector<float> mass_center;
    for (int j=0;j<poss_landmark.size();j++)
    {
        //append the original marker in LList_new_center
        LList[j].name="ori";
        LList[j].color.r=255; LList[j].color.g=LList[j].color.b=0;
        LList_new_center.append(LList.at(j));
        mass_center=fun_obj.mean_shift_center(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        tmp.color.r=170; tmp.color.g=0; tmp.color.b=255;
        tmp.name="ms";
        LList_new_center.append(tmp);
    }
    NeuronTree outresult;
    for(V3DLONG j=0;j<poss_landmark.size();j++)
    {
        NeuronSWC M;
        M.n = j;
        M.type = 5;
        M.x = LList_new_center[j].x;
        M.y = LList_new_center[j].y;
        M.z = LList_new_center[j].z;
        M.radius = nt_marker2[j]->radius;
        M.parent = j-1;
        cout<<"M = "<<M.x<<"  "<<M.y<<"  "<<M.z<<"  "<<M.radius<<endl;
        outresult.listNeuron.push_back(M);
    }
    QString name = "save_swc_file_"+QString::number(i)+".swc";
    //saveSWC_file(name.toStdString(),marker_tmp);
    writeSWC_file(name,outresult);


//    vector<MyMarker*> marker_tmp;
//    for(V3DLONG j=0;j<LList_new_center.size();j++)
//    {
//        cout<<"aaaaaaaaa"<<endl;
//        MyMarker* M;
//        M->x = LList_new_center[j].x;
//        M->y = LList_new_center[j].y;
//        M->z = LList_new_center[j].z;
//        M->radius = nt_marker2[j]->radius;
//        M->parent = nt_marker2[j]->parent;
//        cout<<"M = "<<M->x<<"  "<<M->y<<"  "<<M->z<<"  "<<M->radius<<"  "<<M->parent<<"  "<<endl;
//        marker_tmp.push_back(M);
//    }
//    QString name = "save_swc_file_"+QString::number(i)+".swc";
//    saveSWC_file(name.toStdString(),marker_tmp);
    return true;


}
LandmarkList segment_mean_shift_v2(V3DPluginCallback2 &callback,LandmarkList &LList,PARA PA,int i,vector<MyMarker*> &nt_marker2)
{
    //v3d_msg("LXF");
    //mean_shift_dialog *dialog=new mean_shift_dialog(&callback,0);
    LandmarkList emptylist = LandmarkList();
    mean_shift_fun mean_shift_obj;
    LandmarkList LList_new_center;
    vector<V3DLONG> poss_landmark;
    vector<float> mass_center;
    unsigned char *data_1d;
    V3DLONG in_sz[4];
    int databyte;
    if(!simple_loadimage_wrapper(callback,PA.img_name.toStdString().c_str(),data_1d,in_sz,databyte))
    {
        v3d_msg("something wrong with loading img!");
        return emptylist;
    }
//    if (!load_data(callback,image1Dc_in,LList,pixeltype,sz_img,curwin))
//        return emptylist;
  //  V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

    if(1)//V3D_UNIT8
    {
        mean_shift_obj.pushNewData<unsigned char>((unsigned char*)data_1d, in_sz);
    }

//    else if (pixeltype == 2) //V3D_UINT16;
//    {
//        mean_shift_obj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
//        convert2UINT8_meanshift((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
//    }
//    else if(pixeltype == 4) //V3D_FLOAT32;
//    {
//        mean_shift_obj.pushNewData<float>((float*)image1Dc_in, sz_img);
//        convert2UINT8_meanshift((float*)image1Dc_in, image1Dc_in, size_tmp);
//    }
//    else
//    {
//       QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
//       return(emptylist);
//    }

    //set parameter
    int windowradius=30;


    //start mean-shift
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, in_sz[0],in_sz[0]*in_sz[1]);
    cout<<"LList = "<<LList.size()<<endl;
    cout<<"poss_landmark = "<<poss_landmark.size()<<endl;
    cout<<"in_sz = "<<in_sz[0]<<"  "<<in_sz[1]<<"  "<<in_sz[2]<<endl;
    double ratio = windowradius;
    for (int j=0;j<LList.size();j++)
    {
        qDebug()<<"poss_landmark[j] = "<<poss_landmark[j];
        mass_center=mean_shift_obj.mean_shift_center(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        if (!LList.at(j).name.empty()) tmp.name=LList.at(j).name;
        LList_new_center.append(tmp);

    }

    NeuronTree nt;
    cout<<"LList_new_center.size() = "<<LList_new_center.size()<<endl;
    for(V3DLONG k = 0; k < LList_new_center.size(); k++)
    {
        //cout<<"LList_new_center = "<<LList_new_center.at(k).x<<"  "<<LList_new_center.at(k).y<<"  "<<LList_new_center.at(k).z<<"  "<<endl;
        NeuronSWC S;
        S.x = LList_new_center.at(k).x;
        S.y = LList_new_center.at(k).y;
        S.z = LList_new_center.at(k).z;
        //cout<<"hahahah"<<endl;
        S.n = k;
        S.r = 2;
        S.pn = k-1;
        S.type = 2;
        nt.listNeuron.push_back(S);
    }
    QString swc_out_name = QString::number(i) + "_meanshifted.swc";
    writeSWC_file(swc_out_name,nt);
    // v3d_msg(QString("Now you can drag and drop the meanshifted swc fle [%1] into Vaa3D.").arg(swc_out_name));

    return LList_new_center;
    //dialog-> image1Dc_in = data1d;
   // dialog->core();

}
