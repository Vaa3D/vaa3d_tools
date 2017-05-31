/* line_detector_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-5-25 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

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

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    LandmarkList listLandmarks;
    int win_size;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList line_detector::menulist() const
{
	return QStringList() 
        <<tr("GD Curveline")
		<<tr("about");
}

QStringList line_detector::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
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
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2017-5-25"));
	}
}

bool line_detector::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of line_detector tracing **** \n");
		printf("vaa3d -x line_detector -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
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
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        PARA.listLandmarks = callback.getLandmark(curwin);
        if(PARA.listLandmarks.count() ==0)
        {
            QMessageBox::information(0, "", "No markers in the current image, please select a marker.");
            return;
        }

        bool ok;
        PARA.win_size = QInputDialog::getInteger(parent, "Window size",
                                                 "Enter radius (window size is 2*radius+1):",
                                                 32, 1, 512, 1, &ok);
        if (!ok)
            return;

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
            return;

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
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
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
    trace_para.sp_graph_resolution_step = 0;
    trace_para.b_postMergeClosebyBranches = true;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = true;
    trace_para.b_pruneArtifactBranches = true;
    trace_para.sp_num_end_nodes = 2;
    trace_para.b_deformcurve = false;

    p0.x = PARA.listLandmarks.at(0).x-1;
    p0.y = PARA.listLandmarks.at(0).y-1;
    p0.z = PARA.listLandmarks.at(0).z-1;

    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    start_x = (p0.x - PARA.win_size < 0)?  0 : p0.x - PARA.win_size;
    start_y = (p0.y - PARA.win_size < 0)?  0 : p0.y - PARA.win_size;
    start_z = (p0.z - PARA.win_size < 0)?  0 : p0.z - PARA.win_size;

    end_x = (p0.x + PARA.win_size > N)?  N-1 : p0.x + PARA.win_size;
    end_y = (p0.y + PARA.win_size > M)?  M-1 : p0.y + PARA.win_size;
    end_z = (p0.z + PARA.win_size > P)?  P-1 : p0.z + PARA.win_size;

    NeuronTree nt_original = callback.getSWC(curwin);
    V3DLONG stacksz = N*M*P*sc;
    unsigned char* data1d_mask = 0;
    data1d_mask = new unsigned char [stacksz];

    memset(data1d_mask,0,stacksz*sizeof(unsigned char));

    if(nt_original.listNeuron.size()>0)
        ComputemaskImage(nt_original, data1d_mask, N, M, P);

    unsigned char *localarea=0;
    V3DLONG blockpagesz = (end_x-start_x+1)*(end_y-start_y+1)*(end_z-start_z+1);
    localarea = new unsigned char [blockpagesz];
    V3DLONG d = 0;
    for(V3DLONG iz = start_z; iz < end_z + 1; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = start_y; iy < end_y+1; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = start_x; ix < end_x+1; ix++)
            {

                if(data1d_mask[offsetk + offsetj + ix] == 0)
                    localarea[d] = data1d[offsetk + offsetj + ix];
                else
                    localarea[d] = 1;
                d++;
            }
        }
    }

    V3DLONG sz_tracing[4];
    sz_tracing[0] = end_x-start_x+1;
    sz_tracing[1] = end_y-start_y+1;
    sz_tracing[2] = end_z-start_z+1;
    sz_tracing[3] = 1;

    unsigned char ****p4d = 0;
    if (!new4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], localarea))
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        return;
    }
    p0.x -= start_x;
    p0.y -= start_y;
    p0.z -= start_z;

    LocationSimple tmpp;
    for(V3DLONG ix = 0; ix <=2; ix++)
    {
        tmpp.x = (2-ix)*start_x/2 + ix*end_x/2 - start_x;
        for(V3DLONG iy = 0; iy <=2; iy++)
        {
            tmpp.y = (2-iy)*start_y/2 + iy*end_y/2 - start_y;
            for(V3DLONG iz = 0; iz <=2; iz++)
            {
                tmpp.z = (2-iz)*start_z/2 + iz*end_z/2 - start_z;
                if(!(ix==1 && iy ==1 && iz==1))
                {
                    pp.push_back(tmpp);

                }
            }
        }
    }

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
        if((fabs(nt.listNeuron[pn_ID].x-nt.listNeuron[0].x)+fabs(nt.listNeuron[pn_ID].y-nt.listNeuron[0].y) + abs(nt.listNeuron[pn_ID].z-nt.listNeuron[0].z))< 1e-3)
        {
                nt.listNeuron[i].pn = 1;
        }
    }
    QList<NeuronSWC> neuron_sorted;
    if (!SortSWC(nt.listNeuron, neuron_sorted,VOID, 0))  //was 10
    {
        v3d_msg("fail to call swc sorting function.");
    }
    export_list2file(neuron_sorted, swc_name,swc_name);
    if(neuron_sorted.size() <=0)
    {
        v3d_msg("cannot find any path based on the given marker.");
        return;
    }
    nt = readSWC_file(swc_name);
    QList<NeuronSWC> list = nt.listNeuron;
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 0;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    /*vector<QList<NeuronSWC> > nt_list;
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

    for (int i =0; i<nt_list.size();i++)
    {
        QList<NeuronSWC> nt_seg = nt_list.at(i);
        double seg_intensity = 0;
        for(int j = 0; j < seg_max; j++)
        {
            if(j < nt_seg.size())
            {
                V3DLONG  ix = nt_seg[j].x;
                V3DLONG  iy = nt_seg[j].y;
                V3DLONG  iz = nt_seg[j].z;
                seg_intensity += localarea[iz*sz_tracing[0]*sz_tracing[1]+ iy *sz_tracing[1] + ix];
                printf("%.2f\t",seg_intensity/(j+1));
                nt_seg[j].x += start_x;
                nt_seg[j].y += start_y;
                nt_seg[j].z += start_z;
                nt_seg[j].r = (seg_intensity/(j+1))/j;

            }
            else
                printf("%.2f\t",seg_intensity/nt_seg.size());

        }
        QString swc_seg = swc_name + QString("%1.swc").arg(i);
        export_list2file(nt_seg, swc_seg,swc_name);
        printf("\n");

    }*/

    double seg_mean_max = 0;
    double seg_mean;
    int seg_tip_id;
    for (int i=0;i<list.size();i++)
    {
        if (childs[i].size()==0)
        {
            double intensity_seg = 0;
            int seg_num = 0;
            int index_i = i;
            while(index_i != 1000000000)
            {
                V3DLONG ix = list[index_i].x;
                V3DLONG iy = list[index_i].y;
                V3DLONG iz = list[index_i].z;
                intensity_seg += localarea[iz*sz_tracing[0]*sz_tracing[1]+ iy *sz_tracing[1] + ix];
                v3d_msg(QString("intensity is %1, id is %2").arg(localarea[iz*sz_tracing[0]*sz_tracing[1]+ iy *sz_tracing[1] + ix]).arg(index_i),0);
                seg_num++;
                index_i = getParent(index_i,nt);
            }
            if(intensity_seg >= seg_mean_max && seg_num > 10)
            {
                seg_mean_max = intensity_seg;
                seg_tip_id = i;
                seg_mean = intensity_seg/seg_num;
            }
        }
    }
    printf("mean is %.2f\n",seg_mean);
    bool ending_tip = false;
    while(seg_tip_id != 1000000000)
    {
        V3DLONG ix = list[seg_tip_id].x;
        V3DLONG iy = list[seg_tip_id].y;
        V3DLONG iz = list[seg_tip_id].z;
        printf("intensit is %d,id is %d\n",localarea[iz*sz_tracing[0]*sz_tracing[1]+ iy *sz_tracing[1] + ix],seg_tip_id);
        if(!ending_tip)
        {
            if(localarea[iz*sz_tracing[0]*sz_tracing[1]+ iy *sz_tracing[1] + ix] >= seg_mean)
            {
                flag[seg_tip_id] = 1;
                ending_tip = true;
                PARA.listLandmarks[0].x = ix + 1 + start_x;
                PARA.listLandmarks[0].y = iy + 1 + start_y;
                PARA.listLandmarks[0].z = iz + 1 + start_z;
            }
        }else
            flag[seg_tip_id] = 1;
        seg_tip_id = getParent(seg_tip_id,nt);
    }

    NeuronSWC S;
    V3DLONG nt_length = nt_original.listNeuron.size();
    for (int i=0;i<list.size();i++)
    {
        if(flag[i] == 1)
        {
             NeuronSWC curr = list.at(i);
             S.n 	= curr.n + nt_length;
             S.type 	= 3;
             S.x 	= curr.x + start_x;
             S.y 	= curr.y + start_y;
             S.z 	= curr.z + start_z;
             S.r 	= curr.r;
             S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + nt_length;
             nt_original.listNeuron.append(S);
             nt_original.hashNeuron.insert(S.n, nt_original.listNeuron.size()-1);
        }

   }

    callback.setLandmark(curwin,PARA.listLandmarks);
    nt_original.color.r = 0;
    nt_original.color.g = 0;
    nt_original.color.b = 0;
    nt_original.color.a = 0;

    callback.setSWC(curwin,nt_original);
    callback.open3DWindow(curwin);
    callback.getView3DControl(curwin)->updateWithTriView();
    if(flag) {delete[] flag; flag = 0;}
    if(localarea) {delete []localarea; localarea = 0;}
    if(p4d) {delete []p4d; p4d = 0;}

    for(V3DLONG i = 0; i < nt.listNeuron.size(); i++ )
    {
        nt.listNeuron[i].x += start_x;
        nt.listNeuron[i].y += start_y;
        nt.listNeuron[i].z += start_z;
    }
    writeSWC_file(swc_name,nt);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

 //   v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
