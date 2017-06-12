/* line_detector_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-5-25 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "volimg_proc.h"

#include "line_detector_plugin.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h""
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "stackutil.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h"
#include "hang/topology_analysis.h"
#include "../AllenNeuron_postprocessing/sort_swc_IVSCC.h"
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"


Q_EXPORT_PLUGIN2(line_detector, line_detector);

using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(NTDIS(a,b)*NTDIS(a,c)))*180.0/3.14159265359)
struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    LandmarkList listLandmarks;
    int win_size;
    NeuronTree nt;
    QString outswc_file;
    QList<NeuronSWC> nt_last;

    input_PARA() {win_size=0; channel=0;}
};

//return -1: wrong running parameters, etc.
//return 0: exit normally
//return 1: exit when hit bounadry
int reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList line_detector::menulist() const
{
	return QStringList() 
        <<tr("GD Curveline")
        <<tr("GD Curveline infinite")
        <<tr("about");
}

QStringList line_detector::funclist() const
{
	return QStringList()
        <<tr("GD_curveline")
        <<tr("GD_Curveline_infinite")
        <<tr("help");
}

void line_detector::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("GD Curveline"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (menu_name == tr("GD Curveline infinite"))
    {
        bool bmenu = true;
        input_PARA PARA;
        for (;;)
        {
            printf("\n\n~~~~~~~~~ Enter again @@@@@@@@@@@@@@@@@@@\n\n");
            int res = reconstruction_func(callback,parent,PARA,bmenu);
            if (res!=0)
                break;
        }
    }
    else
	{
        v3d_msg(tr("A small curvelinear structure detector based on GD, 2017-5-25"));
	}
}

bool line_detector::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("GD_Curveline_infinite"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];

        PARA.outswc_file = outfiles[0];        outfiles.empty() ? "" : outfiles[0];
        int k=0;
        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        vector<MyMarker> file_inmarkers;
        if(!inmarker_file.isEmpty())
            file_inmarkers = readMarker_file(string(qPrintable(inmarker_file)));

        LocationSimple t;
        for(int i = 0; i < file_inmarkers.size(); i++)
        {
            t.x = file_inmarkers[i].x+1;
            t.y = file_inmarkers[i].y+1;
            t.z = file_inmarkers[i].z+1;
            PARA.listLandmarks.push_back(t);
        }
        PARA.win_size = (paras.size() >= k+1) ? atoi(paras[k]) : 32;  k++;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        for (;;)
        {
            int res = reconstruction_func(callback,parent,PARA,bmenu);
            if (res!=0)
                break;
        }
    }
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of line_detector tracing **** \n");
        printf("vaa3d -x line_detector -f GD_curveline -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

int reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    v3dhandle curwin;
    if(bmenu)
    {
        curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return -1;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return -1;
        }

        PARA.listLandmarks = callback.getLandmark(curwin);
        if(PARA.listLandmarks.count() ==0)
        {
            QMessageBox::information(0, "", "No markers in the current image, please select a marker.");
            return -1;
        }

        if(PARA.win_size ==0)
        {
            bool ok;
            PARA.win_size = QInputDialog::getInteger(parent, "Window radius",
                                                     "Enter radius (window size is 2*radius+1):",
                                                     32, 16, 512, 1, &ok);
            if (!ok)
                return -1;
        }

        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();

        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return -1;

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;

        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return -1;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return -1;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //GD_tracing
    LocationSimple p0;
    vector<LocationSimple> pp;
    NeuronTree nt;

    double weight_xy_z=1.0;
    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_background = 0;
    trace_para.b_postMergeClosebyBranches = false;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_pruneArtifactBranches = false;
    trace_para.sp_num_end_nodes = 2;
    trace_para.b_deformcurve = false;
    trace_para.sp_graph_resolution_step = 1;

    int markSize = PARA.listLandmarks.size();
    p0.x = PARA.listLandmarks.at(markSize-1).x-1;
    p0.y = PARA.listLandmarks.at(markSize-1).y-1;
    p0.z = PARA.listLandmarks.at(markSize-1).z-1;

    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    start_x = (p0.x - PARA.win_size < 0)?  0 : (p0.x - PARA.win_size);
    start_y = (p0.y - PARA.win_size < 0)?  0 : (p0.y - PARA.win_size);
    start_z = (p0.z - PARA.win_size < 0)?  0 : (p0.z - PARA.win_size);

    end_x = (p0.x + PARA.win_size >= N-1)?  (N-1) : (p0.x + PARA.win_size);
    end_y = (p0.y + PARA.win_size >= M-1)?  (M-1) : (p0.y + PARA.win_size);
    end_z = (p0.z + PARA.win_size >= P-1)?  (P-1) : (p0.z + PARA.win_size);

    NeuronTree nt_original;
    if(bmenu)
    {
        nt_original = callback.getSWC(curwin);
    }
    else
        nt_original = PARA.nt;
    V3DLONG stacksz = N*M*P*sc;
    unsigned char* data1d_mask = 0;
    data1d_mask = new unsigned char [stacksz];

    memset(data1d_mask,0,stacksz*sizeof(unsigned char));

    double margin=2;//by PHC 20170531
    if(nt_original.listNeuron.size()>0)
        ComputemaskImage(nt_original, data1d_mask, N, M, P, margin);

    unsigned char *localarea=0;
    V3DLONG blockpagesz = (end_x-start_x+1)*(end_y-start_y+1)*(end_z-start_z+1);
    localarea = new unsigned char [blockpagesz];
    V3DLONG d = 0;
    for(V3DLONG iz = start_z; iz <= end_z; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = start_y; iy <= end_y; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = start_x; ix <= end_x; ix++)
            {
                localarea[d++] = (data1d_mask[offsetk + offsetj + ix] == 0) ? data1d[offsetk + offsetj + ix] : 1;
//                localarea[d++] = data1d[offsetk + offsetj + ix]; //disable the masking and copy the data as is // by PHC 20170607
            }
        }
    }

    if(data1d_mask) {delete []data1d_mask; data1d_mask=0;}
    V3DLONG sz_tracing[4];
    sz_tracing[0] = end_x-start_x+1;
    sz_tracing[1] = end_y-start_y+1;
    sz_tracing[2] = end_z-start_z+1;
    sz_tracing[3] = 1;

    unsigned char ****p4d = 0;
    if (!new4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], localarea))
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        if(localarea) {delete []localarea; localarea = 0;}
        if(p4d) {delete []p4d; p4d = 0;}
        return -1;
    }
    p0.x -= start_x;
    p0.y -= start_y;
    p0.z -= start_z;

    LocationSimple tmpp;
    vector<MyMarker> file_inmarkers;

    for(V3DLONG ix = 0; ix <=2; ix++)
    {
        tmpp.x = (2-ix)*start_x/2 + ix*end_x/2 - start_x;
        if(tmpp.x < 0) tmpp.x = 0;
        if(tmpp.x > sz_tracing[0]-1) tmpp.x = sz_tracing[0]-1;
        for(V3DLONG iy = 0; iy <=2; iy++)
        {
            tmpp.y = (2-iy)*start_y/2 + iy*end_y/2 - start_y;
            if(tmpp.y < 0) tmpp.y = 0;
            if(tmpp.y > sz_tracing[1]-1) tmpp.y = sz_tracing[1]-1;
            for(V3DLONG iz = 0; iz <=2; iz++)
            {
                tmpp.z = (2-iz)*start_z/2 + iz*end_z/2 - start_z;
                if(tmpp.z < 0) tmpp.z = 0;
                if(tmpp.z > sz_tracing[2]-1) tmpp.z = sz_tracing[2]-1;
                if(!(ix==1 && iy ==1 && iz==1))
                {
                    MyMarker t;
                    t.x = tmpp.x+start_x;
                    t.y = tmpp.y+start_y;
                    t.z = tmpp.z+start_z;
                    file_inmarkers.push_back(t);
                    pp.push_back(tmpp);
                }
            }
        }
    }

    //170606 add favorite direction, by PHC
    if (PARA.nt_last.size()<=10) //ensure there are at least 2 nodes in the existing swc so that we can calculate a relatively meaningful directional vector
    {
        v3d_msg("\n\nset trace_para facorite direction to false==============\n\n",0);

        trace_para.b_use_favorite_direction = false;
    }
    else
    {
        v3d_msg("\n\nset trace_para facorite direction to true!!!!!!!!!!!!!!!!\n\n",0);

        trace_para.b_use_favorite_direction = true;

        V3DLONG nt_length = PARA.nt_last.size();
        if(nt_length>10)
        {
            trace_para.favorite_direction[0] = ((PARA.nt_last.at(nt_length-1).x - PARA.nt_last.at(0).x));
            trace_para.favorite_direction[1] = ((PARA.nt_last.at(nt_length-1).y - PARA.nt_last.at(0).y));
            trace_para.favorite_direction[2] = ((PARA.nt_last.at(nt_length-1).z - PARA.nt_last.at(0).z));

            double halfway_direction[3];
            halfway_direction[0] = ((PARA.nt_last.at(nt_length-1).x - PARA.nt_last.at(nt_length/2).x));
            halfway_direction[1] = ((PARA.nt_last.at(nt_length-1).y - PARA.nt_last.at(nt_length/2).y));
            halfway_direction[2] = ((PARA.nt_last.at(nt_length-1).z - PARA.nt_last.at(nt_length/2).z));

            double cangle = cosangle_two_vectors(trace_para.favorite_direction, halfway_direction);
            if (cangle!=-2 && cangle <=0.5) //0.5 is 45 degree. do not get confused, -2 is a special return value to indicate status of cosangle_two_vectors()
            {
                trace_para.b_use_favorite_direction = false;
                v3d_msg("Detected non-colinear case, reset trace_para facorite direction = false true!!!!!!!!!!!!!!!!\n\n",0);
            }
        }

        printf("dd = %d %5.3f %5.3f %5.3f \n", nt_length, trace_para.favorite_direction[0], trace_para.favorite_direction[1], trace_para.favorite_direction[2]);
    }

//    simple_saveimage_wrapper(callback, "/opt/zhi/Desktop/test.v3draw",(unsigned char *)localarea, sz_tracing, 1);
//    saveMarker_file("/opt/zhi/Desktop/test.marker",file_inmarkers);
    nt = v3dneuron_GD_tracing(p4d, sz_tracing,
                              p0, pp,
                              trace_para, weight_xy_z);


    QString swc_name = PARA.inimg_file + QString("_x_%1_y_%2_z_%3_line_detector.swc").arg(PARA.listLandmarks.at(0).x).arg(PARA.listLandmarks.at(0).y).arg(PARA.listLandmarks.at(0).z);
    nt.name = "line_detector";
    for(V3DLONG i = 1; i < nt.listNeuron.size();i++)
    {
        V3DLONG pn_ID = nt.listNeuron[i].pn-1;
        if(pn_ID <0)
            continue;
        if((fabs(nt.listNeuron[pn_ID].x-nt.listNeuron[0].x) +
            fabs(nt.listNeuron[pn_ID].y-nt.listNeuron[0].y) +
            fabs(nt.listNeuron[pn_ID].z-nt.listNeuron[0].z)) < 1e-3)
        {
                nt.listNeuron[i].pn = 1;
        }
    }
    QList<NeuronSWC> neuron_sorted;
    if (!SortSWC(nt.listNeuron, neuron_sorted, VOID, 0))
    {
        v3d_msg("fail to call swc sorting function.");
        if(localarea) {delete []localarea; localarea = 0;}
        if(p4d) {delete []p4d; p4d = 0;}
        return -1;
    }
    export_list2file(neuron_sorted, swc_name,swc_name);
    if(neuron_sorted.size() <=0)
    {
        v3d_msg("cannot find any path based on the given marker.");
        if(localarea) {delete []localarea; localarea = 0;}
        if(p4d) {delete []p4d; p4d = 0;}
        return -1;
    }
    nt = readSWC_file(swc_name);  //why you want to do this? PHC noted 170607
    QFile file (swc_name);file.remove();
    QList<NeuronSWC> list = nt.listNeuron;
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    vector<QList<NeuronSWC> > nt_list;
    V3DLONG seg_max = 0;
    for (int i=0;i<list.size();i++)
    {
        QList<NeuronSWC> nt_seg;
        if (childs[i].size()==0)
        {
            int index_i = i;          
            while(index_i != 1000000000)
            {
                nt_seg.push_front(list.at(index_i));
                index_i = getParent(index_i,nt);
            }
            nt_list.push_back(nt_seg);
            if(nt_seg.size() > seg_max)
                seg_max = nt_seg.size();
            nt_seg.clear();
        }
    }

    double seg_mean_max = 0;
    int seg_tip_id;
    int break_id_optimal;


    v3d_msg(QString("").setNum(nt_list.size()).prepend("The number of segments right now = "), 0);
    for (int i =0; i<nt_list.size();i++)
    {
        QList<NeuronSWC> nt_seg = nt_list.at(i);
        int break_id = nt_seg.size();
//        printf("\nid %d\n",i);
        int angle_size = 5;
        for(int j = 0; j < nt_seg.size()-angle_size; j++)
        {
            double angle_j = 180;
            if(j < angle_size && PARA.nt_last.size() > 0 && nt_original.listNeuron.size()>angle_size)
            {
                NeuronSWC S;
                S.x =  nt_original.listNeuron[nt_original.listNeuron.size()-1-angle_size+j].x - start_x;
                S.y =  nt_original.listNeuron[nt_original.listNeuron.size()-1-angle_size+j].y - start_y;
                S.z =  nt_original.listNeuron[nt_original.listNeuron.size()-1-angle_size+j].z - start_z;
                angle_j = angle(nt_seg[j], S, nt_seg[j+angle_size]);
            }
            else if( j > angle_size)
                angle_j = angle(nt_seg[j], nt_seg[j-angle_size], nt_seg[j+angle_size]);

           // printf("(%.2f,%d)\n",angle_j,j);
            if(angle_j < 100) //this angle is also quite sensitive it seems. by PHC 170608
            {
                break_id = j;
                break;
            }
        }

        double seg_intensity = 0;
        for(int j = 0; j < break_id; j++)
        {
            V3DLONG  ix = nt_seg[j].x;
            V3DLONG  iy = nt_seg[j].y;
            V3DLONG  iz = nt_seg[j].z;
            seg_intensity += localarea[iz*sz_tracing[0]*sz_tracing[1]+ iy *sz_tracing[0] + ix];
        }

        if(seg_intensity/break_id >= seg_mean_max && break_id > 10)
        {
            seg_mean_max = seg_intensity/break_id;
            seg_tip_id = i;
            break_id_optimal = break_id;
        }

//        QString swc_seg = swc_name + QString("%1.swc").arg(i);
//        for(d = nt_seg.size() -1; d >= break_id;d--)
//            nt_seg[d].type = 1;
//        export_list2file(nt_seg, swc_seg,swc_name);
   //     printf("\n");
    }

//    QString swc_seg = swc_name + QString("%1_%2.swc").arg(seg_tip_id);
//    QList<NeuronSWC> nt_seg = nt_list.at(seg_tip_id);
//    for(d = nt_seg.size() -1; d >= break_id_optimal;d--)
//        nt_seg[d].type = 1;
//    export_list2file(nt_seg, swc_seg,swc_name);
//    v3d_msg(QString("id is %1").arg(seg_tip_id));
    QList<NeuronSWC> nt_selected = nt_list.at(seg_tip_id);
    V3DLONG length_seg = nt_selected.size();

    double std_cof = (break_id_optimal==length_seg) ? 0.5 : 0;

    for(d = length_seg -1; d >= break_id_optimal;d--)
        nt_selected.removeAt(d);


    for(int i =0; i<nt_selected.size();i++)
    {
        V3DLONG  ix = nt_selected[i].x;
        V3DLONG  iy = nt_selected[i].y;
        V3DLONG  iz = nt_selected[i].z;
        printf("selected seg info: %d\t",localarea[iz*sz_tracing[0]*sz_tracing[1]+ iy *sz_tracing[0] + ix]);
    }
    double std = 0;
    double *arr=0,tmp;
    arr = new double[nt_selected.size()];
    int ii,jj;
    ii = 0;
    for(int i=0; i <nt_selected.size();i++)
    {
        NeuronSWC curr = nt_selected.at(i);
        V3DLONG ix = curr.x;
        V3DLONG iy = curr.y;
        V3DLONG iz = curr.z;
        double PX = localarea[iz*sz_tracing[0]*sz_tracing[1]+ iy *sz_tracing[0] + ix];
        arr[ii] = PX;
        if (ii>0)
        {
            jj = ii;
            while(jj > 0 && arr[jj-1]>arr[jj])
            {
                tmp = arr[jj];
                arr[jj] = arr[jj-1];
                arr[jj-1] = tmp;
                jj--;
            }
        }
        ii++;
        std += pow(PX-seg_mean_max,2);
    }
    std = sqrt(std/nt_selected.size());
    printf("mean is %.2f, std is %.2f\n\n\n",seg_mean_max, std);
    if (arr) {delete []arr; arr=0;}

    bool ending_tip = false;
    bool b_boundary = false;
    for(int i = nt_selected.size()-1; i >= 0; i--)
    {
        NeuronSWC curr = nt_selected.at(i);
        V3DLONG ix = curr.x;
        V3DLONG iy = curr.y;
        V3DLONG iz = curr.z;
        if(!ending_tip)
        {
            if(localarea[iz*sz_tracing[0]*sz_tracing[1]+ iy *sz_tracing[0] + ix] >= seg_mean_max + std_cof*std) //the 0.5 is a tricky choice, need optimization later, by PHC 170608
            {
                ending_tip = true;
                LocationSimple newmarker;
                newmarker.x = curr.x + start_x +1;
                newmarker.y = curr.y + start_y +1;
                newmarker.z = curr.z + start_z +1;
                PARA.listLandmarks.removeAt(markSize-1);

                double boundary_margin_ratio=0.02;
                if(newmarker.x <= N*boundary_margin_ratio || newmarker.x >= N*(1-boundary_margin_ratio) ||
                        newmarker.y <= M*boundary_margin_ratio || newmarker.y >= M*(1-boundary_margin_ratio) ||
                        newmarker.z <= P*boundary_margin_ratio || newmarker.z >= P*(1-boundary_margin_ratio))
                {
                    b_boundary = true;
                }
                else
                    PARA.listLandmarks.push_back(newmarker);
                break;
            }
            else
                nt_selected.removeAt(i);
        }
    }

    double overall_mean, overall_std;
    mean_and_std(data1d, N*M*P, overall_mean, overall_std);
    printf("overall mean is %.2f, std is %.2f\n",overall_mean, overall_std);
    double curve_mean = 0, curve_std = 0;
    for (int i=0;i<nt_selected.size();i++)
    {
        NeuronSWC curr = nt_selected.at(i);
        curve_mean += data1d[((int)curr.z + start_z)*N*M+ ((int)curr.y + start_y)*N + ((int)curr.x + start_x)];
    }
    curve_mean = curve_mean/nt_selected.size();

    for (int i=0;i<nt_selected.size();i++)
    {
        NeuronSWC curr = nt_selected.at(i);
        double PX =  data1d[((int)curr.z + start_z)*N*M+ ((int)curr.y + start_y)*N + ((int)curr.x + start_x)];;
        curve_std += pow(PX-curve_mean,2);
    }
    curve_std = sqrt(curve_std/nt_selected.size());
    printf("curve mean is %.2f, std is %.2f\n\n\n",curve_mean, curve_std);

    if(fabs(overall_mean-curve_mean) < (overall_std+curve_std)/2)
    {
        if(localarea) {delete []localarea; localarea = 0;}
        if(p4d) {delete []p4d; p4d = 0;}
        PARA.listLandmarks.removeAt(markSize-1);
        if(bmenu)
        {
            callback.setLandmark(curwin,PARA.listLandmarks);
            callback.pushObjectIn3DWindow(curwin);
        }
        return -1;
    }

    NeuronSWC S;
    V3DLONG nt_length = nt_original.listNeuron.size();
    V3DLONG index;
    if(nt_length>0)
        index = nt_original.listNeuron.at(nt_length-1).n;
    else
        index = 0;

    PARA.nt_last.clear();
    for (int i=0;i<nt_selected.size();i++)
    {
        NeuronSWC curr = nt_selected.at(i);
        S.n 	= curr.n + index;
        S.type 	= 3;
        S.x 	= curr.x + start_x;
        S.y 	= curr.y + start_y;
        S.z 	= curr.z + start_z;
        S.r 	= curr.r;
        S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + index;
        nt_original.listNeuron.append(S);
        nt_original.hashNeuron.insert(S.n, nt_original.listNeuron.size()-1);

        if(!b_boundary) PARA.nt_last.push_back(S); //use global coordinates
    }

//    PARA.nt_last = nt_selected;

    if (0)
    {
        printf("last dd = %d %5.3f %5.3f %5.3f \n", nt_selected.size(),
               nt_selected.at(nt_selected.size()-1).x - nt_selected.at(0).x,
               nt_selected.at(nt_selected.size()-1).y - nt_selected.at(0).y,
               nt_selected.at(nt_selected.size()-1).z - nt_selected.at(0).z);
        v3d_msg("press to continue");
    }

    if(bmenu)
    {
        callback.setLandmark(curwin,PARA.listLandmarks);
        nt_original.color.r = 0;
        nt_original.color.g = 0;
        nt_original.color.b = 0;
        nt_original.color.a = 0;

        callback.setSWC(curwin,nt_original);
        callback.updateImageWindow(curwin);
        callback.pushObjectIn3DWindow(curwin); //by PHC 170601
    }
    else
    {
        PARA.nt = nt_original;
    }

    if(localarea) {delete []localarea; localarea = 0;}
    if(p4d) {delete []p4d; p4d = 0;}

    bool phcdebug=false;
    if (phcdebug)
    {
        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++ )
        {
            nt.listNeuron[i].x += start_x;
            nt.listNeuron[i].y += start_y;
            nt.listNeuron[i].z += start_z;
        }
        writeSWC_file(swc_name,nt);
    }

 //   v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    if (b_boundary)
        v3d_msg("Hit the boundary!",0);

    if (b_boundary && PARA.listLandmarks.size()==0)
    {
        if(!bmenu)
        {
            QString output;
            if(PARA.outswc_file != "NULL")
                output = PARA.outswc_file;
            else
                output = PARA.inimg_file + "_GD_curveline.swc";
            QList<NeuronSWC> neuron_output;
            if (!SortSWC(PARA.nt.listNeuron, neuron_output,1, VOID))
            {
                return -1;
            }
            export_list2file(neuron_output, output,swc_name);

//            vector<MyMarker *> ntmarkers = readSWC_file(output.toStdString());
//            map<MyMarker*, double> score_map;
//            topology_analysis_perturb_intense(data1d, ntmarkers, score_map, 1, N, M, P, 0);
//            for(V3DLONG i = 0; i<ntmarkers.size(); i++){
//                MyMarker * marker = ntmarkers[i];
//                double tmp = score_map[marker] * 120 +19;
//                marker->type = tmp > 255 ? 255 : tmp;
//            }
//            QString fname_tmp = output+"_scored.swc";
//            saveSWC_file(fname_tmp.toStdString(), ntmarkers);
        }
    }

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    return (b_boundary && PARA.listLandmarks.size()==0) ? 1 : 0;
}
