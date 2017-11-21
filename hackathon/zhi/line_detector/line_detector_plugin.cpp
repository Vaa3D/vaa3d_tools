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
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
#include <iostream>

#include <fstream>

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
    int angle_size;
    NeuronTree nt;
    NeuronTree nt_input;
    QString outswc_file;
    QList<NeuronSWC> nt_last;

    input_PARA() {win_size=0; channel=0;}
    int xc0, xc1, yc0, yc1, zc0, zc1; //the six bounding box boundaries

};

//return -1: wrong running parameters, etc.
//return 0: exit normally
//return 1: exit when hit bounadry
int reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
int curveFitting_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
bool curveFitting_func_v2(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
int reconstruction_func_v2(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);

 
QStringList line_detector::menulist() const
{
	return QStringList() 
        <<tr("GD Curveline")
        <<tr("GD Curveline v2")
        <<tr("GD Curveline infinite")
        <<tr("GD Curve fitting")
        <<tr("GD Curve fitting_v2")
        <<tr("about");
}

QStringList line_detector::funclist() const
{
	return QStringList()
        <<tr("GD_curveline")
        <<tr("GD_Curveline_infinite")
        <<tr("GD_Curveline_v2")
        <<tr("mip_swc")
        <<tr("GD_steps")
        <<tr("GD_markers")
        <<tr("Crop_blocks")
        <<tr("Crop_dendrite")
        <<tr("shift_swc")
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
    else if (menu_name == tr("GD Curve fitting"))
    {
        bool bmenu = true;
        input_PARA PARA;
        curveFitting_func(callback,parent,PARA,bmenu);
    }
    else if (menu_name == tr("GD Curve fitting_v2"))
    {
        bool bmenu = true;
        input_PARA PARA;
        curveFitting_func_v2(callback,parent,PARA,bmenu);
    }
    else if (menu_name == tr("GD Curveline v2"))
    {
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func_v2(callback,parent,PARA,bmenu);
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

        PARA.outswc_file = outfiles.empty() ? "" : outfiles[0];
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
        PARA.angle_size = (paras.size() >= k+1) ? atoi(paras[k]) : 10;  k++;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;

        QString inneuron_file = (paras.size() >= k+1) ? paras[k] : "";
        if(!inneuron_file.isEmpty())
        {
            PARA.nt_input = readSWC_file(inneuron_file);
            PARA.nt_last = PARA.nt_input.listNeuron;
        }
        for (;;)
        {
            int res = reconstruction_func(callback,parent,PARA,bmenu);
            if (res!=0)
                break;
        }

        QString output;
        if(PARA.outswc_file != "")
            output = PARA.outswc_file;
        else
            output = PARA.inimg_file + "_GD_curveline.swc";
        QList<NeuronSWC> neuron_output;
        if(PARA.nt.listNeuron.size()>0)
        {
            if (!SortSWC(PARA.nt.listNeuron, neuron_output,1, VOID))
            {
                return -1;
            }
        }
        export_list2file(neuron_output, output,output);

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
    }else if (func_name == tr("GD_Curveline_v2"))
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

        PARA.outswc_file = outfiles.empty() ? "" : outfiles[0];
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
        PARA.angle_size = (paras.size() >= k+1) ? atoi(paras[k]) : 10;  k++;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;

        QString inneuron_file = (paras.size() >= k+1) ? paras[k] : "";
        if(!inneuron_file.isEmpty())
        {
            PARA.nt_input = readSWC_file(inneuron_file);
        }

       // if(reconstruction_func_v2(callback,parent,PARA,bmenu) == 1)
       // {
        QElapsedTimer timer1;
        timer1.start();

        reconstruction_func_v2(callback,parent,PARA,bmenu);
        qint64 etime1 = timer1.elapsed();

        QDir imagefolderDir = QDir(QFileInfo(PARA.inimg_file).absoluteDir());
        QString time_text =  imagefolderDir.absolutePath() + "/time.txt";
        QString output;
        if(PARA.outswc_file != "")
            output = PARA.outswc_file;
        else
            output = PARA.inimg_file + "_GD_curveline_v2.swc";
        export_list2file(PARA.nt.listNeuron, output,output);

        QFile saveTextFile;
        saveTextFile.setFileName(time_text);
        if (!saveTextFile.isOpen()){
            if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
                qDebug()<<"unable to save file!";
                return false;}     }
        QTextStream outputStream;
        outputStream.setDevice(&saveTextFile);
        outputStream<< PARA.inimg_file.toStdString().c_str()<<"\t"<< etime1<<"\n";
        saveTextFile.close();



        //}else

    }else if (func_name == tr("mip_swc"))
    {
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        unsigned char* data1d = 0;
        V3DLONG in_sz[4];
        QString inimg_file = infiles[0];
        int k=0;
//        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        QString swc_name = paras.empty() ? "" : paras[k]; if(swc_name == "NULL") swc_name = ""; k++;
//        QString swc_name_manual = paras.empty() ? "" : paras[k]; if(swc_name_manual == "NULL") swc_name_manual = ""; k++;
//        QString marker_shift = paras.empty() ? "" : paras[k]; if(marker_shift == "NULL") marker_shift = ""; k++;

//        QString filename = (QFileInfo(inmarker_file).completeBaseName());
//        QString outimg_file = QFileInfo(inmarker_file).absolutePath() +"/"+ filename + ".marker_gd.v3dpbd";
//        QString shift_auto = QFileInfo(inmarker_file).absolutePath() +"/"+ filename + ".marker_gd_shifted.swc";
//        QString shift_manual = QFileInfo(inmarker_file).absolutePath() +"/"+ filename + ".marker_manual_shifted.swc";
//        QString inmarker_file_shift = QFileInfo(inmarker_file).absolutePath() +"/"+ filename + ".marker_shifted.apo";


        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return -1;
        }

//        simple_saveimage_wrapper(callback,outimg_file.toStdString().c_str(),data1d,in_sz,datatype);

        V3DLONG stacksz =in_sz[0]*in_sz[1];
        unsigned char *image_mip=0;
        image_mip = new unsigned char [stacksz];
        for(V3DLONG iy = 0; iy < in_sz[1]; iy++)
        {
            V3DLONG offsetj = iy*in_sz[0];
            for(V3DLONG ix = 0; ix < in_sz[0]; ix++)
            {
                int max_mip = 0;
                for(V3DLONG iz = 0; iz < in_sz[2]; iz++)
                {
                    V3DLONG offsetk = iz*in_sz[1]*in_sz[0];
                    if(data1d[offsetk + offsetj + ix] >= max_mip)
                    {
                        image_mip[iy*in_sz[0] + ix] = data1d[offsetk + offsetj + ix];
                        max_mip = data1d[offsetk + offsetj + ix];
                    }
                }
            }
        }

//        vector<MyMarker> file_inmarkers = readMarker_file(string(qPrintable(inmarker_file)));

//        V3DLONG start_x,end_x,start_y,end_y,start_z,end_z;
//        start_x = file_inmarkers[0].x;
//        start_y = file_inmarkers[0].y;
//        start_z = file_inmarkers[0].z;

//        end_x = file_inmarkers[0].x;
//        end_y = file_inmarkers[0].y;
//        end_z = file_inmarkers[0].z;

//        for(int i =1; i < file_inmarkers.size();i++)
//        {
//            start_x = (start_x < file_inmarkers[i].x)? start_x:file_inmarkers[i].x;
//            start_y = (start_y < file_inmarkers[i].y)? start_y:file_inmarkers[i].y;
//            start_z = (start_z < file_inmarkers[i].z)? start_z:file_inmarkers[i].z;

//            end_x = (end_x > file_inmarkers[i].x)? end_x:file_inmarkers[i].x;
//            end_y = (end_y > file_inmarkers[i].y)? end_y:file_inmarkers[i].y;
//            end_z = (end_z > file_inmarkers[i].z)? end_z:file_inmarkers[i].z;
//        }

//        start_x -= 50; start_y -=50; start_z -=50;
//        end_x += 50; end_y +=50; end_z +=50;
//        V3DLONG pagesz = (end_x - start_x)*(end_y - start_y)*(end_z - start_z);
        NeuronTree nt = readSWC_file(swc_name);
//        NeuronTree nt_manual = readSWC_file(swc_name_manual);

//        if(pagesz >= 8589934592)
//        {
//            for(V3DLONG i =0; i < nt.listNeuron.size();i++)
//            {
//                nt.listNeuron[i].x /= 2;
//                nt.listNeuron[i].y /= 2;
//                nt.listNeuron[i].z /= 2;
//            }

//            for(V3DLONG i =0; i < nt_manual.listNeuron.size();i++)
//            {
//                nt_manual.listNeuron[i].x /= 2;
//                nt_manual.listNeuron[i].y /= 2;
//                nt_manual.listNeuron[i].z /= 2;
//            }
//            start_x /= 2; start_y /= 2; start_z /= 2;
//            end_x /= 2;   end_y /= 2;   end_z /= 2;
//        }

//        for(V3DLONG i =0; i < nt.listNeuron.size();i++)
//        {
//            nt.listNeuron[i].x -= start_x;
//            nt.listNeuron[i].y -= start_y;
//            nt.listNeuron[i].z -= start_z;

//            nt.listNeuron[i].x += 5;
//            nt.listNeuron[i].y += 5;
      //      nt.listNeuron[i].r = 1;
//            nt.listNeuron[i].type = 3;
//        }

//        for(V3DLONG i =0; i < nt_manual.listNeuron.size();i++)
//        {
//            nt_manual.listNeuron[i].x -= start_x;
//            nt_manual.listNeuron[i].y -= start_y;
//            nt_manual.listNeuron[i].z -= start_z;;

//            nt_manual.listNeuron[i].x -= 5;
//            nt_manual.listNeuron[i].y -= 5;
          //  nt_manual.listNeuron[i].r = 1;
//            nt_manual.listNeuron[i].type = 2;

//        }

//        vector<MyMarker> file_inmarkers_shifted = readMarker_file(string(qPrintable(marker_shift)));
//        QList<CellAPO> file_apo;

//        for(int i =0; i<2; i++)
//        {
//            CellAPO t;
//            t.x = file_inmarkers_shifted.at(i).x;
//            t.y = file_inmarkers_shifted.at(i).y;
//            t.z = file_inmarkers_shifted.at(i).z;
//            t.color.r=0;
//            t.color.g=255;
//            t.color.b=0;

//            t.volsize = 20;
//            file_apo.push_back(t);
//        }

//        writeAPO_file(inmarker_file_shift,file_apo);

//        export_list2file(nt.listNeuron, shift_auto,shift_auto);
//        export_list2file(nt_manual.listNeuron, shift_manual,shift_manual);

//        QString qs_filename_out = "/local4/Data/MOST_MOUSE/Cell_03129_03228_X13179_Y25412/" + inmarker_file + "_linker.ano";
//        QFile qf_anofile(qs_filename_out);
//        if(!qf_anofile.open(QIODevice::WriteOnly))
//        {
//            v3d_msg("Cannot open file for writing!",0);
//            return false;
//        }

//        QTextStream out(&qf_anofile);
//        out << "RAWIMG=" << inimg_file << endl;
//        out << "SWCFILE=" << shift_auto << endl;
//        out << "SWCFILE=" << shift_manual << endl;
//        out << "APOFILE=" << apo_shift << endl;

        unsigned char* data1d_mask = 0;
 //       unsigned char* data1d_mask_manual = 0;

        data1d_mask = new unsigned char [stacksz];
 //       data1d_mask_manual = new unsigned char [stacksz];

        memset(data1d_mask,0,stacksz*sizeof(unsigned char));
 //       memset(data1d_mask_manual,0,stacksz*sizeof(unsigned char));


        for(V3DLONG i =0; i < nt.listNeuron.size();i++)
        {
            nt.listNeuron[i].z = 0;
            nt.listNeuron[i].r = 1;
        }

//        for(V3DLONG i =0; i < nt_manual.listNeuron.size();i++)
//        {
//            nt_manual.listNeuron[i].z = 0;;
//            nt_manual.listNeuron[i].r = 1;
//        }

        double margin=0;//by PHC 20170531
        ComputemaskImage(nt, data1d_mask, in_sz[0], in_sz[1], 1, margin);
//        QList<NeuronSWC> neuron_output;
//        SortSWC(nt_manual.listNeuron, neuron_output,VOID, 0);
//        nt_manual.listNeuron.clear();
//        nt_manual.listNeuron = neuron_output;
//        ComputemaskImage(nt_manual, data1d_mask_manual, in_sz[0], in_sz[1], 1, margin);

       // QString output_swc = QFileInfo(inimg_file).absolutePath() + "_swc_mip.tif";
        QString output_swc = inimg_file + "_swc_mip.tif";

        in_sz[2] = 1;
        in_sz[3] = 3;

        unsigned char* data1d_2D = 0;
        data1d_2D = new unsigned char [3*stacksz];
//        for(V3DLONG i=0; i<stacksz; i++)
//            data1d_2D[i] = (data1d_mask_manual[i] ==255) ? 255: image_mip[i];

        for(V3DLONG i=0; i<stacksz; i++)
            data1d_2D[i] = image_mip[i];

        for(V3DLONG i=0; i<stacksz; i++)
        {
            data1d_2D[i+stacksz] = (data1d_mask[i] ==255) ? 255: image_mip[i];
        }
        for(V3DLONG i=0; i<stacksz; i++)
            data1d_2D[i+2*stacksz] = image_mip[i];

        simple_saveimage_wrapper(callback,output_swc.toStdString().c_str(),data1d_2D,in_sz,1);

    }else if (func_name == tr("GD_steps"))
    {
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        unsigned char* data1d = 0;
        V3DLONG in_sz[4];
        QString inimg_file = infiles[0];
        int k=0;
        QString swc_name = paras.empty() ? "" : paras[k]; if(swc_name == "NULL") swc_name = ""; k++;
        QString inimg_file_mip = paras.empty() ? "" : paras[k]; if(inimg_file_mip == "NULL") inimg_file_mip = ""; k++;
        QString ano_name = paras.empty() ? "" : paras[k]; if(ano_name == "NULL") ano_name = ""; k++;
        int step_size = 16;

        NeuronTree nt_total;
        QString output2Dimage = inimg_file_mip + QString("_gd_steps_%1.tif").arg(step_size);
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return -1;
        }

        unsigned char ****p4d = 0;
        if (!new4dpointer(p4d, in_sz[0], in_sz[1], in_sz[2], 1, data1d))
        {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            if(p4d) {delete []p4d; p4d = 0;}
            return false;
        }

        NeuronTree nt = readSWC_file(swc_name);
        if(nt.listNeuron.size() < step_size+1)
            return -1;

        LocationSimple p0;
        vector<LocationSimple> pp;


        double weight_xy_z=1.0;
        CurveTracePara trace_para;
        trace_para.channo = 0;
        trace_para.sp_graph_background = 0;
        trace_para.b_postMergeClosebyBranches = false;
        trace_para.sp_graph_resolution_step=4;
        trace_para.b_3dcurve_width_from_xyonly = true;
        trace_para.b_pruneArtifactBranches = false;
        trace_para.sp_num_end_nodes = 2;
        trace_para.b_deformcurve = false;
        trace_para.sp_graph_resolution_step = 1;
        trace_para.b_estRadii = false;
        trace_para.sp_downsample_method = 1;
        trace_para.sp_downsample_step = 4;
        trace_para.sp_graph_resolution_step = 4;

        p0.x = nt.listNeuron[0].x + 10;
        p0.y = nt.listNeuron[0].y + 10;
        p0.z = nt.listNeuron[0].z;
        int step = nt.listNeuron.size()/step_size;

        for(V3DLONG i = step; i <nt.listNeuron.size()+step;i+=step)
        {
            LocationSimple tmpp;
            if(i < nt.listNeuron.size())
            {
                tmpp.x = nt.listNeuron[i].x + 10;
                tmpp.y = nt.listNeuron[i].y + 10;
                tmpp.z = nt.listNeuron[i].z;
            }else
            {
                tmpp.x = nt.listNeuron[nt.listNeuron.size()-1].x + 10;
                tmpp.y = nt.listNeuron[nt.listNeuron.size()-1].y + 10;
                tmpp.z = nt.listNeuron[nt.listNeuron.size()-1].z;
            }
            pp.push_back(tmpp);

            NeuronTree nt_seg = v3dneuron_GD_tracing(p4d, in_sz,
                                                     p0, pp,
                                                     trace_para, weight_xy_z);
            V3DLONG nt_length = nt_total.listNeuron.size();
            V3DLONG index;
            if(nt_length>0)
                index = nt_total.listNeuron.at(nt_length-1).n;
            else
                index = 0;

            NeuronSWC S;
            for (int d=0;d<nt_seg.listNeuron.size();d++)
            {
                NeuronSWC curr = nt_seg.listNeuron.at(d);
                S.n 	= curr.n + index;
                S.type 	= step_size;
                S.x 	= curr.x;
                S.y 	= curr.y;
                S.z 	= curr.z;
                S.r 	= curr.r;
                S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + index;
                nt_total.listNeuron.append(S);
                nt_total.hashNeuron.insert(S.n, nt_total.listNeuron.size()-1);
            }

            pp.clear();
            if(i < nt.listNeuron.size())
            {
                p0.x =  tmpp.x;
                p0.y =  tmpp.y;
                p0.z =  tmpp.z;
            }else
                break;
        }

        QString outputswc = swc_name + QString("_gd_steps_%1.swc").arg(step_size);
        export_list2file(nt_total.listNeuron, outputswc,outputswc);

        V3DLONG stacksz =in_sz[0]*in_sz[1];
        double margin=0;
        unsigned char* data1d_mask = 0;
        data1d_mask = new unsigned char [stacksz];
        for(V3DLONG i =0; i < nt_total.listNeuron.size();i++)
        {
            nt_total.listNeuron[i].z = 0;
            nt_total.listNeuron[i].r = 1;
        }

        ComputemaskImage(nt_total, data1d_mask, in_sz[0], in_sz[1], 1, margin);

        unsigned char* data1d_2D = 0;
        if (!simple_loadimage_wrapper(callback,inimg_file_mip.toStdString().c_str(), data1d_2D, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",inimg_file_mip.toStdString().c_str());
            return -1;
        }

        for(V3DLONG i=0; i<stacksz; i++)
            data1d_2D[i+2*stacksz] = (data1d_mask[i] ==255) ? 255: data1d_2D[i+2*stacksz];

        simple_saveimage_wrapper(callback,output2Dimage.toStdString().c_str(),data1d_2D,in_sz,1);

        ofstream myfile;
        myfile.open (ano_name.toStdString().c_str(),fstream::in | fstream::out | fstream::app);
        myfile << "SWCFILE=" <<outputswc.toStdString().c_str()<<endl;
        myfile.close();

    }else if (func_name == tr("GD_markers"))
    {
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        unsigned char* data1d = 0;
        V3DLONG in_sz[4];
        QString inimg_file = infiles[0];
        int k=0;
        QString swc_name = paras.empty() ? "" : paras[k]; if(swc_name == "NULL") swc_name = ""; k++;
        QString inimg_file_mip = paras.empty() ? "" : paras[k]; if(inimg_file_mip == "NULL") inimg_file_mip = ""; k++;
        QString ano_name = paras.empty() ? "" : paras[k]; if(ano_name == "NULL") ano_name = ""; k++;
        QString marker_file = paras.empty() ? "" : paras[k]; if(marker_file == "NULL") marker_file = ""; k++;

        NeuronTree nt_total;
        QString output2Dimage = inimg_file_mip + "_gd_steps_32.tif";
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return -1;
        }

        unsigned char ****p4d = 0;
        if (!new4dpointer(p4d, in_sz[0], in_sz[1], in_sz[2], 1, data1d))
        {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            if(p4d) {delete []p4d; p4d = 0;}
            return false;
        }

        vector<MyMarker> file_inmarkers = readMarker_file(string(qPrintable(marker_file)));
        LocationSimple p0;
        vector<LocationSimple> pp;


        double weight_xy_z=1.0;
        CurveTracePara trace_para;
        trace_para.channo = 0;
        trace_para.sp_graph_background = 0;
        trace_para.b_postMergeClosebyBranches = false;
        trace_para.sp_graph_resolution_step=4;
        trace_para.b_3dcurve_width_from_xyonly = true;
        trace_para.b_pruneArtifactBranches = false;
        trace_para.sp_num_end_nodes = 2;
        trace_para.b_deformcurve = false;
        trace_para.sp_graph_resolution_step = 1;
        trace_para.b_estRadii = false;
        trace_para.sp_downsample_method = 1;
        trace_para.sp_downsample_step = 4;
        trace_para.sp_graph_resolution_step = 4;

        for(V3DLONG i = 1; i <file_inmarkers.size();i++)
        {
            p0.x = file_inmarkers.at(i-1).x;
            p0.y = file_inmarkers.at(i-1).y;
            p0.z = file_inmarkers.at(i-1).z;

            LocationSimple tmpp;
            tmpp.x = file_inmarkers.at(i).x;
            tmpp.y = file_inmarkers.at(i).y;
            tmpp.z = file_inmarkers.at(i).z;
            pp.push_back(tmpp);

            NeuronTree nt_seg = v3dneuron_GD_tracing(p4d, in_sz,
                                                     p0, pp,
                                                     trace_para, weight_xy_z);
            V3DLONG nt_length = nt_total.listNeuron.size();
            V3DLONG index;
            if(nt_length>0)
                index = nt_total.listNeuron.at(nt_length-1).n;
            else
                index = 0;

            NeuronSWC S;
            for (int d=0;d<nt_seg.listNeuron.size();d++)
            {
                NeuronSWC curr = nt_seg.listNeuron.at(d);
                S.n 	= curr.n + index;
                S.type 	= 7;
                S.x 	= curr.x;
                S.y 	= curr.y;
                S.z 	= curr.z;
                S.r 	= curr.r;
                S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + index;
                nt_total.listNeuron.append(S);
                nt_total.hashNeuron.insert(S.n, nt_total.listNeuron.size()-1);
            }

            pp.clear();
        }

        QString outputswc = marker_file + "_gd_steps_32.swc";
        export_list2file(nt_total.listNeuron, outputswc,outputswc);

        V3DLONG stacksz =in_sz[0]*in_sz[1];
        double margin=0;
        unsigned char* data1d_mask = 0;
        data1d_mask = new unsigned char [stacksz];
        for(V3DLONG i =0; i < nt_total.listNeuron.size();i++)
        {
            nt_total.listNeuron[i].z = 0;
            nt_total.listNeuron[i].r = 1;
        }

        ComputemaskImage(nt_total, data1d_mask, in_sz[0], in_sz[1], 1, margin);

        unsigned char* data1d_2D = 0;
        if (!simple_loadimage_wrapper(callback,inimg_file_mip.toStdString().c_str(), data1d_2D, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",inimg_file_mip.toStdString().c_str());
            return -1;
        }

        for(V3DLONG i=0; i<stacksz; i++)
            data1d_2D[i+2*stacksz] = (data1d_mask[i] ==255) ? 255: data1d_2D[i+2*stacksz];

        simple_saveimage_wrapper(callback,output2Dimage.toStdString().c_str(),data1d_2D,in_sz,1);

        ofstream myfile;
        myfile.open (ano_name.toStdString().c_str(),fstream::in | fstream::out | fstream::app);
        myfile << "SWCFILE=" <<outputswc.toStdString().c_str()<<endl;
        myfile.close();

    }else if (func_name == tr("Crop_blocks"))
    {

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        QString inimg_file = infiles[0];
        QString output_folder = outfiles[0];
        int k=0;
        QString swc_name = paras.empty() ? "" : paras[k]; if(swc_name == "NULL") swc_name = ""; k++;
        int adaptive_size = paras.empty() ? 128 : atoi(paras[k]);

        NeuronTree nt = readSWC_file(swc_name);
        QVector<QVector<V3DLONG> > childs;
        V3DLONG neuronNum = nt.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }

        QList<NeuronSWC> list = nt.listNeuron;
        for(V3DLONG ID = 50; ID < list.size(); ID +=50)
        {
            if(childs.at(ID).size()==1)
            {
                NeuronSWC curr = list.at(ID);
                int last_ID = (ID -10) <=0 ? 0:ID -10;
                double x_diff = curr.x - list.at(last_ID).x;
                double y_diff = curr.y - list.at(last_ID).y;
                double z_diff = curr.z - list.at(last_ID).z;
                double overlap = 0.1;

                V3DLONG start_x, start_y, start_z;
                if(fabs(x_diff) >= fabs(y_diff) && fabs(x_diff) >= fabs(z_diff))
                {
                    if(x_diff < 0)
                        start_x = -floor(adaptive_size*(1-overlap)) + curr.x;
                    else
                        start_x = curr.x - floor(adaptive_size*overlap);

                    start_y = floor(curr.y - adaptive_size/2);
                    start_z = floor(curr.z - adaptive_size/2);
                }else if(fabs(y_diff) >= fabs(x_diff) && fabs(y_diff) >= fabs(z_diff))
                {
                    if(y_diff < 0)
                        start_y = -floor(adaptive_size*(1-overlap)) + curr.y;
                    else
                        start_y =  curr.y - floor(adaptive_size*overlap);

                    start_x = floor(curr.x - adaptive_size/2);
                    start_z = floor(curr.z - adaptive_size/2);

                }else if(fabs(z_diff) >= fabs(x_diff) && fabs(z_diff) >= fabs(y_diff))
                {
                    if(z_diff < 0)
                        start_z = -floor(adaptive_size*(1-overlap)) + curr.z;
                    else
                        start_z =  curr.z - floor(adaptive_size*overlap);
                    start_x = floor(curr.x - adaptive_size/2);
                    start_y = floor(curr.y - adaptive_size/2);
                }

                unsigned char * total1dData = 0;
                total1dData = callback.getSubVolumeTeraFly(inimg_file.toStdString(),start_x,start_x+adaptive_size,
                                                           start_y,start_y + adaptive_size,start_z,start_z + adaptive_size);
                V3DLONG mysz[4];
                mysz[0] = adaptive_size;
                mysz[1] = adaptive_size;
                mysz[2] = adaptive_size;
                mysz[3] = 1;
                QString imageSaveString = output_folder + QString("/x_%1_y%2_z%3.v3dpbd").arg(curr.x).arg(curr.y).arg(curr.z);
                simple_saveimage_wrapper(callback, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, 1);
                QString marker_name = imageSaveString + ".marker";
                ImageMarker outputMarker;
                QList<ImageMarker> seedsToSave;
                outputMarker.x = curr.x - start_x +1;
                outputMarker.y = curr.y - start_y +1;
                outputMarker.z = curr.z - start_z +1;
                seedsToSave.append(outputMarker);
                writeMarker_file(marker_name, seedsToSave);

                NeuronTree nt_tps;
                QList <NeuronSWC> & listNeuron = nt_tps.listNeuron;
                listNeuron << curr;
                int child_id = childs[ID].at(0);
                while(childs[child_id].size()>0 && list.at(child_id).x >= start_x && list.at(child_id).x < start_x+adaptive_size
                      && list.at(child_id).y >= start_y && list.at(child_id).y < start_y+adaptive_size
                      && list.at(child_id).z >= start_z && list.at(child_id).z < start_z+adaptive_size)
                {
                    listNeuron << list.at(child_id);
                    child_id = childs[child_id].at(0);
                }
                for(V3DLONG d = 0; d < nt_tps.listNeuron.size();d++)
                {
                    nt_tps.listNeuron[d].x -= start_x;
                    nt_tps.listNeuron[d].y -= start_y;
                    nt_tps.listNeuron[d].z -= start_z;
                }
                QString manual_swc = imageSaveString + ".swc";

                writeSWC_file(manual_swc,nt_tps);
                listNeuron.clear();
            }
        }
    }else if (func_name == tr("Crop_dendrite"))
    {

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        QString inimg_file = infiles[0];
        int k=0;
        QString apo_name = paras.empty() ? "" : paras[k]; if(apo_name == "NULL") apo_name = ""; k++;
        QList<CellAPO> file_inmarkers;
        file_inmarkers = readAPO_file(apo_name);
        V3DLONG start_x = file_inmarkers.at(0).x - 512;
        V3DLONG end_x = start_x + 1024 - 1;
        V3DLONG start_y = file_inmarkers.at(0).y - 512;
        V3DLONG end_y = start_y + 1024 - 1;
        V3DLONG start_z = file_inmarkers.at(0).z - 256;
        V3DLONG end_z = start_z + 512 - 1;


//        QString swc_name = paras.empty() ? "" : paras[k]; if(swc_name == "NULL") swc_name = ""; k++;
//        QString marker_name = paras.empty() ? "" : paras[k]; if(marker_name == "NULL") swc_name = ""; k++;

//        NeuronTree nt = readSWC_file(swc_name);
//        V3DLONG start_x = nt.listNeuron.at(0).x;
//        V3DLONG end_x = nt.listNeuron.at(0).x;
//        V3DLONG start_y = nt.listNeuron.at(0).y;
//        V3DLONG end_y = nt.listNeuron.at(0).y;
//        V3DLONG start_z = nt.listNeuron.at(0).z;
//        V3DLONG end_z = nt.listNeuron.at(0).z;

//        for(V3DLONG i = 1; i < nt.listNeuron.size();i++)
//        {
//            if(start_x>nt.listNeuron.at(i).x) start_x = nt.listNeuron.at(i).x;
//            if(end_x<nt.listNeuron.at(i).x) end_x = nt.listNeuron.at(i).x;

//            if(start_y>nt.listNeuron.at(i).y) start_y = nt.listNeuron.at(i).y;
//            if(end_y<nt.listNeuron.at(i).y) end_y = nt.listNeuron.at(i).y;

//            if(start_z>nt.listNeuron.at(i).z) start_z = nt.listNeuron.at(i).z;
//            if(end_z<nt.listNeuron.at(i).z) end_z = nt.listNeuron.at(i).z;

//        }

        unsigned char * total1dData = 0;
        total1dData = callback.getSubVolumeTeraFly(inimg_file.toStdString(),start_x,end_x+1,
                                                   start_y,end_y+1,start_z,end_z+1);
        V3DLONG mysz[4];
        mysz[0] = end_x -start_x +1;
        mysz[1] = end_y - start_y +1;
        mysz[2] = end_z - start_z +1;
        mysz[3] = 1;
        QString imageSaveString = apo_name + "_cropped.v3dpbd";
        simple_saveimage_wrapper(callback, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, 1);

        vector<MyMarker> marker_inmarkers;
        QString shifted_marker_name = apo_name + "_cropped.marker";
        ImageMarker outputMarker;
        QList<ImageMarker> seedsToSave;
        outputMarker.x = file_inmarkers.at(0).x - start_x;
        outputMarker.y = file_inmarkers.at(0).y - start_y;
        outputMarker.z = file_inmarkers.at(0).z - start_z;
        seedsToSave.append(outputMarker);
        writeMarker_file(shifted_marker_name, seedsToSave);

//        vector<MyMarker> file_inmarkers = readMarker_file(string(qPrintable(marker_name)));


//        QString shifted_marker_name = marker_name + "_cropped.marker";
//        ImageMarker outputMarker;
//        QList<ImageMarker> seedsToSave;
//        outputMarker.x = file_inmarkers.at(0).x - start_x;
//        outputMarker.y = file_inmarkers.at(0).y - start_y;;
//        outputMarker.z = file_inmarkers.at(0).z - start_z;
//        seedsToSave.append(outputMarker);
//        writeMarker_file(shifted_marker_name, seedsToSave);
//        for(V3DLONG i = 0; i < nt.listNeuron.size();i++)
//        {
//            nt.listNeuron[i].x -= start_x;
//            nt.listNeuron[i].y -= start_y;
//            nt.listNeuron[i].z -= start_z;
//        }
//        QString swcString = marker_name + "_cropped.v3draw_manual.swc";
//        writeSWC_file(swcString,nt);


    }else if (func_name == tr("shift_swc"))
    {
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        int k=0;
        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        QString swc_name = paras.empty() ? "" : paras[k]; if(swc_name == "NULL") swc_name = ""; k++;

        vector<MyMarker> file_inmarkers = readMarker_file(string(qPrintable(inmarker_file)));

        V3DLONG start_x,start_y,start_z;
        start_x = file_inmarkers[0].x;
        start_y = file_inmarkers[0].y;
        start_z = file_inmarkers[0].z;

        for(int i =1; i < file_inmarkers.size();i++)
        {
            start_x = (start_x < file_inmarkers[i].x)? start_x:file_inmarkers[i].x;
            start_y = (start_y < file_inmarkers[i].y)? start_y:file_inmarkers[i].y;
            start_z = (start_z < file_inmarkers[i].z)? start_z:file_inmarkers[i].z;
        }

        start_x -= 50; start_y -=50; start_z -=50;
        NeuronTree nt = readSWC_file(swc_name);
        for(V3DLONG i =0; i < nt.listNeuron.size();i++)
        {
            nt.listNeuron[i].x += start_x;
            nt.listNeuron[i].y += start_y;
            nt.listNeuron[i].z += start_z;
            nt.listNeuron[i].type = 2;
        }
        QString swc_name_output = swc_name + "_global.swc";
        writeSWC_file(swc_name_output,nt);

    }
    else if (func_name == tr("linker"))
    {
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        QString cellfile = paras[0];
        QString markerfile = paras[1];

       // QString qs_filename_out = "/local4/Data/MOST_MOUSE/manual_22_branches/" + cellfile + "/" + markerfile + "_linker.ano";
        QString qs_filename_out = cellfile + "_linker.ano";
        QFile qf_anofile(qs_filename_out);
        if(!qf_anofile.open(QIODevice::WriteOnly))
        {
            v3d_msg("Cannot open file for writing!",0);
            return false;
        }

        QTextStream out(&qf_anofile);
//        out << "RAWIMG=" << markerfile << "/" << markerfile << ".marker_gd.v3dpbd"<<endl;
//        out << "SWCFILE=" << markerfile << "/" << markerfile << ".marker_gd_shifted.swc"<<endl;
//        out << "SWCFILE=" <<markerfile << "/" << markerfile << ".marker_manual_shifted.swc"<<endl;
//        out << "APOFILE=" <<markerfile << "/" << markerfile << ".marker_shifted.apo"<<endl;
        out << "SWCFILE=" << cellfile << "/" << cellfile << ".marker_cropped.v3draw_manual.swc"<<endl;
        out << "SWCFILE=" << markerfile<<endl;

    }
    else if (func_name == tr("help"))
    {

		printf("**** Usage of line_detector tracing **** \n");
        printf("vaa3d -x line_detector -f GD_Curveline_infinite -i <inimg_file> -p <marker_file> <win_size> <angle_size> <channel> <inputswc_file> -o <outswc_file>\n");
        printf("inimg_file       The input image\n");
        printf("marker_file      Starting location\n");
        printf("win_size         Window size for GD tracing (default 32).\n");
        printf("angle_size       Window size for angle calculation (default 5).\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("inputswc_file    Input swc file to guide the tracing direction.\n");
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
        PARA.angle_size = 10;
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
    trace_para.b_estRadii = false;

    int markSize = PARA.listLandmarks.size();
    if (PARA.nt.listNeuron.size() > 0 && PARA.listLandmarks.at(markSize-1).category !=1)
    {
        for(V3DLONG i=0; i<PARA.nt.listNeuron.size();i++)
        {
            if(NTDIS(PARA.nt.listNeuron.at(i),PARA.listLandmarks.at(markSize-1))<5)
            {
                   PARA.listLandmarks.removeAt(markSize-1);
                   callback.setLandmark(curwin,PARA.listLandmarks);
                   callback.updateImageWindow(curwin);
                   callback.pushObjectIn3DWindow(curwin);
                   return(PARA.listLandmarks.size()==0)? 1 : 0;
            }
        }
    }
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

    V3DLONG stacksz = N*M*P*sc;
    unsigned char* data1d_mask = 0;
    data1d_mask = new unsigned char [stacksz];
    memset(data1d_mask,0,stacksz*sizeof(unsigned char));
    double margin=3;//by PHC 20170531
    if (PARA.nt.listNeuron.size() > 0)
        ComputemaskImage(PARA.nt, data1d_mask, N, M, P, margin);
    else if (PARA.nt_input.listNeuron.size() > 0)
    {
        margin = 10;
        ComputemaskImage(PARA.nt_input, data1d_mask, N, M, P, margin);
    }

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
        v3d_msg("fail to call swc sorting function.",0);
        if(localarea) {delete []localarea; localarea = 0;}
        if(p4d) {delete []p4d; p4d = 0;}
        QFile file (swc_name);file.remove();
        return -1;
    }
    export_list2file(neuron_sorted, swc_name,swc_name);
    if(neuron_sorted.size() <=0)
    {
        v3d_msg("cannot find any path based on the given marker.",0);
        if(localarea) {delete []localarea; localarea = 0;}
        if(p4d) {delete []p4d; p4d = 0;}
        QFile file (swc_name);file.remove();
        PARA.listLandmarks.removeAt(markSize-1);
        return(PARA.listLandmarks.size()==0)? 1 : 0;
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
    NeuronTree nt_original = PARA.nt;
    for (int i =0; i<nt_list.size();i++)
    {
        QList<NeuronSWC> nt_seg = nt_list.at(i);
        int break_id = nt_seg.size();
//        printf("\nid %d\n",i);
        int angle_size = PARA.angle_size;
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

//            printf("(%.2f,%d)\n",angle_j,j);
            if(angle_j < 120) //this angle is also quite sensitive it seems. by PHC 170608
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

        if(seg_intensity/break_id >= seg_mean_max && break_id > 5)
        {
            seg_mean_max = seg_intensity/break_id;
            seg_tip_id = i;
            break_id_optimal = break_id;
        }

//        QString swc_seg = swc_name + QString("%1.swc").arg(i);
//        for(d = nt_seg.size() -1; d >= break_id;d--)
//            nt_seg[d].type = 1;
//        export_list2file(nt_seg, swc_seg,swc_name);
    }
//    QString swc_seg = swc_name + QString("%1.swc").arg(seg_tip_id);
//    QList<NeuronSWC> nt_seg = nt_list.at(seg_tip_id);
//    for(d = nt_seg.size() -1; d >= break_id_optimal;d--)
//        nt_seg[d].type = 1;
//    export_list2file(nt_seg, swc_seg,swc_name);
//    v3d_msg(QString("id is %1").arg(seg_tip_id));
    if(seg_mean_max ==0)
    {
        if(localarea) {delete []localarea; localarea = 0;}
        if(p4d) {delete []p4d; p4d = 0;}
        PARA.listLandmarks.removeAt(markSize-1);
        if(bmenu)
        {
            callback.setLandmark(curwin,PARA.listLandmarks);
            callback.pushObjectIn3DWindow(curwin);
        }
        return(PARA.listLandmarks.size()==0)? 1 : 0;
    }
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
                newmarker.category = 1;
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
    bool b_darkSegment = false;
    if(nt_selected.size() ==0 || (overall_mean-curve_mean < (overall_std+curve_std)/2 && overall_mean-curve_mean>=0))
    {
        nt_selected.clear();
        PARA.listLandmarks.removeAt(markSize-1);
        b_darkSegment = true;
        v3d_msg("Dark segment!",0);
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

    if (0)
    {
        printf("last dd = %d %5.3f %5.3f %5.3f \n", nt_selected.size(),
               nt_selected.at(nt_selected.size()-1).x - nt_selected.at(0).x,
               nt_selected.at(nt_selected.size()-1).y - nt_selected.at(0).y,
               nt_selected.at(nt_selected.size()-1).z - nt_selected.at(0).z);
        v3d_msg("press to continue");
    }

    PARA.nt = nt_original;
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

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    return ((b_boundary || b_darkSegment) && PARA.listLandmarks.size()==0) ? 1 : 0;
}

int reconstruction_func_v2(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
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

    //meanshift first before tracing

//    mean_shift_fun fun_obj;
//    vector<V3DLONG> poss_landmark;
//    vector<float> mass_center;
//    double windowradius = 5;

//    fun_obj.pushNewData<unsigned char>((unsigned char*)data1d, in_sz);
//    poss_landmark=landMarkList2poss(PARA.listLandmarks, in_sz[0], in_sz[0]*in_sz[1]);
//    PARA.listLandmarks.clear();
//    for (V3DLONG j=0;j<poss_landmark.size();j++)
//    {
//        mass_center=fun_obj.mean_shift_center_mass(poss_landmark[j],windowradius);
//        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
//        PARA.listLandmarks.push_back(tmp);
//    }

    V3DLONG stacksz = N*M*P*sc;
    unsigned char* data1d_mask = 0;
    data1d_mask = new unsigned char [stacksz];
    memset(data1d_mask,0,stacksz*sizeof(unsigned char));
    double margin=20;//by PHC 20170531
    if (PARA.nt_input.listNeuron.size() > 0)
        ComputemaskImage(PARA.nt_input, data1d_mask, N, M, P, margin);

    //GD_tracing
    LocationSimple p0;

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
    trace_para.b_estRadii = false;
    trace_para.sp_downsample_method = 1;

    int markSize = PARA.listLandmarks.size();
    p0.x = PARA.listLandmarks.at(markSize-1).x-1;
    p0.y = PARA.listLandmarks.at(markSize-1).y-1;
    p0.z = PARA.listLandmarks.at(markSize-1).z-1;


    V3DLONG pagesz = N*M*P;
    vector<pair<V3DLONG,MyMarker> > vp;
    vp.reserve(pagesz);

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                MyMarker t;
                t.x = ix;
                t.y = iy;
                t.z = iz;
                V3DLONG I_t = (data1d_mask[offsetk + offsetj + ix] == 0) ? data1d[offsetk + offsetj + ix] : 1;
                vp.push_back(make_pair(I_t, t));
            }
        }
    }

    sort(vp.begin(), vp.end());
    vector<MyMarker> file_inmarkers;
    file_inmarkers.push_back(vp[vp.size()-1].second);
    for (size_t i = vp.size()-2 ; i >=0; i--)
    {
        bool flag =false;
        for(int j = 0; j < file_inmarkers.size();j++)
        {
            if(NTDIS(file_inmarkers.at(j),vp[i].second) < N/15)
            {
                flag = true;
                break;
            }
        }
        if(!flag)
            file_inmarkers.push_back(vp[i].second);

        if(file_inmarkers.size()>40)
            break;
    }
    vp.clear();

    for(int i =0; i < file_inmarkers.size();i++)
    {
        LocationSimple p;
        p.x = file_inmarkers.at(i).x+1;
        p.y = file_inmarkers.at(i).y+1;
        p.z = file_inmarkers.at(i).z+1;
        PARA.listLandmarks.push_back(p);
    }

    double overall_mean, overall_std;
    mean_and_std(data1d, N*M*P, overall_mean, overall_std);
    printf("overall mean is %.2f, std is %.2f\n",overall_mean, overall_std);


    vector<LocationSimple> pp;
    for(V3DLONG i = 0; i <PARA.listLandmarks.size();i++)
        pp.push_back(PARA.listLandmarks.at(i));


    unsigned char ****p4d = 0;
    if (!new4dpointer(p4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3], data1d))
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        if(p4d) {delete []p4d; p4d = 0;}
        return false;
    }

    NeuronTree nt = v3dneuron_GD_tracing(p4d, in_sz,
                              p0, pp,
                              trace_para, weight_xy_z);

    if(nt.listNeuron.size()<=0)
    {
        if(p4d) {delete []p4d; p4d = 0;}
        PARA.listLandmarks.removeAt(0);
        return -1;
    }
    QString swc_name = PARA.inimg_file+"_1st.swc";
    export_list2file(nt.listNeuron, swc_name,swc_name);

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
        v3d_msg("fail to call swc sorting function.",0);
        if(p4d) {delete []p4d; p4d = 0;}
        QFile file (swc_name);file.remove();
        return false;
    }

    if(p4d) {delete []p4d; p4d = 0;}

    export_list2file(neuron_sorted, swc_name,swc_name);
    nt = readSWC_file(swc_name);
   // QFile file (swc_name);file.remove();
    QList<NeuronSWC> list = nt.listNeuron;
    vector<QList<NeuronSWC> > nt_list;

    for(int j = 0; j< PARA.listLandmarks.size();j++)
    {
        QList<NeuronSWC> nt_seg;
        for (int i=0;i<list.size();i++)
        {
            if (NTDIS(PARA.listLandmarks.at(j),list.at(i))< 2)
            {
                int index_i = i;
                while(index_i != 1000000000)
                {
                    nt_seg.push_front(list.at(index_i));
                    index_i = getParent(index_i,nt);
                }
                nt_list.push_back(nt_seg);
                nt_seg.clear();
                break;
            }
        }
    }

    double I_max = -1000000;
    int seg_tip_id = -1;
    for (int i =0; i<nt_list.size();i++)
    {
        QList<NeuronSWC> nt_seg = nt_list.at(i);
        double seg_intensity = 0;
        double seg_angle;
        bool flag = false;
        for(int j = 0; j < nt_seg.size(); j++)
        {
            V3DLONG  ix = nt_seg[j].x;
            V3DLONG  iy = nt_seg[j].y;
            V3DLONG  iz = nt_seg[j].z;
            seg_intensity += data1d[iz*in_sz[0]*in_sz[1]+ iy *in_sz[0] + ix] - (overall_mean + 0*overall_std); //was 5 *overall_std
            if(j >=7 && j<nt_seg.size()-7)
            {
                seg_angle = angle(nt_seg[j], nt_seg[j-7], nt_seg[j+7]);
                if(seg_angle < 120)
                {
//                    double I_part1 = 0;
//                    double I_part2 = 0;
//                    for(int d = 1; d <= 10;d++)
//                    {
//                        I_part1 += data1d[(V3DLONG)nt_seg[j-d].z*in_sz[0]*in_sz[1]+ (V3DLONG)nt_seg[j-d].y *in_sz[0] + (V3DLONG)nt_seg[j-d].x];
//                        I_part2 += data1d[(V3DLONG)nt_seg[j+d].z*in_sz[0]*in_sz[1]+ (V3DLONG)nt_seg[j+d].y *in_sz[0] + (V3DLONG)nt_seg[j+d].x];
//                    }
//                    if(fabs(I_part1-I_part2)/10 > 20.0)
//                    {
//                        v3d_msg(QString("%1").arg(fabs(I_part1-I_part2)/10));
                        flag = true;
                        break;
//                    }
                }
            }
        }
        if(seg_intensity >= I_max && !flag)
        {
            I_max = seg_intensity;
            seg_tip_id = i;
        }
    }

    if(seg_tip_id ==-1)
    {
        v3d_msg("no path!",bmenu);
        return -1;
    }

    if(bmenu)
    {
        QString swc_seg = swc_name +QString("_x%1_y%2_z%3_I_%4.swc").arg(p0.x).arg(p0.y).arg(p0.z).arg(I_max);
        export_list2file(nt_list.at(seg_tip_id), swc_seg,swc_seg);
    }else
    {
        PARA.nt.listNeuron = nt_list.at(seg_tip_id);
    }
    return 1;
}

int curveFitting_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
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

    V3DLONG pagesz = N*M*P;
    vector<pair<V3DLONG,MyMarker> > vp;
    vp.reserve(pagesz);

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                MyMarker t;
                t.x = ix;
                t.y = iy;
                t.z = iz;
                vp.push_back(make_pair(data1d[offsetk+offsetj+ix], t));
            }
        }
    }

    sort(vp.begin(), vp.end());
    vector<MyMarker> file_inmarkers;
    file_inmarkers.push_back(vp[vp.size()-1].second);
    for (size_t i = vp.size()-2 ; i >=0; i--)
    {
        bool flag =false;
        for(int j = 0; j < file_inmarkers.size();j++)
        {
            if(NTDIS(file_inmarkers.at(j),vp[i].second) < N/15)
            {
                flag = true;
                break;
            }
        }
        if(!flag)
            file_inmarkers.push_back(vp[i].second);

        if(file_inmarkers.size()>40)
            break;
    }
    vp.clear();

    for(int i =0; i < file_inmarkers.size();i++)
    {
        LocationSimple p;
        p.x = file_inmarkers.at(i).x+1;
        p.y = file_inmarkers.at(i).y+1;
        p.z = file_inmarkers.at(i).z+1;
        PARA.listLandmarks.push_back(p);
    }
    callback.setLandmark(curwin,PARA.listLandmarks);
    callback.updateImageWindow(curwin);
    callback.pushObjectIn3DWindow(curwin);

  //  PARA.angle_size = 2;

    for (;;)
    {
        printf("\n\n~~~~~~~~~ Enter again @@@@@@@@@@@@@@@@@@@\n\n");
        int res = reconstruction_func(callback,parent,PARA,bmenu);
        if (res!=0)
            break;
    }
}

bool curveFitting_func_v2(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
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

    V3DLONG pagesz = N*M*P;
    vector<pair<V3DLONG,MyMarker> > vp;
    vp.reserve(pagesz);

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                MyMarker t;
                t.x = ix;
                t.y = iy;
                t.z = iz;
                vp.push_back(make_pair(data1d[offsetk+offsetj+ix], t));
            }
        }
    }

    sort(vp.begin(), vp.end());
    vector<MyMarker> file_inmarkers;
    file_inmarkers.push_back(vp[vp.size()-1].second);
    for (size_t i = vp.size()-2 ; i >=0; i--)
    {
        bool flag =false;
        for(int j = 0; j < file_inmarkers.size();j++)
        {
            if(NTDIS(file_inmarkers.at(j),vp[i].second) < N/15)
            {
                flag = true;
                break;
            }
        }
        if(!flag)
            file_inmarkers.push_back(vp[i].second);

        if(file_inmarkers.size()>40)
            break;
    }
    vp.clear();

    for(int i =0; i < file_inmarkers.size();i++)
    {
        LocationSimple p;
        p.x = file_inmarkers.at(i).x+1;
        p.y = file_inmarkers.at(i).y+1;
        p.z = file_inmarkers.at(i).z+1;
        PARA.listLandmarks.push_back(p);
    }
    callback.setLandmark(curwin,PARA.listLandmarks);
    callback.updateImageWindow(curwin);
    callback.pushObjectIn3DWindow(curwin);

    double overall_mean, overall_std;
    mean_and_std(data1d, N*M*P, overall_mean, overall_std);
    printf("overall mean is %.2f, std is %.2f\n",overall_mean, overall_std);


    V3DLONG stacksz = N*M*P*sc;
    unsigned char* data1d_mask = 0;
    data1d_mask = new unsigned char [stacksz];
    memset(data1d_mask,0,stacksz*sizeof(unsigned char));

    NeuronTree nt_total;
    LandmarkList checkPoints;
    while(PARA.listLandmarks.size()>1)
    {
        LocationSimple p0 = PARA.listLandmarks.at(0);
        vector<LocationSimple> pp;
        for(V3DLONG i = 1; i <PARA.listLandmarks.size();i++)
            pp.push_back(PARA.listLandmarks.at(i));

        if(nt_total.listNeuron.size()>0)
            ComputemaskImage(nt_total, data1d_mask, N, M, P, 3);

        unsigned char* data1d_updated = 0;
        data1d_updated = new unsigned char [stacksz];
        for(V3DLONG i =0; i < stacksz; i++)
        {
            data1d_updated[i] = (data1d_mask[i] == 0)? data1d[i] : 0;
        }

        unsigned char ****p4d = 0;
        if (!new4dpointer(p4d, in_sz[0], in_sz[1], in_sz[2], in_sz[3], data1d_updated))
        {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            if(p4d) {delete []p4d; p4d = 0;}
            return false;
        }


        double weight_xy_z=1.0;
        CurveTracePara trace_para;
        trace_para.channo = 0;
        trace_para.sp_graph_background = 0;
        trace_para.b_postMergeClosebyBranches = false;
        trace_para.sp_graph_resolution_step=2;
        trace_para.b_3dcurve_width_from_xyonly = true;
        trace_para.b_pruneArtifactBranches = false;
        trace_para.sp_num_end_nodes = 2;
        trace_para.b_deformcurve = false;
        trace_para.sp_graph_resolution_step = 1;
        trace_para.b_estRadii = false;

        NeuronTree nt = v3dneuron_GD_tracing(p4d, in_sz,
                                  p0, pp,
                                  trace_para, weight_xy_z);

        if(nt.listNeuron.size()<=0)
        {
            if(data1d_updated) {delete []data1d_updated; data1d_updated = 0;}
            if(p4d) {delete []p4d; p4d = 0;}
            PARA.listLandmarks.removeAt(0);
            continue;
        }
        QString swc_name = PARA.inimg_file+"_1st.swc";
        export_list2file(nt.listNeuron, swc_name,swc_name);

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
            v3d_msg("fail to call swc sorting function.",0);
            if(p4d) {delete []p4d; p4d = 0;}
            QFile file (swc_name);file.remove();
            return false;
        }

        if(p4d) {delete []p4d; p4d = 0;}

        export_list2file(neuron_sorted, swc_name,swc_name);
        nt = readSWC_file(swc_name);
       // QFile file (swc_name);file.remove();
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

        checkPoints = PARA.listLandmarks;
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

        double I_max = 0;
        int seg_tip_id;
        for (int i =0; i<nt_list.size();i++)
        {
            QList<NeuronSWC> nt_seg = nt_list.at(i);
            double seg_intensity = 0;
            double seg_angle;
            bool flag = false;
            for(int j = 0; j < nt_seg.size(); j++)
            {
                V3DLONG  ix = nt_seg[j].x;
                V3DLONG  iy = nt_seg[j].y;
                V3DLONG  iz = nt_seg[j].z;
                seg_intensity += data1d_updated[iz*in_sz[0]*in_sz[1]+ iy *in_sz[0] + ix] - (overall_mean + 5*overall_std);
                if(nt_seg.size() >5)
                {
                    for(int d = 5; d < nt_seg.size()-5; d++)
                    {
                        seg_angle = angle(nt_seg[d], nt_seg[d-5], nt_seg[d+5]);
                        if(seg_angle < 120) //this angle is also quite sensitive it seems. by PHC 170608
                        {
                            flag = true;
                            break;
                        }
                    }
                }

            }
            if(seg_intensity >= I_max && !flag)
            {
                I_max = seg_intensity;
                seg_tip_id = i;
            }
        }


        QList<NeuronSWC> nt_seg_optimal = nt_list.at(seg_tip_id);
        for(V3DLONG i =0 ; i<nt_seg_optimal.size();i++)
        {
            nt_total.listNeuron.push_back(nt_seg_optimal.at(i));
        }
        QString swc_seg = swc_name +QString("_x%1_y%2_z%3_I_%4.swc").arg(p0.x).arg(p0.y).arg(p0.z).arg(I_max);
        export_list2file(nt_list.at(seg_tip_id), swc_seg,swc_seg);
        for(V3DLONG i = PARA.listLandmarks.size()-1; i >=0;i--)
        {
            bool flag = false;
            for(V3DLONG j = 0; j < nt_total.listNeuron.size();j++)
            {
                if(NTDIS(PARA.listLandmarks.at(i),nt_total.listNeuron.at(j))<3)
                {
                   flag = true;
                   break;
                }
            }
            if(flag)
                PARA.listLandmarks.removeAt(i);
        }
        if(data1d_updated) {delete []data1d_updated; data1d_updated = 0;}

    }

    if(data1d_mask) {delete []data1d_mask; data1d_mask = 0;}

}
