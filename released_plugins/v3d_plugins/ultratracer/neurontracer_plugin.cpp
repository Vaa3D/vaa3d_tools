/* neurontracer_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-2-16 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracer_plugin.h"
#include "tracing_func.h"

#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../istitch/y_imglib.h"
#include "../neurontracing_vn2/app2/my_surf_objs.h"

using namespace std;

Q_EXPORT_PLUGIN2(neurontracer, neurontracer);

 
QStringList neurontracer::menulist() const
{
    return QStringList()
            <<tr("trace_APP2")
           <<tr("trace_APP1")
          <<tr("trace_MOST")
         <<tr("trace_NEUTUBE")
        <<tr("trace_SNAKE")
       <<tr("trace_MST")
      <<tr("trace_NeuroGPSTree")
     <<tr("trace_Rivulet2")
    <<tr("trace_TReMAP")
   <<tr("generate_final_result(3D)")

      //     <<tr("trace_Advantra")
      //   <<tr("trace_NeuronChaser")
    <<tr("about");
}

QStringList neurontracer::funclist() const
{
    return QStringList()
            <<tr("trace_APP2")
           <<tr("trace_APP2_GD")
          <<tr("trace_MOST")
         <<tr("")
        <<tr("trace_SNAKE")
       <<tr("trace_NeuroGPSTree")
      <<tr("trace_TReMAP")
     <<tr("trace_MST")
    <<tr("trace_NeuronChaser")
    <<tr("trace_Rivulet2")
    <<tr("trace_GD_curveline")
    <<tr("trace_pairs")
    <<tr("help");
}

void neurontracer::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("trace_APP2"))
	{
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_app2_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty() && dialog.teraflyfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        if(dialog.teraflyfilename.isEmpty())
            P.inimg_file = dialog.rawfilename;
        else
            P.inimg_file = dialog.teraflyfilename;

        P.is_gsdt = dialog.is_gsdt;
        P.is_break_accept = dialog.is_break_accept;
        P.bkg_thresh = dialog.bkg_thresh;
        P.length_thresh = dialog.length_thresh;
        P.cnn_type = dialog.cnn_type;
        P.channel = dialog.channel;
        P.SR_ratio = dialog.SR_ratio;
        P.b_256cube = dialog.b_256cube;
        P.b_RadiusFrom2D = dialog.b_RadiusFrom2D;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.method = app2;
        P.tracing_3D = dialog.tracing_3D;
        P.tracing_comb = dialog.tracing_comb;
        crawler_raw_app(callback,parent,P,bmenu);

    }else if (menu_name == tr("trace_APP1"))
	{
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_app1_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.bkg_thresh = dialog.bkg_thresh;
        P.channel = dialog.channel;
        P.b_256cube = dialog.b_256cube;
        P.visible_thresh = dialog.visible_thresh;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.method = app1;
        P.tracing_3D = false;

        crawler_raw_app(callback,parent,P,bmenu);
	}
    else if (menu_name == tr("trace_MOST"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_most_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty() && dialog.teraflyfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        if(dialog.teraflyfilename.isEmpty())
            P.inimg_file = dialog.rawfilename;
        else
            P.inimg_file = dialog.teraflyfilename;
        P.bkg_thresh = dialog.bkg_thresh;
        P.channel = dialog.channel;
        P.seed_win = dialog.seed_win;
        P.slip_win = dialog.slip_win;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.method = most;
        P.tracing_3D = dialog.tracing_3D;
        crawler_raw_all(callback,parent,P,bmenu);
    }
    else if (menu_name == tr(""))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty() && dialog.teraflyfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        if(dialog.teraflyfilename.isEmpty())
            P.inimg_file = dialog.rawfilename;
        else
            P.inimg_file = dialog.teraflyfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.method = neutube;
        P.tracing_3D = dialog.tracing_3D;
        if(!dialog.grid_trace)
            crawler_raw_all(callback,parent,P,bmenu);
        else
            grid_raw_all(callback,parent,P,bmenu);
    }else if (menu_name == tr("trace_SNAKE"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.tracing_3D = dialog.tracing_3D;
        P.method = snake;
        crawler_raw_all(callback,parent,P,bmenu);
    }else if (menu_name == tr("trace_NeuroGPSTree"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.tracing_3D = dialog.tracing_3D;
        P.method = neurogpstree;
        crawler_raw_all(callback,parent,P,bmenu);
    }else if (menu_name == tr("trace_Advantra"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.tracing_3D = dialog.tracing_3D;
        P.method = advantra;
        crawler_raw_all(callback,parent,P,bmenu);
    }else if (menu_name == tr("trace_TReMAP"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.tracing_3D = dialog.tracing_3D;
        P.method = tremap;
        crawler_raw_all(callback,parent,P,bmenu);
    }else if (menu_name == tr("trace_MST"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.tracing_3D = dialog.tracing_3D;
        P.method = mst;
        crawler_raw_all(callback,parent,P,bmenu);
    }else if (menu_name == tr("trace_NeuronChaser"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.tracing_3D = dialog.tracing_3D;
        P.method = neuronchaser;
        crawler_raw_all(callback,parent,P,bmenu);
    }else if (menu_name == tr("trace_Rivulet2"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neurontracer_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.tracing_3D = dialog.tracing_3D;
        P.method = rivulet2;
        crawler_raw_all(callback,parent,P,bmenu);
    }
    else if (menu_name == tr("generate_final_result(3D)"))
    {
        QString txtfilenName = QFileDialog::getOpenFileName(0, QObject::tr("Open TXT File"),
                                                        "",
                                                        QObject::tr("Supported file (*.txt *.TXT)"));
        if(txtfilenName.isEmpty())
            return;

        list<string> infostring;
        processSmartScan_3D(callback,infostring,txtfilenName);
        v3d_msg("Done!");

    }
    else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2016-2-16"));
	}
}

bool neurontracer::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    TRACE_LS_PARA P;
    bool bmenu = false;

    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

    if (func_name == tr("trace_APP2"))
	{
        if(infiles.empty())
        {
            cerr<<"Need input image"<<endl;
            return false;
        }

        P.inimg_file = infiles[0];
        P.image = 0;
        int k=0;

        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        if(inmarker_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }else
            P.markerfilename = inmarker_file;

        P.block_size = (paras.size() >= k+1) ? atof(paras[k]) : 1024; k++;
        P.adap_win = (paras.size() >= k+1) ? atof(paras[k]) : 0; k++;

        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 10; k++;
        P.b_256cube = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;
        P.b_RadiusFrom2D = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.is_gsdt = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;
        P.is_break_accept = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;
        P.length_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 5;  k++;
        P.tracing_3D = true;
        P.tracing_comb = false;
        P.method = app2;
//        P.global_name = true;
        crawler_raw_app(callback,parent,P,bmenu);
	}
    else if (func_name == tr("trace_pairs"))
	{
        if(infiles.empty())
        {
            cerr<<"Need input image"<<endl;
            return false;
        }

        P.inimg_file = infiles[0];
        P.image = 0;
        int k=0;

        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        if(inmarker_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }else
            P.markerfilename = inmarker_file;

        P.inimg_file_2nd = (paras.size() >= k+1) ? paras[k] : "NULL";
        P.method = gd;
        tracing_pair_app(callback,parent,P,bmenu);
	}
    else if (func_name == tr("trace_APP2_GD"))
    {
        if(infiles.empty())
        {
            cerr<<"Need input image"<<endl;
            return false;
        }

        P.inimg_file = infiles[0];
        P.image = 0;
        int k=0;

        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        if(inmarker_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }else
            P.markerfilename = inmarker_file;

        v3d_msg("enter trace_APP2_GD_before_parsing");
        P.block_size = (paras.size() >= k+1) ? atof(paras[k]) : 1024; k++;
        P.adap_win = (paras.size() >= k+1) ? atof(paras[k]) : 0; k++;

        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 10; k++;
        P.b_256cube = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;
        P.b_RadiusFrom2D = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.is_gsdt = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;
        P.is_break_accept = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;
        P.length_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 5;  k++;
        P.tracing_3D = true;
        P.tracing_comb = false;
        P.global_name = true;
        P.method = app2;
        crawler_raw_app(callback,parent,P,bmenu);
        v3d_msg("enter trace_APP2_GD_finish_crawler_raw_app");
        v3d_msg("enter trace_APP2_GD_finish_extract_tips");

    }
    else if (func_name == tr("trace_NEUTUBE"))
    {
        if(infiles.empty())
        {
            cerr<<"Need input image"<<endl;
            return false;
        }

        P.inimg_file = infiles[0];
        P.image = 0;
        int k=0;

        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        if(inmarker_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }else
            P.markerfilename = inmarker_file;

        P.block_size = (paras.size() >= k+1) ? atof(paras[k]) : 1024; k++;
        P.grid_trace = (paras.size() >= k+1) ? atof(paras[k]) : 0; k++;
        P.adap_win = (paras.size() >= k+1) ? atof(paras[k]) : 0; k++;
        P.method = neutube;
        P.tracing_3D = true;
        P.global_name = true;
        if(P.grid_trace)
            grid_raw_all(callback,parent,P,bmenu);
        else
            crawler_raw_all(callback,parent,P,bmenu);
    }
    else if (func_name == tr("trace_MOST"))
    {
        if(infiles.empty())
        {
            cerr<<"Need input image"<<endl;
            return false;
        }

        P.inimg_file = infiles[0];
        P.image = 0;
        int k=0;

        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        if(inmarker_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }else
            P.markerfilename = inmarker_file;

        P.block_size = (paras.size() >= k+1) ? atof(paras[k]) : 1024; k++;
        P.adap_win = (paras.size() >= k+1) ? atof(paras[k]) : 0; k++;

        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 10; k++;
        P.seed_win = (paras.size() >= k+1) ? atoi(paras[k]) : 20;  k++;
        P.slip_win = (paras.size() >= k+1) ? atoi(paras[k]) : 20;  k++;

        P.method = most;
        crawler_raw_all(callback,parent,P,bmenu);
    }
    else if (func_name == tr("trace_GD_curveline"))
    {
        if(infiles.empty())
        {
            cerr<<"Need input image"<<endl;
            return false;
        }

        P.inimg_file = infiles[0];
        P.image = 0;
        int k=0;

        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        if(inmarker_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }else
            P.markerfilename = inmarker_file;

        P.block_size = (paras.size() >= k+1) ? atof(paras[k]) : 1024; k++;
        P.adap_win = (paras.size() >= k+1) ? atof(paras[k]) : 0; k++;
        P.seed_win = (paras.size() >= k+1) ? atoi(paras[k]) : 32; k++;
        P.swcfilename = (paras.size() >= k+1) ? paras[k] : ""; k++;
        P.tracing_3D = true;
        P.tracing_comb = false;
        P.method = gd;
        crawler_raw_app(callback,parent,P,bmenu);
    }
	else if (func_name == tr("help"))
	{
        printf("\n**** Usage of UltraTracer plugin ****\n");
        printf("vaa3d -x plugin_name -f trace_APP2 -i <inimg_file> -p <inmarker_file> <block_size> <adaptive_win> <channel> <bkg_thresh> <b_256cube> <b_RadiusFrom2D> <is_gsdt> <is_gap> <length_thresh>\n");
        printf("inimg_file       Should be 8 bit image\n");
        printf("inmarker_file    Please specify the path of the marker file\n");
        printf("block_size       Default 1024\n");
        printf("adaptive_win     If use adaptive block size (1 for yes and 0 for no. Default 0.)\n");

        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("bkg_thresh       Default 10 (is specified as AUTO then auto-thresolding)\n");
        printf("b_256cube        If trace in a auto-downsampled volume (1 for yes, and 0 for no. Default 1.)\n");
        printf("b_RadiusFrom2D   If estimate the radius of each reconstruction node from 2D plane only (1 for yes as many times the data is anisotropic, and 0 for no. Default 1 which which uses 2D estimation.)\n");
        printf("is_gsdt          If use gray-scale distance transform (1 for yes and 0 for no. Default 0.)\n");
        printf("is_gap           If allow gap (1 for yes and 0 for no. Default 0.)\n");
        printf("length_thresh    Default 5\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

        printf("vaa3d -x plugin_name -f trace_NEUTUBE -i <inimg_file> -p <inmarker_file> <block_size> <adaptive_win>\n");
        printf("inimg_file       Should be 8 bit image\n");
        printf("inmarker_file    Please specify the path of the marker file\n");
        printf("block_size       Default 1024\n");
        printf("adaptive_win     If use adaptive block size (1 for yes and 0 for no. Default 0.)\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

        printf("vaa3d -x plugin_name -f trace_MOST -i <inimg_file> -p <inmarker_file> <block_size> <adaptive_win> <channel> <bkg_thresh> <seed> <slip>\n");
        printf("inimg_file       Should be 8 bit image\n");
        printf("inmarker_file    Please specify the path of the marker file\n");
        printf("block_size       Default 1024\n");
        printf("adaptive_win     If use adaptive block size (1 for yes and 0 for no. Default 0.)\n");

        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("bkg_thresh       Default 10.\n");
        printf("seed             window size of the seed, default 20.\n");
        printf("slip             window size to slip from seed, default 20\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

