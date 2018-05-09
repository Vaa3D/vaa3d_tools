/* neurontracer_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-2-16 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracer_plugin.h"
#include "tracing_func.h"

#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"

using namespace std;

Q_EXPORT_PLUGIN2(neurontracer,neurontracer);
static lookPanel *panel = 0;
NeuronTree nt;
 
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
    else if (menu_name == tr("trace_NEUTUBE"))
    {
        TRACE_LS_PARA P;
        bool bmenu = false;
        //int curr_model=0;
        if(bmenu)
        {
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
            //  P.adap_win = dialog.adap_win;

            // P.tracing_3D = dialog.tracing_3D;
        }
        P.method = neutube;
        grid_raw_all(callback,parent,P,bmenu);

        QString name = P.inimg_file+"_neutube.swc";
        nt = readSWC_file(name);
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            nt.listNeuron[i].x = nt.listNeuron[i].x*P.ratio_x +P.o_x;
            nt.listNeuron[i].y = nt.listNeuron[i].y*P.ratio_y+P.o_y;
            nt.listNeuron[i].z = nt.listNeuron[i].z*P.ratio_z+P.o_z;
        }
        if (panel)
        {
            panel->show();
            return;
        }
        else
        {
            panel = new lookPanel(callback, parent);

            if (panel)
            {
                panel->show();
                panel->raise();
                panel->move(100,100);
                panel->activateWindow();
            }
        }
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
        //extract_tips(callback,parent,P);

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
//        if(P.grid_trace)
            grid_raw_all(callback,parent,P,bmenu);
//        else
//            crawler_raw_all(callback,parent,P,bmenu);
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
        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
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
        printf("inimg_file  }     Should be 8 bit image\n");
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
lookPanel::lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{

    gridLayout = new QGridLayout();
    QPushButton* sync     = new QPushButton("Sync (one shot)");
    QPushButton* set_markers     = new QPushButton("Set Annotations");
    gridLayout->addWidget(sync, 0,0);
    gridLayout->addWidget(set_markers, 1,0);
    setLayout(gridLayout);
    setWindowTitle(QString("Synchronize annotation "));
    connect(sync,     SIGNAL(clicked()), this, SLOT(_slot_sync_onetime()));
    connect(set_markers,     SIGNAL(clicked()), this, SLOT(_slot_set_markers()));

}

lookPanel::~lookPanel()
{
}


void lookPanel::_slot_sync_onetime()
{
    cout<<"this is slot sync onetime"<<endl;
    NeuronTree nt_p = m_v3d.getSWCTeraFly();
    NeuronTree  resultTree;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    V3DLONG max_id=0;

    for(int j = 0; j < nt_p.listNeuron.size(); j++)
    {
        listNeuron.append(nt_p.listNeuron.at(j));
        hashNeuron.insert(nt_p.listNeuron.at(j).n, listNeuron.size()-1);
    }
    for(V3DLONG i=0;i<nt_p.listNeuron.size();i++)
    {
        if(nt_p.listNeuron[i].n>max_id)
        {
            max_id = nt_p.listNeuron[i].n;
        }
    }
    resultTree.listNeuron = listNeuron;
    resultTree.hashNeuron = hashNeuron;
    resultTree.color.r = 0;
    resultTree.color.g = 0;
    resultTree.color.b = 0;
    resultTree.color.a = 0;
    QString namelxf = "jjjjjjjjjjjjjjj.swc";
    writeSWC_file(namelxf,resultTree);
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        nt.listNeuron[i].n = nt.listNeuron[i].n + max_id;
        if(nt.listNeuron[i].pn!=-1)
        {
            nt.listNeuron[i].pn = nt.listNeuron[i].pn + max_id;
        }
        resultTree.listNeuron.push_back(nt.listNeuron[i]);
    }
    QString outname = "hahhaah.swc";
    writeSWC_file(outname,resultTree);
    QList <V3dR_MainWindow *> list_3dviewer = m_v3d.getListAll3DViewers();
    int current_index = -1;
    for(int i=0; i<list_3dviewer.size();i++)
    {
        if(m_v3d.getImageName(list_3dviewer[i]) == "3D View [Annotations in TeraFly]")
        {
            current_index = i;
         }
    }

    LandmarkList terafly_landmarks = m_v3d.getLandmarkTeraFly();
    if(current_index == -1)
    {
        V3dR_MainWindow * new3DWindow = NULL;
        new3DWindow = m_v3d.createEmpty3DViewer();
        QList<NeuronTree> * treeList = m_v3d.getHandleNeuronTrees_Any3DViewer (new3DWindow);
        treeList->push_back(resultTree);
        m_v3d.setWindowDataTitle(new3DWindow, "Annotations in TeraFly");
        m_v3d.setHandleLandmarkList_Any3DViewer(new3DWindow,terafly_landmarks);
        m_v3d.update_NeuronBoundingBox(new3DWindow);
        m_v3d.setSWCTeraFly(resultTree);

    }else
    {
        V3dR_MainWindow *surface_win = m_v3d.getListAll3DViewers()[current_index];
        QList<NeuronTree> * treeList = m_v3d.getHandleNeuronTrees_Any3DViewer (surface_win);
        treeList->clear();
        treeList->push_back(resultTree);
        m_v3d.setHandleLandmarkList_Any3DViewer(surface_win,terafly_landmarks);
        m_v3d.update_NeuronBoundingBox(surface_win);
        m_v3d.setSWCTeraFly(resultTree);
    }
}

void lookPanel::_slot_set_markers()
{
    cout<<"this is slot set markers"<<endl;
    QList <V3dR_MainWindow *> list_3dviewer = m_v3d.getListAll3DViewers();
    int current_index = -1;
    for(int i=0; i<list_3dviewer.size();i++)
    {
        if(m_v3d.getImageName(list_3dviewer[i]) == "3D View [Annotations in TeraFly]")
        {
            current_index = i;
         }
    }
    if(current_index != -1)
    {
        V3dR_MainWindow *surface_win = m_v3d.getListAll3DViewers()[current_index];
        LandmarkList* updated_landmarks = m_v3d.getHandleLandmarkList_Any3DViewer(surface_win);
        QList<NeuronTree> * updated_treeList = m_v3d.getHandleNeuronTrees_Any3DViewer(surface_win);
        m_v3d.setLandmarkTeraFly(*updated_landmarks);
        if(updated_treeList->size()>0){
            NeuronTree updated_nt = updated_treeList->at(0);
            m_v3d.setSWCTeraFly(updated_nt);

        }

    }

}

