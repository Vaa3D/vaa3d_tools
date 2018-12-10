#include <v3d_interface.h>
#include "v3d_message.h"
#include "tracing_func.h"
#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app1.h"
#include "swc_surf_objs.h"
#include "stack"
#include "neurontracer_plugin.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../../../released_plugins/v3d_plugins/resample_swc/resampling.h"
#include "../../../released_plugins/v3d_plugins/neuron_image_profiling/profile_swc.h"
#include "../../../v3d_main/jba/c++/convert_type2uint8.h"
//#include "../../../hackathon/zhi/AllenNeuron_postprocessing/sort_swc_IVSCC.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h"
#include "../../../hackathon/zhi/branch_point_detection/branch_pt_detection_func.h"
#include "../../../../v3d_external/v3d_main/basic_c_fun/stackutil.h"
#include "../anisodiffusion_LXF/src/q_imgaussian3D.h"
#include "../anisodiffusion_LXF/src/q_EigenVectors3D.h"
#include "../anisodiffusion_LXF/src/q_derivatives3D.h"
#include "../anisodiffusion_LXF/src/q_AnisoDiff3D.h"
<<<<<<< HEAD
=======
#include "../soma_remove/soma_remove_main.h"
>>>>>>> remove_glio version which is operational
//#include "../anisodiffusion_LXF/src/EigenDecomposition3.h"
//#include "../anisodiffusion_LXF/src/EigenDecomposition3.h"
bool export_2dtif(V3DPluginCallback & cb,const char * filename, unsigned char * pdata, V3DLONG sz[3], int datatype);
extern int thresh;
extern NeuronTree trace_result,resultTree_rebase,resultTree;
LandmarkList marker_rebase,marker_rebase2;
LandmarkList marker_rebase3;  //original marker
LocationSimple simple_rebase,true_rebase,LXF;
extern LocationSimple next_m;
V3DLONG marker_num_rebase=0;
V3DLONG thres_rebase=42;
QString outimg_file;
extern bool change;
bool is_soma =false;
extern bool enhance;
#if  defined(Q_OS_LINUX)
    #include <omp.h>
#endif

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

#include <boost/lexical_cast.hpp>
template <class T> T pow2(T a)
{
    return a*a;

}

QString getAppPath();

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

// group images blending function

bool match_marker(V3DPluginCallback2 &callback,vector<int> &ind,LandmarkList &terafly_landmarks,LocationSimple &t)
{
    for(V3DLONG i=0;i<terafly_landmarks.size();i++)
    {
        cout<<"new marker is "<<terafly_landmarks[i].x<<"  "<<terafly_landmarks[i].y<<"  "<<terafly_landmarks[i].z<<"  "<<endl;
    }
    const Image4DSimple* curr = callback.getImageTeraFly();
    double para_ratio = curr->getRezX()/curr->getXDim();
    vector<int> loc;
    NeuronTree resultTree_2;
    loc.clear();
    resultTree_2 = callback.getSWCTeraFly();
    writeSWC_file(QString("test_swc.swc"),resultTree_2);
    //v3d_msg("check1");
    if(terafly_landmarks.size()==1)
    {
        t.x = terafly_landmarks[0].x;
        t.y = terafly_landmarks[0].y;
        t.z = terafly_landmarks[0].z;
    }
    else
    {
        if(ind.size()==1)
        {
            t.x = terafly_landmarks[ind[0]].x;
            t.y = terafly_landmarks[ind[0]].y;
            t.z = terafly_landmarks[ind[0]].z;
        }
        else
        {

            for(V3DLONG i=0;i<terafly_landmarks.size();i++)
            {
                double dis;double min_dis=100000000000;
                for(V3DLONG j=0;j<resultTree_2.listNeuron.size();j++)
                {
                    cout<<resultTree_2.listNeuron[j].x<<"  "<<terafly_landmarks[i].x<<endl;
                    dis = sqrt( (resultTree_2.listNeuron[j].x - terafly_landmarks[i].x)*(resultTree_2.listNeuron[j].x - terafly_landmarks[i].x) + (resultTree_2.listNeuron[j].y - terafly_landmarks[i].y)*(resultTree_2.listNeuron[j].y - terafly_landmarks[i].y)+(resultTree_2.listNeuron[j].z - terafly_landmarks[i].z)*(resultTree_2.listNeuron[j].z - terafly_landmarks[i].z) );
                    if(min_dis>dis)
                    {
                        min_dis = dis;
                    }
                }
                //cout<<"min_dis = "<<min_dis<<endl;
                //v3d_msg("min_dis");
                //cout<<"resultTree.listNeuron.size = "<<resultTree.listNeuron.size()<<endl;
                if(min_dis==100000000000)
                {
                    v3d_msg("resultTree_2 is void");
                    return false;
                }
                //cout<<"min______________________dis = "<<min_dis<<endl;

                //cout<<"min_dis/para = "<<min_dis/para_ratio<<endl;
                if(min_dis/para_ratio>3)  //4
                {
                    //v3d_msg("terafly_landmarks fit");
                    cout<<"terafly_landmarks fit = "<<terafly_landmarks[i].x<<"  "<<terafly_landmarks[i].y<<endl;
                    loc.push_back(i);
                }
                else
                {
                    //v3d_msg("terafly_landmarks don't fit");
                    cout<<"terafly_landmarks_don't fit = "<<terafly_landmarks[i].x<<"  "<<terafly_landmarks[i].y<<endl;
                }
                cout<<"loc.size() = "<<loc.size()<<endl;
            }

            if(loc.size()!=1)
            {
                if(loc.size()==0)
                {
                    t = simple_rebase;
                    return true;
                }
                else
                {
                    cout<<"loc.size = "<<loc.size()<<endl;
                    v3d_msg("there is a problem about markers");
                    return false;
                }
            }
            else   //loc just one
            {
                cout<<"loc[0] = "<<loc[0]<<endl;
                cout<<"ind.size = "<<ind.size()<<endl;
                if(ind.size()==0)
                {
                    v3d_msg("marker is wrong,please check it");
                    return false;
                }
                for(V3DLONG i=0;i<ind.size();i++)
                {
                    cout<<"ind"<<ind[i]<<endl;
                }
                t.x = terafly_landmarks[loc[0]].x;
                t.y = terafly_landmarks[loc[0]].y;
                t.z = terafly_landmarks[loc[0]].z;
            }
            //v3d_msg("in 2");.

        }
    }
    simple_rebase = t;
    return true;
}


bool crawler_raw_app(V3DPluginCallback2 &callback, QWidget *parent,TRACE_LS_PARA &P,bool bmenu)
{
    cout<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%crawler raw app&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
    QElapsedTimer timer1;
    timer1.start();
    bool use_comment = false;
    if(resultTree_rebase.listNeuron.size()==0)
    {
        resultTree_rebase = callback.getSWCTeraFly();
    }
    const Image4DSimple *curr_block = callback.getImageTeraFly();
    outimg_file = "X_"+QString::number(curr_block->getOriginX())+"Y_"+QString::number(curr_block->getOriginY())+"Z_"+QString::number(curr_block->getOriginZ())+".v3draw";
    QString imageSaveString = "test_app2.v3draw";
    QString swcString = outimg_file + "_app2.swc";
    QString outmarker_file =outimg_file + ".marker";
    V3DLONG data1d_sz[4];
    if(bmenu)
    {
        double center_x,center_y,center_z;
        V3DLONG xe,xb,ye,yb,ze,zb;
        V3DLONG sc = 1;
        LandmarkList terafly_landmarks = callback.getLandmarkTeraFly();
        if(P.markerfilename.isEmpty())
        {
            for(V3DLONG i=0;i<terafly_landmarks.size();i++)
            {
                center_x = terafly_landmarks[i].x;
                center_y = terafly_landmarks[i].y;
                center_z = terafly_landmarks[i].z;
            }
        }
        else
        {
            if(P.markerfilename.endsWith(".marker",Qt::CaseSensitive))
            {

                vector<MyMarker> markers;
                markers = readMarker_file(string(qPrintable(P.markerfilename)));
                for(int i = 0; i < markers.size(); i++)
                {
                    center_x = markers[i].x;
                    center_y = markers[i].y;
                    center_z = markers[i].z;
                }
            }
            else
            {
                QList<CellAPO> markers;
                markers = readAPO_file(P.markerfilename);
                for(int i = 0; i < markers.size(); i++)
                {
                    center_x = markers[i].x;
                    center_y = markers[i].y;
                    center_z = markers[i].z;
                }
            }
        }

        V3DLONG lens_x=P.block_size;
        V3DLONG lens_y=P.block_size;
        V3DLONG lens_z=P.block_size;
        xb = center_x-lens_x;
        xe = center_x+lens_x;
        yb = center_y-lens_y;
        ye = center_y+lens_y;
        zb = center_z-lens_z;
        ze = center_z+lens_z;
        data1d_sz[0] = xe - xb + 1;
        data1d_sz[1] = ye - yb + 1;
        data1d_sz[2] = ze - zb + 1;
        data1d_sz[3] = sc;
        P.o_x = xb;
        P.o_y = yb;
        P.o_z = zb;
        unsigned char *data1d=0;
        data1d = callback.getSubVolumeTeraFly(P.inimg_file.toStdString(),xb,xe+1,yb,ye+1,zb,ze+1);
        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)data1d,data1d_sz,1);
    }
    else
    {
        cout<<"$$$$$$$$$$$$$$$$$$$$$$get into curr window$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
        P.inimg_file = outimg_file;

        LandmarkList terafly_landmarks_terafly = callback.getLandmarkTeraFly();//terafly_landmarks
        LandmarkList new_marker;
        LocationSimple t;
        double ox = curr_block->getOriginX();
        double oy = curr_block->getOriginY();
        double oz = curr_block->getOriginZ();
        double lx = curr_block->getRezX();
        double ly = curr_block->getRezY();
        double lz = curr_block->getRezZ();
        P.ratio_x = curr_block->getRezX()/curr_block->getXDim();
        P.ratio_y = curr_block->getRezY()/curr_block->getYDim();
        P.ratio_z = curr_block->getRezZ()/curr_block->getZDim();
        P.o_x=curr_block->getOriginX();
        P.o_y=curr_block->getOriginY();
        P.o_z=curr_block->getOriginZ();


        if(terafly_landmarks_terafly.isEmpty())return false;
        LandmarkList terafly_landmarks;
        LandmarkList other_marker;
        //set<LocationSimple> terafly_landmarks;
        for(V3DLONG i=0;i<terafly_landmarks_terafly.size();i++) //rebase one is t,add in 0607,if it doesn't work or don't need it,just delete this;
        {
            LocationSimple s = terafly_landmarks_terafly[i];
            if(s.comments == "a")
            {
                t = s;
                use_comment = true;
            }
            else
            {
                marker_rebase3.push_back(s);
            }
        }
        if(use_comment == false)
        {
            for(V3DLONG i=0;i<terafly_landmarks_terafly.size();i++)
            {
                LocationSimple s = terafly_landmarks_terafly[i];
                if(s.x<ox+lx&&s.y<oy+ly&&s.z<oz+lz&&s.x>ox&&s.y>oy&&s.z>oz)
                {
                    //terafly_landmarks.insert(s);
                    //   s.comments
                    terafly_landmarks.push_back(s);
                }
                else
                {
                    other_marker.push_back(s);
                }
            }

            data1d_sz[0] = curr_block->getXDim();
            data1d_sz[1] = curr_block->getYDim();
            data1d_sz[2] = curr_block->getZDim();
            data1d_sz[3] = curr_block->getCDim();



            cout<<"marker rebase num = "<<marker_rebase.size()<<endl;
            vector<int> ind;
            for(V3DLONG i=0;i<terafly_landmarks.size();i++)
            {
                double dis;
                double min_dis = 10000000000;
                for(V3DLONG j=0;j<marker_rebase.size();j++)
                {
                    dis = (terafly_landmarks[i].x-marker_rebase[j].x)*(terafly_landmarks[i].x-marker_rebase[j].x)+(terafly_landmarks[i].y-marker_rebase[j].y)*(terafly_landmarks[i].y-marker_rebase[j].y)+(terafly_landmarks[i].z-marker_rebase[j].z)*(terafly_landmarks[i].z-marker_rebase[j].z);
                    if(dis<min_dis)
                    {
                        min_dis = dis;
                    }

                }
                //v3d_msg("out_1");
                if(min_dis>0.0001)
                {
                    ind.push_back(i);
                    new_marker.push_back(terafly_landmarks[i]);
                }
            }

            if(!match_marker(callback,ind,new_marker,t))
            {
                v3d_msg("abort");
                return false;
            }
            marker_rebase3.clear();
            for(V3DLONG i=0;i<terafly_landmarks_terafly.size();i++)
            {
                if((t.x-terafly_landmarks_terafly[i].x)*(t.x-terafly_landmarks_terafly[i].x)+(t.y-terafly_landmarks_terafly[i].y)*(t.y-terafly_landmarks_terafly[i].y)+(t.z-terafly_landmarks_terafly[i].z)*(t.z-terafly_landmarks_terafly[i].z)>0.001)
                {
                    marker_rebase3.push_back(terafly_landmarks_terafly[i]);
                }
            }
            cout<<marker_rebase3.size()<<"  "<<terafly_landmarks_terafly.size()<<endl;
            //    v3d_msg("check two size");
        }

            cout<<"t = "<<t.x<<"  "<<t.y<<"  "<<t.z<<"  "<<endl;
            cout<<"simple_rebase = "<<true_rebase.x<<"  "<<true_rebase.y<<"  "<<true_rebase.z<<"  "<<endl;
        double diff = (t.x-true_rebase.x)*(t.x-true_rebase.x)+(t.y-true_rebase.y)*(t.y-true_rebase.y)+(t.z-true_rebase.z)*(t.z-true_rebase.z);//0524
        cout<<"diff = "<<diff<<endl;
        true_rebase = t;//0524
        if(diff<0.001)
        {
            v3d_msg("the same marker!");
            change = false;
            bool miok;
            thresh = QInputDialog::getInt(0,"Intensity Threshold 1%-99%","please input your number",42,1,200,5,&miok);
            if(miok)
            {
                cout<<"input number is "<<thresh<<endl;
            }
            else
            {
                return false;
                //callback.setSWCTeraFly(resultTree_rebase);
            }
            cout<<"thresh = "<<thres_rebase<<endl;
            if(thresh != thres_rebase)
            {
                callback.setSWCTeraFly(resultTree_rebase);
            }
            else
            {
                v3d_msg("the same thresh,please try again!");
                return false;
            }
            thres_rebase = thresh;
        }
        cout<<t.x<<"  "<<t.y<<"  "<<t.z<<endl;
      //  v3d_msg("show_marker");
        //callback.setLandmarkTeraFly(marker_rebase);
        marker_rebase2 = terafly_landmarks_terafly;   // 20180522

        t.x = t.x-P.o_x;
        t.y = t.y-P.o_y;
        t.z = t.z-P.o_z;
        t.x = t.x/P.ratio_x;
        t.y = t.y/P.ratio_y;
        t.z = t.z/P.ratio_z;
        P.listLandmarks.clear();
        P.listLandmarks.push_back(t);
        cout<<"P = "<<P.listLandmarks.size()<<endl;

        QList<ImageMarker> marker;
        ImageMarker markerlist;
        markerlist.x = t.x;
        markerlist.y = t.y;
        markerlist.z = t.z;
        marker.push_back(markerlist);

        writeMarker_file(outmarker_file,marker);


        Image4DSimple *data = new Image4DSimple;
        data->setData((unsigned char *)curr_block->getRawData(),curr_block->getXDim(),curr_block->getYDim(),curr_block->getZDim(),curr_block->getCDim(),V3D_UINT8);
        data1d_sz[0] = data->getXDim();
        data1d_sz[1] = data->getYDim();
        data1d_sz[2] = data->getZDim();
        data1d_sz[3] = data->getCDim();

        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)data->getRawData(),data1d_sz,1);

        //bool menu_enhance = true;
        input_PARA PARA;
        QString coord_name = QString::number(curr_block->getOriginX())+QString::number(curr_block->getOriginY())+QString::number(curr_block->getOriginZ());
        PARA.inimg_file = outimg_file;
        QString str_outimg_filename = outimg_file + "_anisodiff.raw";
         fstream _file;
         _file.open(str_outimg_filename.toStdString().c_str(),ios::in);
        if(!_file)
        {
<<<<<<< HEAD
            //v3d_msg("gggggg");
            if(enhance)
            {
                //v3d_msg("llllllllllll");
                thresh = 30;
                PARA.channel = 1;
                anisodiff_func(callback,parent,PARA,bmenu);
=======
            if(enhance)
            {
                thresh = 30;
                PARA.channel = 1;
                anisodiff_func(callback,parent,PARA,bmenu);

>>>>>>> remove_glio version which is operational
            }
        }


        V3DLONG all_volume = data1d_sz[0]*data1d_sz[1]*data1d_sz[2];
        cout<<"all_volume = "<<all_volume<<endl;
        //v3d_msg("check all volume!");
        double c = 0;
        double d = 0;
        for(V3DLONG i=0;i<all_volume;i++)
        {

            if((int)curr_block->getRawData()[i]>thresh)
            {
                //cout<<"(int)curr_block->getRawData()[i] = "<<(int)curr_block->getRawData()[i]<<endl;
                c++;
            }
            if((int)curr_block->getRawData()[i]>60)
            {
                //cout<<"(int)curr_block->getRawData()[i] = "<<(int)curr_block->getRawData()[i]<<endl;
                d++;
            }
        }
        cout<<"c/all_volume = "<<c/all_volume<<endl;
        cout<<"d        /all_volume = "<<d/all_volume<<endl;
        //v3d_msg("check!");
        if(c/all_volume>0.3)
        {
            v3d_msg("thresh is too low!");
            return false;
        }
        if(d/all_volume>0.3)
        {
            is_soma = true;
        }
//        for(V3DLONG i=0;i<all_volume;i++)
//        {
//            if((int)curr_block->getRawData()[i]<10)
//            {
//                int last_point1 = curr_block->getRawData()[i-1];
//                int next_point1 = curr_block->getRawData()[i+1];
//                int last_point2 = curr_block->getRawData()[i-2];
//                int next_point2 = curr_block->getRawData()[i+2];
//                int average = (last_point1+last_point2+next_point1+next_point2)/4;
//            }

//        }
        PARA_APP2 p2;
        QString versionStr = "v0.001";
        p2.inmarker_file = outmarker_file;
        cout<<"p2"<<p2.inmarker_file.size()<<endl;
        if(enhance)
        {
<<<<<<< HEAD
            p2.inimg_file = str_outimg_filename;
=======
            //p2.inimg_file = str_outimg_filename;
            p2.inimg_file = "final_img.v3draw";
>>>>>>> remove_glio version which is operational
        }
        else
        {
            p2.inimg_file = outimg_file;
        }
        p2.bkg_thresh = thresh;
        p2.is_gsdt = true;
        p2.b_resample = 0;
        cout<<" iiiiiiiiiiiiiiiiiiiiii = "<<thresh<<endl;
        //v3d_msg("start app2");
        p2.outswc_file =swcString;
        if(!proc_app2(callback, p2, versionStr))
        {
            return false;
        }
        //thresh=30;

    }
    enhance = false;

    //v3d_msg("app2_done");











    return true;
}




bool grid_raw_all(V3DPluginCallback2 &callback, QWidget *parent,TRACE_LS_PARA &P,bool bmenu)
{
    cout<<"this is grid_raw_all"<<endl;
    QElapsedTimer timer1;
    timer1.start();

    QString outimg_file = "test.v3draw";
    V3DLONG data1d_sz[4];
    if(bmenu)
    {
        double center_x,center_y,center_z;
        V3DLONG xe,xb,ye,yb,ze,zb;
        V3DLONG sc = 1;
        LandmarkList terafly_landmarks = callback.getLandmarkTeraFly();
        if(P.markerfilename.isEmpty())
        {
            for(V3DLONG i=0;i<terafly_landmarks.size();i++)
            {
                center_x = terafly_landmarks[i].x;
                center_y = terafly_landmarks[i].y;
                center_z = terafly_landmarks[i].z;
            }
        }
        else
        {
            if(P.markerfilename.endsWith(".marker",Qt::CaseSensitive))
            {

                vector<MyMarker> markers;
                markers = readMarker_file(string(qPrintable(P.markerfilename)));
                for(int i = 0; i < markers.size(); i++)
                {
                    center_x = markers[i].x;
                    center_y = markers[i].y;
                    center_z = markers[i].z;
                }
            }
            else
            {
                QList<CellAPO> markers;
                markers = readAPO_file(P.markerfilename);
                for(int i = 0; i < markers.size(); i++)
                {
                    center_x = markers[i].x;
                    center_y = markers[i].y;
                    center_z = markers[i].z;
                }
            }
        }

        V3DLONG lens_x=P.block_size;
        V3DLONG lens_y=P.block_size;
        V3DLONG lens_z=P.block_size;
        xb = center_x-lens_x;
        xe = center_x+lens_x;
        yb = center_y-lens_y;
        ye = center_y+lens_y;
        zb = center_z-lens_z;
        ze = center_z+lens_z;
        data1d_sz[0] = xe - xb + 1;
        data1d_sz[1] = ye - yb + 1;
        data1d_sz[2] = ze - zb + 1;
        data1d_sz[3] = sc;
        P.o_x = xb;
        P.o_y = yb;
        P.o_z = zb;
        unsigned char *data1d=0;
        data1d = callback.getSubVolumeTeraFly(P.inimg_file.toStdString(),xb,xe+1,yb,ye+1,zb,ze+1);
        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)data1d,data1d_sz,1);
    }
    else
    {
        const Image4DSimple *curr_block = callback.getImageTeraFly();
        data1d_sz[0] = curr_block->getXDim();
        data1d_sz[1] = curr_block->getYDim();
        data1d_sz[2] = curr_block->getZDim();
        data1d_sz[3] = curr_block->getCDim();

        P.ratio_x = curr_block->getRezX()/curr_block->getXDim();
        P.ratio_y = curr_block->getRezY()/curr_block->getYDim();
        P.ratio_z = curr_block->getRezZ()/curr_block->getZDim();
        P.o_x=curr_block->getOriginX();
        P.o_y=curr_block->getOriginY();
        P.o_z=curr_block->getOriginZ();

        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)curr_block->getRawData(),data1d_sz,1);


    }


    QString fileOpenName = outimg_file;


    if(fileOpenName.endsWith(".tc",Qt::CaseSensitive))
    {
        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

        if( !vim.y_load( P.inimg_file.toStdString()) )
        {
            printf("Wrong stitching configuration file to be load!\n");
            return false;
        }

        P.in_sz[0] = vim.sz[0];
        P.in_sz[1] = vim.sz[1];
        P.in_sz[2] = vim.sz[2];

    }else if (fileOpenName.endsWith(".raw",Qt::CaseSensitive) || fileOpenName.endsWith(".v3draw",Qt::CaseSensitive))
    {
        unsigned char * datald = 0;
        V3DLONG *in_zz = 0;
        V3DLONG *in_sz = 0;
        int datatype;
        //if (!loadRawRegion(const_cast<char *>(P.inimg_file.toStdString().c_str()), datald, in_zz, in_sz,datatype,0,0,0,1,1,1))
        if (!loadRawRegion(const_cast<char *>(fileOpenName.toStdString().c_str()), datald, in_zz, in_sz,datatype,0,0,0,1,1,1))
        {
            return false;
        }
        if(datald) {delete []datald; datald = 0;}
        P.in_sz[0] = data1d_sz[0];
        P.in_sz[1] = data1d_sz[1];
        P.in_sz[2] = data1d_sz[2];
        cout<<P.in_sz[0]<<"  "<<P.in_sz[1]<<"  "<<P.in_sz[2]<<"  "<<endl;
    }else
    {
        V3DLONG *in_zz = 0;
        if(!callback.getDimTeraFly(fileOpenName.toStdString(),in_zz))
        {
            return false;
        }
        P.in_sz[0] = in_zz[0];
        P.in_sz[1] = in_zz[1];
        P.in_sz[2] = in_zz[2];
    }

    P.inimg_file = outimg_file;
    all_tracing_grid(callback,P,P.in_sz[0],P.in_sz[1],P.in_sz[2]);
    //all_tracing_grid(callback,P,data1d_sz[0],data1d_sz[1],data1d_sz[2]);


    return true;
}

bool all_tracing_grid(V3DPluginCallback2 &callback,TRACE_LS_PARA &P,V3DLONG ix, V3DLONG iy, V3DLONG iz)
{
//    cout<<"name = "<<P.inimg_file.toStdString()<<endl;
    QString saveDirString;


    QString imageSaveString = saveDirString;

    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    start_x = ix;
    start_y = iy;
    start_z = iz;
    end_x = ix + P.block_size; if(end_x > P.in_sz[0]) end_x = P.in_sz[0];
    end_y = iy + P.block_size; if(end_y > P.in_sz[1]) end_y = P.in_sz[1];
    end_z = iz + P.block_size; if(end_z > P.in_sz[2]) end_z = P.in_sz[2];


    QString swcString = saveDirString;

    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    QString full_plugin_name;
    QString func_name;

    arg.type = "random";
    std::vector<char*> arg_input;
    std:: string fileName_Qstring(P.inimg_file.toStdString());
    char* fileName_string =  new char[fileName_Qstring.length() + 1];
    strcpy(fileName_string, fileName_Qstring.c_str());
    //std:: string fileName_Qstring(imageSaveString.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
    arg_input.push_back(fileName_string);
    cout<<"fileName_string = "<<fileName_string<<endl;
    arg.p = (void *) & arg_input;
    input<< arg;

    char* char_swcout =  new char[swcString.length() + 1];strcpy(char_swcout, swcString.toStdString().c_str());
    arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(char_swcout); arg.p = (void *) & arg_output; output<< arg;

    arg.type = "random";
    std::vector<char*> arg_para;
    if(P.method == neutube || P.method == app2)
    {
        arg_para.push_back("1");
        arg_para.push_back("1");
        full_plugin_name = "neuTube";
        func_name =  "neutube_trace";
    }else if(P.method == snake)
    {
        arg_para.push_back("1");
        arg_para.push_back("1");
        full_plugin_name = "snake";
        func_name =  "snake_trace";
    }else if(P.method == most)
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
    QString swcNEUTUBE = saveDirString;

    vector<MyMarker*> inputswc;
    inputswc = readSWC_file(swcNEUTUBE.toStdString());;
    for(V3DLONG d = 0; d < inputswc.size(); d++)
    {
        inputswc[d]->x = inputswc[d]->x + start_x;
        inputswc[d]->y = inputswc[d]->y + start_y;
        inputswc[d]->z = inputswc[d]->z + start_z;

    }
    cout<<"swcString.toStdString() = "<<swcString.toStdString()<<endl;
    saveSWC_file(swcString.toStdString().c_str(), inputswc);
    system(qPrintable(QString("rm %1").arg(swcNEUTUBE.toStdString().c_str())));
    system(qPrintable(QString("rm %1").arg(imageSaveString.toStdString().c_str())));
}


QString getAppPath()
{
    QString v3dAppPath("~/Work/v3d_external/v3d");
    QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
    }
#endif

    v3dAppPath = testPluginsDir.absolutePath();
    return v3dAppPath;
}




bool export_2dtif(V3DPluginCallback & cb,const char * filename, unsigned char * pdata, V3DLONG sz[3], int datatype)
{
    if (!filename || !sz || !pdata)
    {
        v3d_msg("some of the parameters for simple_saveimage_wrapper() are not valid.", 0);
        return false;
    }

    ImagePixelType dt;
    if (datatype==1)
        dt = V3D_UINT8;
    else if (datatype==2)
        dt = V3D_UINT16;
    else if (datatype==4)
        dt = V3D_FLOAT32;
    else
    {
        v3d_msg(QString("the specified save data type in simple_saveimage_wrapper() is not valid, dt=[%1].").arg(datatype), 0);
        return false;
    }

    Image4DSimple * outimg = new Image4DSimple;
    if (outimg)
    {
        cout<<"save"<<endl;
        outimg->setData(pdata, sz[0], sz[1], 0 ,sz[3], dt);
    }
    else
    {
        v3d_msg("Fail to new Image3DSimple for outimg.");
        return false;
    }

    return cb.saveImage(outimg, (char *)filename);
}
bool anisodiff_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* p_img_input = 0;
    V3DLONG sz_img_input[4];
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return false;
        }
        Image4DSimple* p4DImage = callback.getImage(curwin);
        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return false;
        }
        if(p4DImage->getDatatype()!=V3D_UINT8)
        {
            QMessageBox::information(0, "", "Please convert the image to be UINT8 and try again!");
            return false;
        }
        if(p4DImage->getCDim()!=1)
        {
            QMessageBox::information(0, "", "The input image is not one channel image!");
            return false;
        }
        p_img_input = p4DImage->getRawData();
        sz_img_input[0] = p4DImage->getXDim();
        sz_img_input[1] = p4DImage->getYDim();
        sz_img_input[2] = p4DImage->getZDim();
        sz_img_input[3] = 1;
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), p_img_input, sz_img_input, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return false;
        }
        if(PARA.channel < 1 || PARA.channel > sz_img_input[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return false;
        }

        if(datatype !=1)
        {
            fprintf (stderr, "Please convert the image to be UINT8 and try again!\n");
            return false;
        }
    }
    printf("0. Make some backgrand pixels as 0 \n");
    V3DLONG pagesz = sz_img_input[0]*sz_img_input[1]*sz_img_input[2];
    for(V3DLONG i=0;i<pagesz;i++)
    {
        if(int(p_img_input[i])<15)
        {
            p_img_input[i]=0;
        }
    }
    //-----------------------------------------------------------------------------------------
    printf("1. Find the bounding box and crop image. \n");
    long l_boundbox_min[3],l_boundbox_max[3];//xyz
    V3DLONG sz_img_crop[4];
    long l_npixels_crop;
    unsigned char *p_img8u_crop=0;
    {
    //find bounding box
    unsigned char ***p_img8u_3d=0;
    if(!new3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2],p_img_input))
    {
        printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
        if(p_img8u_3d) 				{delete3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2]);}
        return false;
    }

    l_boundbox_min[0]=sz_img_input[0];	l_boundbox_min[1]=sz_img_input[1];	l_boundbox_min[2]=sz_img_input[2];
    l_boundbox_max[0]=0;				l_boundbox_max[1]=0;				l_boundbox_max[2]=0;
    for(long X=0;X<sz_img_input[0];X++)
        for(long Y=0;Y<sz_img_input[1];Y++)
            for(long Z=0;Z<sz_img_input[2];Z++)
                if(p_img8u_3d[Z][Y][X]>0.1)
                {
                    if(l_boundbox_min[0]>X) l_boundbox_min[0]=X;	if(l_boundbox_max[0]<X) l_boundbox_max[0]=X;
                    if(l_boundbox_min[1]>Y) l_boundbox_min[1]=Y;	if(l_boundbox_max[1]<Y) l_boundbox_max[1]=Y;
                    if(l_boundbox_min[2]>Z) l_boundbox_min[2]=Z;	if(l_boundbox_max[2]<Z) l_boundbox_max[2]=Z;
                }
    printf(">>boundingbox: x[%ld~%ld],y[%ld~%ld],z[%ld~%ld]\n",l_boundbox_min[0],l_boundbox_max[0],
                                                               l_boundbox_min[1],l_boundbox_max[1],
                                                               l_boundbox_min[2],l_boundbox_max[2]);

    //crop image
    sz_img_crop[0]=l_boundbox_max[0]-l_boundbox_min[0]+1;
    sz_img_crop[1]=l_boundbox_max[1]-l_boundbox_min[1]+1;
    sz_img_crop[2]=l_boundbox_max[2]-l_boundbox_min[2]+1;
    sz_img_crop[3]=1;
    l_npixels_crop=sz_img_crop[0]*sz_img_crop[1]*sz_img_crop[2];

    p_img8u_crop=new(std::nothrow) unsigned char[l_npixels_crop]();
    if(!p_img8u_crop)
    {
        printf("ERROR: Fail to allocate memory for p_img32f_crop!\n");
        if(p_img8u_3d) 				{delete3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2]);}
        return false;
    }
    unsigned char *p_tmp=p_img8u_crop;
    for(long Z=0;Z<sz_img_crop[2];Z++)
        for(long Y=0;Y<sz_img_crop[1];Y++)
            for(long X=0;X<sz_img_crop[0];X++)
            {
                *p_tmp = p_img8u_3d[Z+l_boundbox_min[2]][Y+l_boundbox_min[1]][X+l_boundbox_min[0]];
                p_tmp++;
            }
    if(p_img8u_3d) 			{delete3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2]);}
    }
    //saveImage("d:/SVN/Vaa3D_source_code/v3d_external/released_plugins/v3d_plugins/anisodiffusion_littlequick/crop.raw",p_img8u_crop,sz_img_crop,1);

    //-----------------------------------------------------------------------------------------
    //convert image data type to float
    printf("2. Convert image data to float and scale to [0~255]. \n");
    float *p_img32f_crop=0;
    {
    p_img32f_crop=new(std::nothrow) float[l_npixels_crop]();
    if(!p_img32f_crop)
    {
        printf("ERROR: Fail to allocate memory for p_img32f_crop!\n");
        if(p_img8u_crop) 			{delete []p_img8u_crop;		p_img8u_crop=0;}
        if(p_img32f_crop) 			{delete []p_img32f_crop;			p_img32f_crop=0;}
        return false;
    }
    //find the maximal intensity value
    float d_maxintensity_input=0.0;
    for(long i=0;i<l_npixels_crop;i++)
        if(p_img8u_crop[i]>d_maxintensity_input)
            d_maxintensity_input=p_img8u_crop[i];
    //convert and rescale
    for(long i=0;i<l_npixels_crop;i++)
        p_img32f_crop[i]=p_img8u_crop[i]/d_maxintensity_input*255.0;
    printf(">>d_maxintensity=%.2f\n",d_maxintensity_input);
    //free input image to save memory
    //if(p_img_input) 			{delete []p_img_input;		p_img_input=0;}
    if(p_img8u_crop) 			{delete []p_img8u_crop;		p_img8u_crop=0;}
    }

    //-----------------------------------------------------------------------------------------
    //do anisotropic diffusion
    printf("3. Do anisotropic diffusion... \n");
    float *p_img32f_crop_output=0;
    if(!q_AnisoDiff3D(p_img32f_crop,sz_img_crop,p_img32f_crop_output))
    {
        printf("ERROR: q_AnisoDiff3D() return false!\n");
        if(p_img8u_crop) 			{delete []p_img8u_crop;		p_img8u_crop=0;}
        if(p_img32f_crop) 				{delete []p_img32f_crop;		p_img32f_crop=0;}
        if(p_img32f_crop_output) 		{delete []p_img32f_crop_output;	p_img32f_crop_output=0;}
        return false;
    }
    if(p_img32f_crop) 				{delete []p_img32f_crop;		p_img32f_crop=0;}

    //-----------------------------------------------------------------------------------------
    printf("4. Reconstruct processed crop image back to original size. \n");
    unsigned char *p_img8u_output=0;
    long l_npixels=sz_img_input[0]*sz_img_input[1]*sz_img_input[2]*sz_img_input[3];
    {
    p_img8u_output=new(std::nothrow) unsigned char[l_npixels]();
    if(!p_img8u_output)
    {
        printf("ERROR: Fail to allocate memory for p_img8u_output!\n");
        if(p_img32f_crop_output) 		{delete []p_img32f_crop_output;	p_img32f_crop_output=0;}
        return false;
    }
    //copy original image data to output image
    for(long i=0;i<l_npixels;i++)
        p_img8u_output[i]=p_img_input[i];

    unsigned char ***p_img8u_3d=0;
    if(!new3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2],p_img8u_output))
    {
        printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
        if(p_img8u_output) 				{delete []p_img8u_output;	p_img8u_output=0;}
        if(p_img32f_crop_output) 		{delete []p_img32f_crop_output;	p_img32f_crop_output=0;}
        return false;
    }

    float *p_tmp=p_img32f_crop_output;
    for(long Z=0;Z<sz_img_crop[2];Z++)
        for(long Y=0;Y<sz_img_crop[1];Y++)
            for(long X=0;X<sz_img_crop[0];X++)
            {
                p_img8u_3d[Z+l_boundbox_min[2]][Y+l_boundbox_min[1]][X+l_boundbox_min[0]]=(unsigned char)(*p_tmp);
                p_tmp++;
            }

    if(p_img8u_3d) 	{delete3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2]);}
    if(p_img32f_crop_output) 		{delete []p_img32f_crop_output;	p_img32f_crop_output=0;}
    }

    //-----------------------------------------------------------------------------------------
    //save or display
    if(bmenu)
    {
        printf("5. Display the processed image in Vaa3D. \n");
        //push result image back to v3d
        v3dhandle newwin=callback.newImageWindow("output");
        Image4DSimple img4D_output;
        img4D_output.setData(p_img8u_output,sz_img_input[0],sz_img_input[1],sz_img_input[2],1,V3D_UINT8);
        callback.setImage(newwin,&img4D_output);
        callback.updateImageWindow(newwin);
        callback.open3DWindow(newwin);
    }
    else
    {
        printf("5. Save the processed image to file. \n");
        QString str_outimg_filename = PARA.inimg_file + "_anisodiff.raw";
<<<<<<< HEAD
        saveImage(qPrintable(str_outimg_filename),p_img8u_output,sz_img_input,1);
=======
        //saveImage(qPrintable(str_outimg_filename),p_img8u_output,sz_img_input,1);
        soma_remove_main_2(p_img8u_output,sz_img_input,callback);
>>>>>>> remove_glio version which is operational

        if(p_img8u_output) 		{delete []p_img8u_output;		p_img8u_output=0;}
    }

    printf(">>Program complete success!\n");
    return true;
}
