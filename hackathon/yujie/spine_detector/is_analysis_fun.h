#ifndef IS_ANALYSIS_FUN_H
#define IS_ANALYSIS_FUN_H

#include <QtGui>
#include <QDialog>
#include "v3d_interface.h"
#include "vector"
#include "map"

using namespace std;

struct is_parameters{
    int red_bgthr; //background threshold
    int green_bgthr;
    int max_dis;
    int min_voxel;
    int halfwindowsize;
};

struct GOI{
    vector<V3DLONG> voxel;
    int label_id;
    int nearest_node;
    bool on_dendrite;
    GOI(vector<V3DLONG> one_group,int id)
    {
        voxel=one_group;
        nearest_node=-1;
        on_dendrite=false;
        label_id=id;
    }
};



class is_analysis_fun : public QDialog
{
    Q_OBJECT

public:
    explicit is_analysis_fun(V3DPluginCallback2 *cb,QStringList name_list);
    void run();
    void create_proofread_panel();
private:
    V3DPluginCallback2 * callback;
    V3DLONG sz_img[4],mask_sz[4],sz[4];
    NeuronTree nt;
    unsigned char * p_img1D, *mask,*image_trun;
    //unsigned char ***ppp_img3D;
    int type_img;
    is_parameters param;
    vector<GOI> voxel_groups;
    map<V3DLONG,bool> voxels_map;
    LandmarkList LList_out,LList_adj;
    QDialog *mydialog;
    QListWidget *list_markers;
    QPlainTextEdit *edit_box;
    int x_start,y_start,z_start,x_end,y_end,z_end;
    v3dhandle main_win,curwin;
    QString basedir,swc_name,image_name,csv_out_name;

private:
    void obtain_mask();
    void connected_components();
    void stat_generate();
    void visualize_image();

    void set_trunc_image(); //not used
    void adjust_LList_to_imagetrun(); //not used
    void open_main_win();
    void open_cur_win();  //not used
    bool check_cur_win();   //not used
    bool csv_generate();
    int finish_implement();

public slots:
    void marker_doubleclicked(); //not used
    void reject_marker();
    void set_on_spine();
    void set_on_dendrite();
    void finish_dialog();

public:
    bool pushImageData(unsigned char * data1Dc_in, V3DLONG new_sz_img[4])
    {
        //qDebug()<<"in push imageData";
//        if(ppp_img3D!=0){
//            delete [] ppp_img3D; ppp_img3D=0;
//        }
        sz_img[0]=new_sz_img[0];
        sz_img[1]=new_sz_img[1];
        sz_img[2]=new_sz_img[2];
        sz_img[3]=new_sz_img[3];

        //we know which channel to select
        //need to check this channel whether there are voxels

        V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        p_img1D = new unsigned char [size_tmp];
        for(V3DLONG i=0; i<size_tmp; i++){
            p_img1D[i] = data1Dc_in[i];
        }
        //arrange the image into 3D
//        ppp_img3D=new unsigned char ** [sz_img[2]];
//        for(V3DLONG z=0; z<sz_img[2]; z++){
//            ppp_img3D[z]=new unsigned char * [sz_img[1]];
//            for(V3DLONG y=0; y<sz_img[1]; y++){
//                ppp_img3D[z][y]=p_img1D+y*sz_img[0]+z*sz_img[0]*sz_img[1];
//            }
//        }
        qDebug()<<"in pushnewdata:"<<sz_img[0]<<":"<<sz_img[1]<<":"<<sz_img[2];
        return true;
    }

    void pushSWCData(NeuronTree neuron)
    {
        nt.copy(neuron);
        qDebug()<<"nt size:"<<nt.listNeuron.size();
    }
};

#endif // IS_ANALYSIS_FUN_H
