/* neurontracer_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-2-16 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracer_plugin.h"
#include "tracing_func.h"
#include "stdio.h"



#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
//#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
using namespace std;
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
Q_EXPORT_PLUGIN2(neurontracer,neurontracer);
static lookPanel *panel = 0;
LocationSimple next_m,next_m_rebase;
NeuronTree trace_result_pp,trace_result_p,trace_result,resultTree_rebase,resultTree;
extern LandmarkList marker_rebase,marker_rebase2; //original marker and marker which use for app2
extern LandmarkList marker_rebase3; //only original marker
extern V3DLONG thres_rebase;
bool change = true;
int check_void=0;
extern QString outimg_file;
extern bool is_soma;
int thresh=42;
int func_name;
struct relationship
{
    double datald;
    double x;
    double y;
    double z;
};
void sort_bubble(vector<double> &min_dis_v);
void move_block(LocationSimple &next_m);
bool point_at_boundry(V3DPluginCallback2 &callback,NeuronSWC &s,vector<int> &count_v,vector<NeuronSWC> &point_b);
QStringList neurontracer::menulist() const
{
    return QStringList()
            <<tr("trace_APP2")
     //      <<tr("trace_APP1")
      //    <<tr("trace_MOST")
         <<tr("trace_NEUTUBE")
    //    <<tr("trace_SNAKE")
    //   <<tr("trace_MST")
   //   <<tr("trace_NeuroGPSTree")
  //   <<tr("trace_Rivulet2")
  //  <<tr("trace_TReMAP")
 //  <<tr("generate_final_result(3D)")

      //     <<tr("trace_Advantra")
      //   <<tr("trace_NeuronChaser")
           <<tr("move_to_another_block_with_marker")
             <<tr("set_thresh")
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



    if (menu_name == tr("trace_APP2"))     //finished
	{

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
        return;


        TRACE_LS_PARA P;
        bool bmenu = false;
        if(bmenu)
        {
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
            P.tracing_3D = dialog.tracing_3D;
            P.tracing_comb = dialog.tracing_comb;
        }
        P.method = app2;
        func_name = app2;
        if(!crawler_raw_app(callback,parent,P,bmenu))return;

        QString name = P.inimg_file+"_app2.swc";
        trace_result_pp = readSWC_file(name);


        if(trace_result_pp.listNeuron.size()!=0)   //check if the tracing result is void
        {
            marker_rebase = marker_rebase2;
        }
        else
        {
            check_void++;
            //cout<<"check  "<<check_void<<endl;
            v3d_msg("this tracing has no result,please press A to have another try.If there is still no result,please make sure your marker is in the right position!");
            thresh = thres_rebase ;
            return;

        }
        for(V3DLONG i=0;i<trace_result_pp.listNeuron.size();i++)   //use ratio to update the coordinate
        {
            trace_result_pp.listNeuron[i].x = trace_result_pp.listNeuron[i].x*P.ratio_x + P.o_x;
            trace_result_pp.listNeuron[i].y = trace_result_pp.listNeuron[i].y*P.ratio_y + P.o_y;
            trace_result_pp.listNeuron[i].z = trace_result_pp.listNeuron[i].z*P.ratio_z + P.o_z;
        }

        trace_result.listNeuron.clear();
        vector<int> count_v;
        vector<NeuronSWC> point_b;
        //v3d_msg("remove point at boundry");
        trace_result_p = trace_result_pp; //tmp need to be modified&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

        for(V3DLONG i=0;i<trace_result_p.listNeuron.size();i++)  //remove point at boundry
        {
            if(point_at_boundry(callback,trace_result_p.listNeuron[i],count_v,point_b))
            {
                if(trace_result_p.listNeuron[i].pn!=-1)
                {
                    trace_result.listNeuron.push_back(trace_result_p.listNeuron[i]);
                }
            }
        }
        double min_count = 1000000000;
        int ind_count;
        for(V3DLONG i=0;i<count_v.size();i++)
        {
            if(min_count>count_v[i])
            {
                min_count = count_v[i];
                ind_count = i;
            }
        }

        if(count_v.size()!=0)
        {
            next_m.x = point_b[ind_count].x;
            next_m.y = point_b[ind_count].y;
            next_m.z = point_b[ind_count].z;
            next_m.color.a = 0;
            next_m.color.b = 0;
            next_m.color.g = 0;
            next_m.color.r = 0;
            next_m.comments = "a";
        }
        cout<<"trace_result_p = "<<trace_result_p.listNeuron.size()<<endl;
        cout<<"trace_result = "<<trace_result.listNeuron.size()<<endl;

        const Image4DSimple *curr = callback.getImageTeraFly();
        NeuronTree curr_win_nt = callback.getSWCTeraFly();
        NeuronTree curr_window_nt = match_area(curr,callback,trace_result,curr_win_nt);
        QList <NeuronSWC> listNeuron;
        QHash <int, int>  hashNeuron;
        listNeuron.clear();
        hashNeuron.clear();
        V3DLONG max_id=0;
        for(int j = 0; j < curr_win_nt.listNeuron.size(); j++)
        {
            listNeuron.append(curr_win_nt.listNeuron.at(j));
            hashNeuron.insert(curr_win_nt.listNeuron.at(j).n, listNeuron.size()-1);
        }
        for(V3DLONG i=0;i<curr_win_nt.listNeuron.size();i++)
        {
            if(curr_win_nt.listNeuron[i].n>max_id)
            {
                max_id = curr_win_nt.listNeuron[i].n;
            }
        }
//        resultTree.listNeuron.clear();
//        resultTree.hashNeuron.clear();

        resultTree.listNeuron = listNeuron;
        resultTree.hashNeuron = hashNeuron;
        resultTree.color.r = 0;
        resultTree.color.g = 0;
        resultTree.color.b = 0;
        resultTree.color.a = 0;
        if(change)
        {
            resultTree_rebase.listNeuron = listNeuron;
            resultTree_rebase.hashNeuron = hashNeuron;
            resultTree_rebase.color.r = 0;
            resultTree_rebase.color.g = 0;
            resultTree_rebase.color.b = 0;
            resultTree_rebase.color.a = 0;
            QString outname = "resulttree_rebase.swc";
            writeSWC_file(outname,resultTree_rebase);
        }
        change = true;
        for(V3DLONG i=0;i<curr_window_nt.listNeuron.size();i++)
        {
            curr_window_nt.listNeuron[i].type = 10;
            curr_window_nt.listNeuron[i].n = curr_window_nt.listNeuron[i].n + max_id;
            if(curr_window_nt.listNeuron[i].pn!=-1)
            {
                curr_window_nt.listNeuron[i].pn = curr_window_nt.listNeuron[i].pn + max_id;
            }
            resultTree.listNeuron.push_back(curr_window_nt.listNeuron[i]);
        }
      //  NeuronTree result_output;
       // SortSWC(resultTree.listNeuron,result_output.listNeuron,resultTree.listNeuron[0].n,1000000);
        callback.setSWCTeraFly(resultTree);
        QString final_name = "result.swc";
        writeSWC_file(final_name,resultTree);







        /******************************next marker*************************/


        //next_m = next_marker(callback,curr_window_nt);


    }
    else if (menu_name == tr("trace_NEUTUBE"))  //finished
    {

        v3d_msg("success");
        TRACE_LS_PARA P;
        bool bmenu = false;
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
        }
        P.method = neutube;
        func_name = neutube;
        if(!grid_raw_all(callback,parent,P,bmenu))return;

        QString name = P.inimg_file+"_neutube.swc";
        trace_result = readSWC_file(name);



        if(trace_result.listNeuron.size()!=0)
        {
            marker_rebase = marker_rebase2;
        }
        else
        {
            v3d_msg("this tracing has no result");
        }
        for(V3DLONG i=0;i<trace_result.listNeuron.size();i++)
        {
            trace_result.listNeuron[i].x = trace_result.listNeuron[i].x*P.ratio_x + P.o_x;
            trace_result.listNeuron[i].y = trace_result.listNeuron[i].y*P.ratio_y + P.o_y;
            trace_result.listNeuron[i].z = trace_result.listNeuron[i].z*P.ratio_z + P.o_z;
        }

        const Image4DSimple *curr = callback.getImageTeraFly();
        NeuronTree curr_win_nt = callback.getSWCTeraFly();
        NeuronTree curr_window_nt = match_area(curr,callback,trace_result,curr_win_nt);
        //NeuronTree resultTree;
        QList <NeuronSWC> listNeuron;
        QHash <int, int>  hashNeuron;
        listNeuron.clear();
        hashNeuron.clear();
        V3DLONG max_id=0;
        for(int j = 0; j < curr_win_nt.listNeuron.size(); j++)
        {
            listNeuron.append(curr_win_nt.listNeuron.at(j));
            hashNeuron.insert(curr_win_nt.listNeuron.at(j).n, listNeuron.size()-1);
        }
        for(V3DLONG i=0;i<curr_win_nt.listNeuron.size();i++)
        {
            if(curr_win_nt.listNeuron[i].n>max_id)
            {
                max_id = curr_win_nt.listNeuron[i].n;
            }
        }
        resultTree.listNeuron.clear();
        resultTree.hashNeuron.clear();
        resultTree.listNeuron = listNeuron;
        resultTree.hashNeuron = hashNeuron;
        resultTree.color.r = 0;
        resultTree.color.g = 0;
        resultTree.color.b = 0;
        resultTree.color.a = 0;
        if(change)
        {
          //  v3d_msg("enter into resultTree_rebase");
            resultTree_rebase.listNeuron = listNeuron;
            resultTree_rebase.hashNeuron = hashNeuron;
            resultTree_rebase.color.r = 0;
            resultTree_rebase.color.g = 0;
            resultTree_rebase.color.b = 0;
            resultTree_rebase.color.a = 0;
            //cout<<"resultTree_rebase.listNeuron.size = "<<resultTree_rebase.listNeuron.size()<<endl;
        }
        change = true;
        for(V3DLONG i=0;i<curr_window_nt.listNeuron.size();i++)
        {
            curr_window_nt.listNeuron[i].type = 10;
            curr_window_nt.listNeuron[i].n = curr_window_nt.listNeuron[i].n + max_id;
            if(curr_window_nt.listNeuron[i].pn!=-1)
            {
                curr_window_nt.listNeuron[i].pn = curr_window_nt.listNeuron[i].pn + max_id;
            }
            resultTree.listNeuron.push_back(curr_window_nt.listNeuron[i]);
        }

        callback.setSWCTeraFly(resultTree);
        QString final_name = "result.swc";
        writeSWC_file(final_name,resultTree);



    }
    else if(menu_name == tr("move_to_another_block_with_marker"))
    {

        resultTree_rebase = callback.getSWCTeraFly();

        cout<<"next_m ="<<next_m.x<<"  "<<next_m.y<<"  "<<next_m.z<<"  "<<next_m.comments<<endl;
        //v3d_msg("check next_m");
        if(next_m.x == 0&&next_m.y == 0&&next_m.z == 0)
        {
            //v3d_msg("use landmark");
            LocationSimple next;
            next_landmarker(callback,next);
            //mean_shift_marker(callback,next_m,next);
            LandmarkList all_marker;
            for(V3DLONG i=0;i<marker_rebase3.size();i++)
            {
                all_marker.push_back(marker_rebase3[i]);
            }
            all_marker.push_back(next);
            callback.setLandmarkTeraFly(all_marker);
            cout<<"next = "<<next.x<<"  "<<next.y<<"  "<<next.z<<endl;

            double dif = (next.x - next_m_rebase.x)*(next.x - next_m_rebase.x) +(next.y - next_m_rebase.y)*(next.y - next_m_rebase.y)+(next.z - next_m_rebase.z)*(next.z - next_m_rebase.z);
            if(dif<0.001)
            {
                v3d_msg("the same next marker");

                return;
            }
            else
            {

                //v3d_msg("about to move");
                callback.setImageTeraFly(next.x,next.y,next.z);
                next_m_rebase.x = next.x;
                next_m_rebase.y = next.y;
                next_m_rebase.z = next.z;
            }
        }
        else
        {
            LocationSimple next;
            mean_shift_marker(callback,next_m,next);
            cout<<"next = "<<next.x<<"  "<<next.y<<"  "<<next.z<<endl;
            LandmarkList all_marker;
            for(V3DLONG i=0;i<marker_rebase3.size();i++)
            {
                all_marker.push_back(marker_rebase3[i]);
            }
            all_marker.push_back(next);
            callback.setLandmarkTeraFly(all_marker);

            double dif = (next.x - next_m_rebase.x)*(next.x - next_m_rebase.x) +(next.y - next_m_rebase.y)*(next.y - next_m_rebase.y)+(next.z - next_m_rebase.z)*(next.z - next_m_rebase.z);
            if(dif<0.001)
            {
                v3d_msg("the same next marker");

                return;
            }
            else
            {
                //v3d_msg("about to move");
                callback.setImageTeraFly(next.x,next.y,next.z);
                next_m_rebase.x = next.x;
                next_m_rebase.y = next.y;
                next_m_rebase.z = next.z;
            }
        }


    }
    else if(menu_name == tr("set_thresh"))
    {
        bool miok;
        thresh = QInputDialog::getInt(0,"Intensity Threshold 1%-99%","please input your number",40,1,200,5,&miok);
        if(miok)
        {
            cout<<"input number is "<<thresh<<endl;
        }
    }
    else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by LXF, 2018-5-21"));
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
            cout<<"Need input image"<<endl;
            return false;
        }

        P.inimg_file = infiles[0];
        P.image = 0;
        int k=0;

        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        if(inmarker_file.isEmpty())
        {
            cout<<"Need a marker file"<<endl;
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

lookPanel::lookPanel(V3DPluginCallback2 &callback, QWidget *parent) :
    QDialog(parent),callback(callback)
{

    gridLayout = new QGridLayout();
    QPushButton* set_thresh     = new QPushButton("                  Set Thresh                  ");
    QPushButton* blank1     = new QPushButton("                                              ");
    QPushButton* blank2     = new QPushButton("                                              ");
   // QPushButton* move_block     = new QPushButton("move_block");
    QPushButton* use_landmark     = new QPushButton("                use_landmark                ");
    gridLayout->addWidget(set_thresh, 0,0);
    gridLayout->addWidget(set_thresh, 1,0);
    //gridLayout->addWidget(blank2, 2,0);
    gridLayout->addWidget(use_landmark, 2,0);
    //gridLayout->addWidget(move_block, 2,0);

    setLayout(gridLayout);
    setWindowTitle(QString("Synchronize annotation "));
    connect(set_thresh,     SIGNAL(clicked()), this, SLOT(_slot_set_thresh()));
    //connect(move_block,     SIGNAL(clicked()), this, SLOT(_slot_move_block()));
    connect(use_landmark,     SIGNAL(clicked()), this, SLOT(_slot_use_landmarker()));


}
NeuronTree match_area(const Image4DSimple* curr,V3DPluginCallback2 &m_v3d,NeuronTree &trace_result,NeuronTree &curr_win_nt)
{
    //v3d_msg("this is match_area");
    NeuronTree curr_swc,updated_nt,result;
    double ox = curr->getOriginX();
    double oy = curr->getOriginY();
    double oz = curr->getOriginZ();
    double lx = curr->getRezX();
    double ly = curr->getRezY();
    double lz = curr->getRezZ();

    if(curr_win_nt.listNeuron.size()>0)
    {
        for(V3DLONG i=0;i<curr_win_nt.listNeuron.size();i++)
        {
            NeuronSWC s = curr_win_nt.listNeuron[i];
            if(s.x<ox+lx&&s.y<oy+ly&&s.z<oz+lz&&s.x>ox&&s.y>oy&&s.z>oz)
            {
                curr_swc.listNeuron.push_back(s);
            }
            else
            {
                updated_nt.listNeuron.push_back(s);
            }
        }
        QString name = "lllll.swc";
        writeSWC_file(name,curr_swc);
        //cout<<"trace = "<<trace_result.listNeuron.size()<<endl;

        for(V3DLONG i=0;i<trace_result.listNeuron.size();i++)
        {
            double dis=0;
            double min_dis=100000000000;
            for(V3DLONG j=0;j<curr_swc.listNeuron.size();j++)
            {
                dis = NTDIS(trace_result.listNeuron[i],curr_swc.listNeuron[j]);
                if(dis<min_dis)
                {
                    min_dis = dis;
                }

            }

            cout<<"min______________________dis = "<<min_dis<<endl;

            double para = curr->getRezX()/curr->getXDim();
            cout<<"min_dis/para = "<<min_dis/para<<endl;
            vector<int> count_v;
            vector<NeuronSWC> point_b;
            if(min_dis/para>10)  //rebase 15
            {

                if(point_at_boundry(m_v3d,trace_result.listNeuron[i],count_v,point_b))
                {
                    //trace_result.listNeuron[i].type = 10;
                    result.listNeuron.push_back(trace_result.listNeuron[i]);
                }
            }


        }
    }
    else
    {
        result = trace_result;
    }
    //v3d_msg("match_area done");
    return result;
}

lookPanel::~lookPanel()
{
}

void lookPanel::_slot_set_thresh()
{
    cout<<"this is slot set thresh"<<endl;

    bool miok;
    thresh = QInputDialog::getInt(0,"Intensity Threshold 1%-99%","please input your number",42,1,200,5,&miok);
    if(miok)
    {
        cout<<"input number is "<<thresh<<endl;
    }


}
void lookPanel::_slot_move_block()
{
    cout<<"this is slot move block"<<endl;
    cout<<"next_m = "<<next_m.x<<"  "<<next_m.y<<"  "<<next_m.z<<endl;
    LandmarkList all_marker;
    //all_marker = callback.getLandmarkTeraFly();
    for(V3DLONG i=0;i<marker_rebase3.size();i++)
    {
        all_marker.push_back(marker_rebase3[i]);
    }
    all_marker.push_back(next_m);
    v3d_msg("lllllllllll");
    callback.setLandmarkTeraFly(all_marker);
    v3d_msg("move_to_another_block_with_marker");

    cout<<next_m.x<<"  "<<next_m.y<<"  "<<next_m.z<<endl;
    double dif = (next_m.x - next_m_rebase.x)*(next_m.x - next_m_rebase.x) +(next_m.y - next_m_rebase.y)*(next_m.y - next_m_rebase.y)+(next_m.z - next_m_rebase.z)*(next_m.z - next_m_rebase.z);
    if(dif<0.001)
    {
        v3d_msg("the same next marker");
        return;
    }
    else
    {
        v3d_msg("about to move");
        callback.setImageTeraFly(next_m.x,next_m.y,next_m.z);
        next_m_rebase.x = next_m.x;
        next_m_rebase.y = next_m.y;
        next_m_rebase.z = next_m.z;
    }
}
void lookPanel::_slot_use_landmarker()
{
        next_m.x = 0;
        next_m.y = 0;
        next_m.z = 0;

}
bool point_at_boundry(V3DPluginCallback2 &callback,NeuronSWC &s,vector<int> &count_v,vector<NeuronSWC> &point_b)
{
    const Image4DSimple *curr = callback.getImageTeraFly();
    double para_ratio = curr->getRezX()/curr->getXDim();
    double ox = curr->getOriginX();
    double oy = curr->getOriginY();
    double oz = curr->getOriginZ();
    double lx = curr->getRezX();
    double ly = curr->getRezY();
    double lz = curr->getRezZ();
    double min_dis = 1000000000000;
    double dis_x1 = (s.x-ox)/para_ratio;
    if(dis_x1<min_dis)min_dis = dis_x1;
    double dis_x2 = (ox+lx-s.x)/para_ratio;
    if(dis_x2<min_dis)min_dis = dis_x2;
    double dis_y1 = (s.y-oy)/para_ratio;
    if(dis_y1<min_dis)min_dis = dis_y1;
    double dis_y2 = (oy+ly-s.y)/para_ratio;
    if(dis_y2<min_dis)min_dis = dis_y2;
    double dis_z1 = (s.z-oz)/para_ratio;
    if(dis_z1<min_dis)min_dis = dis_z1;
    double dis_z2 = (oz+lz-s.z)/para_ratio;
    if(dis_z2<min_dis)min_dis = dis_z2;

    cout<<"min_dis = "<<min_dis<<endl;
    //v3d_msg("check inside2");

    if(min_dis<6)    //5
    {
        cout<<"          1         "<<endl;
        if(resultTree.listNeuron.size()==0)
        {
            next_m.x = s.x;
            next_m.y = s.y;
            next_m.z = s.z;
            next_m.color.a = 0;
            next_m.color.b = 0;
            next_m.color.g = 0;
            next_m.color.r = 0;
        }
        else
        {
            point_b.push_back(s);
            int b_size = 30;     //thresh for avoid marker move back
            int count = 0;
            for(V3DLONG i=0;i<resultTree.listNeuron.size();i++)   //need to modify
            {
                NeuronSWC curr_nt = resultTree.listNeuron[i];

                if(curr_nt.x<s.x+b_size&&curr_nt.x>s.x-b_size&&curr_nt.y<s.y+b_size&&curr_nt.y>s.y-b_size&&curr_nt.z<s.z+b_size&&curr_nt.z>s.z-b_size)
                {
                    count++;
                }
            }
            count_v.push_back(count);
        }

        return false;
    }
    else
    {
        return true;
    }






}
bool next_landmarker(V3DPluginCallback2 &callback,LocationSimple &next)
{
    //v3d_msg("get next landmarker");
    V3DLONG data1d_sz[4];
    const Image4DSimple *curr_block = callback.getImageTeraFly();
    LandmarkList terafly_landmarks_terafly = callback.getLandmarkTeraFly();
    LandmarkList new_marker;
    LocationSimple t;
    double ox = curr_block->getOriginX();
    double oy = curr_block->getOriginY();
    double oz = curr_block->getOriginZ();
    double lx = curr_block->getRezX();
    double ly = curr_block->getRezY();
    double lz = curr_block->getRezZ();


    if(terafly_landmarks_terafly.isEmpty())return false;
    LandmarkList terafly_landmarks;
    LandmarkList other_marker;
    cout<<"terafly_landmarks_terafly.size() = "<<terafly_landmarks_terafly.size()<<endl;
    //v3d_msg("check size");
    for(V3DLONG i=0;i<terafly_landmarks_terafly.size();i++)
    {
        //LocationSimple n;
        LocationSimple s = terafly_landmarks_terafly[i];
        if(s.comments == "a")
        {
            next = s;return true;
        }
        if(s.x<ox+lx&&s.y<oy+ly&&s.z<oz+lz&&s.x>ox&&s.y>oy&&s.z>oz)
        {
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
        next = t;
}
LocationSimple next_marker(V3DPluginCallback2 &callback,NeuronTree &trace_result_part)
{
    LocationSimple point;
    const Image4DSimple *curr = callback.getImageTeraFly();
    double ox = curr->getOriginX();
    double oy = curr->getOriginY();
    double oz = curr->getOriginZ();
    double lx = curr->getRezX();
    double ly = curr->getRezY();
    double lz = curr->getRezZ();
    vector<double> min_dis_v;
    for(V3DLONG i=0;i<trace_result_part.listNeuron.size();i++)
    {
        double min_dis = 1000000000000;
        NeuronSWC s = trace_result_part.listNeuron[i];
        double dis_x1 = s.x-ox;
        if(dis_x1<min_dis)min_dis = dis_x1;
        double dis_x2 = ox+lx-s.x;
        if(dis_x2<min_dis)min_dis = dis_x2;
        double dis_y1 = s.y-oy;
        if(dis_y1<min_dis)min_dis = dis_y1;
        double dis_y2 = oy+ly-s.y;
        if(dis_y2<min_dis)min_dis = dis_y2;
        double dis_z1 = s.z-oz;
        if(dis_z1<min_dis)min_dis = dis_z1;
        double dis_z2 = oz+lz-s.z;
        if(dis_z2<min_dis)min_dis = dis_z2;
        min_dis_v.push_back(min_dis);
    }


//    sort_bubble(min_dis_v);
//    for(V3DLONG i=0;i<min_dis_v.size();i++)
//    {
//        cout<<min_dis_v[i]<<endl;
//    }
//    point.x = trace_result_part.listNeuron[1].x;
//    point.y = trace_result_part.listNeuron[1].y;
//    point.z = trace_result_part.listNeuron[1].z;




    double min_dis_in_v=100000000000;
    int ind;
    cout<<"oooooooooooo"<<min_dis_v.size()<<endl;
    for(V3DLONG i=0;i<min_dis_v.size();i++)
    {
        if(min_dis_v[i]<min_dis_in_v)
        {
            min_dis_in_v = min_dis_v[i];
            ind = i;
        }
    }
//    min_dis_v.erase(min_dis_v.begin()+ind-1);
//    min_dis_in_v=100000000000;
//    //ind = 0;
//    cout<<"zzzzzzzzzzzzz"<<min_dis_v.size()<<endl;
//    for(V3DLONG i=0;i<min_dis_v.size();i++)
//    {
//        if(min_dis_v[i]<min_dis_in_v)
//        {
//            min_dis_in_v = min_dis_v[i];
//            ind = i;
//        }
//    }
//    v3d_msg("check inside2");
    point.x = trace_result_part.listNeuron[ind].x;
    point.y = trace_result_part.listNeuron[ind].y;
    point.z = trace_result_part.listNeuron[ind].z;

    return point;

}
void sort_bubble(vector<double> &min_dis_v)
{
    int len = min_dis_v.size();
    double tmp;
    for(V3DLONG i=0;i<len-1;i++)
    {
        for(V3DLONG j=0;j<len-i-1;j++)
        {
            if(min_dis_v[j]>min_dis_v[j+1])
            {
                tmp = min_dis_v[j];
                min_dis_v[j] = min_dis_v[j+1];
                min_dis_v[j+1] = tmp;
            }
        }
    }
}

bool mean_shift_marker(V3DPluginCallback2 &callback,LocationSimple &next_m,LocationSimple &next)
{

    const Image4DSimple *curr = callback.getImageTeraFly();
    double para_ratio_x = curr->getRezX()/curr->getXDim();
    double para_ratio_y = curr->getRezY()/curr->getYDim();
    double para_ratio_z = curr->getRezZ()/curr->getZDim();
    double ox = curr->getOriginX();
    double oy = curr->getOriginY();
    double oz = curr->getOriginZ();
    double lx = curr->getRezX();
    double ly = curr->getRezY();
    double lz = curr->getRezZ();

    NeuronSWC s;
    s.x = (next_m.x-ox)/para_ratio_x;
    s.y = (next_m.y-oy)/para_ratio_y;
    s.z = (next_m.z-oz)/para_ratio_z;



    V3DLONG im_cropped_sz[4];


    unsigned char * datald = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;
    int datatype;
    if (!loadRawRegion(const_cast<char *>(outimg_file.toStdString().c_str()), datald, in_zz, in_sz,datatype,0,0,0,1,1,1))
    {
        return false;
    }

    V3DLONG N = in_zz[0];
    V3DLONG M = in_zz[1];
    V3DLONG P = in_zz[2];
    unsigned char * im_cropped = 0;
    V3DLONG pagesz;
    double para = 4;
    double l_x = para;
    double l_y = para;
    double l_z = para;



    V3DLONG xb = s.x-l_x;
    V3DLONG xe = s.x+l_x-1;
    V3DLONG yb = s.y-l_y;
    V3DLONG ye = s.y+l_y-1;
    V3DLONG zb = s.z-l_z;
    V3DLONG ze = s.z+l_z-1;
    if(xb<0) xb = 0;
    if(xe>=N-1) xe = N-1;
    if(yb<0) yb = 0;
    if(ye>=M-1) ye = M-1;
    if(zb<0) zb = 0;
    if(ze>=N-1) ze = P-1;
 //   cout<<"begin = "<<xb<<"  "<<yb<<"  "<<zb<<endl;
//    cout<<"end   = "<<xe<<"  "<<ye<<"  "<<ze<<endl;
//    v3d_msg("check!");
    pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
    im_cropped_sz[0] = xe-xb+1;
    im_cropped_sz[1] = ye-yb+1;
    im_cropped_sz[2] = ze-zb+1;
    im_cropped_sz[3] = 1;


    try {im_cropped = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
    //v3d_msg("kkkkkkkkkkkkkk");
     vector<relationship> relation;
     V3DLONG j = 0;
     for(V3DLONG iz = zb; iz <= ze; iz++)
     {
         V3DLONG offsetk = iz*M*N;

         for(V3DLONG iy = yb; iy <= ye; iy++)
         {
             V3DLONG offsetj = iy*N;
             for(V3DLONG ix = xb; ix <= xe; ix++)
             {

                  im_cropped[j] = datald[offsetk + offsetj + ix];
                  relationship relate;
                  relate.datald = im_cropped[j];
                  relate.x = ix;
                  relate.y = iy;
                  relate.z = iz;
                  relation.push_back(relate);
                  j++;
             }
         }
     }
//     cout<<"j = "<<j<<endl;
 //    v3d_msg("check j!");
     double max_datald=-1;
     int ind;
     for(V3DLONG i=0;i<j;i++)
     {
         if(im_cropped[i]>max_datald)
         {
            max_datald = im_cropped[i];
            ind = i;
         }
     }
     LocationSimple tmp;
     for(V3DLONG i=0;i<relation.size();i++)
     {
         //cout<<"max_datald = "<<max_datald<<endl;
         //cout<<"relation[i].datald = "<<relation[i].datald<<endl;
         if(max_datald == relation[i].datald)
         {
//             cout<<"tmp.x = "<<tmp.x<<endl;
//             cout<<"tmp.y = "<<tmp.y<<endl;
//             cout<<"tmp.z = "<<tmp.z<<endl;
             tmp.x = relation[i].x;
             tmp.y = relation[i].y;
             tmp.z = relation[i].z;
         }
     }
     next.x = tmp.x*para_ratio_x+ox;
     next.y = tmp.y*para_ratio_x+oy;
     next.z = tmp.z*para_ratio_x+oz;
     next.color.r = 0;
     next.color.g = 0;
     next.color.b = 0;
     next.color.a = 0;
     next.comments = "a";


           if(datald) {delete []datald; datald = 0;}

          if(im_cropped) {delete []im_cropped; im_cropped = 0;}


}
