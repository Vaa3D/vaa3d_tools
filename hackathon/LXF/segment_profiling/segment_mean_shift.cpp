//#include "mean_shift_fun.h"

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
LandmarkList segment_mean_shift_v2(V3DPluginCallback2 &callback,LandmarkList &LList,PARA &PA,int i,QList <NeuronSWC> &nt_marker2)
{
 //   LandmarkList emptylist = LandmarkList();
    mean_shift_fun mean_shift_obj;
    LandmarkList LList_new_center;
    vector<V3DLONG> poss_landmark;
    vector<float> mass_center;
    int methodcode = 1;
  //  unsigned char *data_1d;
  //  V3DLONG in_sz[4];
  //  int databyte;
//    if(!simple_loadimage_wrapper(callback,PA.img_name.toStdString().c_str(),data_1d,in_sz,databyte))
//    {
//        v3d_msg("something wrong with loading img!");
//        return emptylist;
//    }
    mean_shift_obj.pushNewData<unsigned char>((unsigned char*)PA.data1d, PA.im_cropped_sz);
    //set parameter
    int windowradius=15;
    //start mean-shift
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, PA.im_cropped_sz[0],PA.im_cropped_sz[0]*PA.im_cropped_sz[1]);
    for (int j=0;j<LList.size();j++)
    {
        if (methodcode==2)
        mass_center=mean_shift_obj.mean_shift_with_constraint(poss_landmark[j],windowradius);
        else
        mass_center=mean_shift_obj.mean_shift_center(poss_landmark[j],windowradius);

        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        if (!LList.at(j).name.empty()) tmp.name=LList.at(j).name;
        LList_new_center.append(tmp);
    }

    NeuronTree nt;
    for(V3DLONG k = 0; k < LList_new_center.size(); k++)
    {

        NeuronSWC S;
        S.x = LList_new_center.at(k).x;
        S.y = LList_new_center.at(k).y;
        S.z = LList_new_center.at(k).z;
        S.n = k;
        S.r = LList_new_center.at(k).radius;
        S.pn = k-1;
        S.type = 2;
        nt.listNeuron.push_back(S);
    }
    PA.nt_meanshift = nt;
    QString swc_out_name = QString::number(i) + "_meanshifted.swc";
    writeSWC_file(swc_out_name,nt);
    return LList_new_center;

}
