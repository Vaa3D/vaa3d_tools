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


Q_EXPORT_PLUGIN2(line_detector, line_detector);

using namespace std;

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
		<<tr("tracing_menu")
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
	if (menu_name == tr("tracing_menu"))
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
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
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
    bool b_mergeCloseBranches = false;
    bool b_usedshortestpathonly = false;
    bool b_postTrim = false;
    bool b_pruneArtifactBranches = true;
    int ds_step = 0;

    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_resolution_step = ds_step;
    trace_para.b_deformcurve = b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = b_postTrim;
    trace_para.b_pruneArtifactBranches = b_pruneArtifactBranches;

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

                localarea[d] = data1d[offsetk + offsetj + ix];
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

    double min_score = INF;
    LocationSimple tmpp;
    QString fname_tmp;
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

                    nt = v3dneuron_GD_tracing(p4d, sz_tracing,
                                              p0, pp,
                                              trace_para, weight_xy_z);

                    QString swc_name = PARA.inimg_file + "_line_detector.swc";
                    nt.name = "line_detector";
                    writeSWC_file(swc_name.toStdString().c_str(),nt);
                    vector<MyMarker*> swc_file = readSWC_file(swc_name.toStdString());
                    map<MyMarker*, double> score_map;
                    topology_analysis_perturb_intense(localarea, swc_file, score_map, 1, sz_tracing[0], sz_tracing[1], sz_tracing[2],1);
                    double seg_score = score_map[swc_file[0]] * 120 +19;;
                    if(seg_score <= min_score)
                    {
                        min_score = seg_score;;
                        fname_tmp =  PARA.inimg_file +"_scored.swc";
                        for(V3DLONG i = 0; i<swc_file.size(); i++){
                            MyMarker * marker = swc_file[i];
                            double tmp = score_map[marker] * 120 +19;
                            marker->type = tmp > 255 ? 255 : tmp;
                        }
                         saveSWC_file(fname_tmp.toStdString(), swc_file);
                    }
                    pp.clear();
                }
            }
        }
    }

    vector<MyMarker*> seg_min_score = readSWC_file(fname_tmp.toStdString());
    map<MyMarker*, double> score_map;
    topology_analysis_perturb_intense(localarea, seg_min_score, score_map, 1, sz_tracing[0], sz_tracing[1], sz_tracing[2],0);

    double score_min = 0;
    double score_std = 0;
    for(V3DLONG i = 0; i<seg_min_score.size(); i++){
        MyMarker * marker = seg_min_score[i];
        double tmp = score_map[marker] * 120 +19;
        score_min += tmp/seg_min_score.size();
    }

//    for(V3DLONG i = 0; i<seg_min_score.size(); i++){
//        MyMarker * marker = seg_min_score[i];
//        double tmp = score_map[marker] * 120 +19;
//        score_std += pow(tmp-score_min,2)/seg_min_score.size();
//    }

    printf("mean is %.2f, std is %.2f\n\n\n",score_min,sqrt(score_std));

    for(V3DLONG i = 0; i<seg_min_score.size(); i++){
        MyMarker * marker = seg_min_score[i];
        double tmp = score_map[marker] * 120 +19;
        if(tmp > score_min)
            marker->type = 0;
        else
            marker->type = tmp;
        marker->x += start_x;
        marker->y += start_y;
        marker->z += start_z;
    }
     saveSWC_file(fname_tmp.toStdString(), seg_min_score);

    //Output
//    for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
//    {
//        nt.listNeuron[i].x += start_x;
//        nt.listNeuron[i].y += start_y;
//        nt.listNeuron[i].z += start_z;
//    }



    if(localarea) {delete []localarea; localarea = 0;}
    if(p4d) {delete []p4d; p4d = 0;}


    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

 //   v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
