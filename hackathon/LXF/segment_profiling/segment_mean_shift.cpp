//#include "mean_shift_fun.h"
#include "segment_profiling_main.h"
#include "segment_mean_shift.h"

bool segment_mean_shift(unsigned char* &data1d,LandmarkList &LList,V3DLONG im_cropped_sz[4],int i,vector<MyMarker*> &nt_marker2)
{
    cout<<"segment_mean_shift"<<endl;
    mean_shift_fun fun_obj;
    int windowradius=5;
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
    cout<<"LList_new_center.size() = "<<LList_new_center.size()<<endl;

cout<<"poss_landmark.size() = "<<poss_landmark.size()<<endl;
    NeuronTree outresult;
    for(V3DLONG j=0;j<poss_landmark.size();j++)
    {
        cout<<"aaaaaaaaa"<<endl;
        NeuronSWC M;
        M.x = LList_new_center[j].x;
        M.y = LList_new_center[j].y;
        M.z = LList_new_center[j].z;
        M.radius = nt_marker2[j]->radius;
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
