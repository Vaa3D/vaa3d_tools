/* neurontracing_region_app2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-01-16 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracing_region_neuron2_plugin.h"
#include "stackutil.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../v3d_main/jba/c++/convert_type2uint8.h"



using namespace std;
Q_EXPORT_PLUGIN2(neurontracing_region_neuron2, neurontracing_region_neuron2);

struct BASIC_LS_PARA
{
    int bkg_thresh;
    int region_number;

    Image4DSimple * image;

    QString inimg_file;
};

struct NC_LS_PARA
{
    int bkgd_thresh;
    int region_num;
    int channel;
    int scale;
    double corr_thresh;
    int nr_dirs;
    double angular_sigma;
    int nr_iters;
    int nr_states;
    double z_dist;
    int save_midres;

    Image4DSimple * image;

    QString inimg_file;
};

struct MST_LS_PARA
{
    int bkg_thresh;
    int region_number;
    int window_size;

    Image4DSimple * image;

    QString inimg_file;
};

struct ADVANTRA_LS_PARA
{
    int scale;
    double background_ratio;
    double  correlation_thresh;
    int nr_directions;
    double angular_sigma;
    int nr_iters;
    int nr_states;
    double z_layer_dist_pix;
    int bkgd_thresh;
    int region_num;

    Image4DSimple * image;

    QString inimg_file;
};

struct APP2_LS_PARA
{
    int is_gsdt;
    int is_break_accept;
    int  bkg_thresh;
    double length_thresh;
    int  cnn_type;
    int  channel;
    double SR_ratio;
    int  b_256cube;
    int b_RadiusFrom2D;
    int root_1st[3];
    int region_number;

    Image4DSimple * image;

    QString inimg_file;
};

struct APP1_LS_PARA
{
    int  bkg_thresh;
    int  channel;
    int  b_256cube;
    int region_number;

    Image4DSimple * image;

    QString inimg_file;
};

void autotrace_region_app1(V3DPluginCallback2 &callback, QWidget *parent,APP1_LS_PARA &p,bool bmenu);
void autotrace_region(V3DPluginCallback2 &callback, QWidget *parent,void * Para,bool bmenu,const QString trace);

template <class T> void app2_tracing(const T* image_region,
                                     unsigned char * data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     APP2_LS_PARA &Para,
                                     bool bmenu);


QString getAppPath();

template <class T> void advantra_tracing(const T* image_region,
                                     unsigned char * data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     ADVANTRA_LS_PARA &Para,
                                     bool bmenu);


QString getAppPath();

template <class T> void ent_tracing(const T* image_region,
                                     unsigned char * data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     BASIC_LS_PARA &Para,
                                     bool bmenu);


QString getAppPath();

template <class T> void gd_tracing(const T* image_region,
                                     unsigned char * data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     BASIC_LS_PARA &Para,
                                     bool bmenu);


QString getAppPath();

template <class T> void mst_tracing(const T* image_region,
                                     unsigned char * data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     MST_LS_PARA &Para,
                                     bool bmenu);


QString getAppPath();

template <class T> void nc_tracing(const T* image_region,
                                     unsigned char * data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     NC_LS_PARA &Para,
                                     bool bmenu);


QString getAppPath();


 
QStringList neurontracing_region_neuron2::menulist() const
{
    return QStringList()
        <<tr("trace_advantra")
        <<tr("trace_app1")
        <<tr("trace_app2")
        <<tr("trace_ent")
        <<tr("trace_gd")
        <<tr("trace_mst")
        <<tr("trace_neuronchaser")
		<<tr("about");
}

QStringList neurontracing_region_neuron2::funclist() const
{
	return QStringList()
        <<tr("trace")
		<<tr("help");
}

void neurontracing_region_neuron2::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("trace_app1"))
    {
        APP1_LS_PARA P;
        bool bmenu = true;
        regionAPP1Dialog dialog(callback, parent);
        if (!dialog.image)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        P.inimg_file = dialog.image->getFileName();
        P.bkg_thresh = dialog.bkg_thresh;
        P.channel = dialog.channel;
        P.b_256cube = dialog.b_256cube;
        P.region_number = dialog.region_number;

        autotrace_region_app1(callback,parent,P,bmenu);

    }
    else if (menu_name == tr("trace_app2"))
	{
        APP2_LS_PARA P;
        bool bmenu = true;
        regionAPP2Dialog dialog(callback, parent);
        if (!dialog.image)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;
        P.inimg_file = dialog.image->getFileName();

        P.is_gsdt = dialog.is_gsdt;
        P.is_break_accept = dialog.is_break_accept;
        P.bkg_thresh = dialog.bkg_thresh;
        P.length_thresh = dialog.length_thresh;
        P.cnn_type = dialog.cnn_type;
        P.channel = dialog.channel;
        P.SR_ratio = dialog.SR_ratio;
        P.b_256cube = dialog.b_256cube;
        P.b_RadiusFrom2D = dialog.b_RadiusFrom2D;
        P.region_number = dialog.region_number;

        //autotrace_region_app2(callback,parent,P,bmenu);
        autotrace_region(callback,parent,&P,bmenu,menu_name);
	}
    else if (menu_name == tr("trace_advantra"))
    {
        ADVANTRA_LS_PARA P;
        bool bmenu = true;
        regionADVANTRADialog dialog(callback, parent);
        if (!dialog.image)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;
        P.inimg_file = dialog.image->getFileName();

        P.scale = dialog.scale;
        P.background_ratio = dialog.bkg_ratio;
        P.correlation_thresh = dialog.correlation_thresh;
        P.nr_directions = dialog.nr_dirs;
        P.angular_sigma = dialog.angular_sigma;
        P.nr_iters = dialog.nr_iters;
        P.nr_states = dialog.nr_states;
        P.z_layer_dist_pix = dialog.z_dist;
        P.bkgd_thresh = dialog.bkgd_thresh;
        P.region_num = dialog.region_num;

        //autotrace_region_advantra(callback,parent,P,bmenu);
        autotrace_region(callback,parent,&P,bmenu,menu_name);
    }
    else if (menu_name == tr("trace_ent"))
    {
        BASIC_LS_PARA P;
        bool bmenu = true;
        regionBasicDialog dialog(callback, parent);
        if (!dialog.image)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;
        P.inimg_file = dialog.image->getFileName();
        P.bkg_thresh = dialog.bkgd_thresh;
        P.region_number = dialog.region_num;

        //autotrace_region_basic(callback,parent,P,bmenu,"ent");
        //autotrace_region(callback,parent,&P,bmenu,P.inimg_file,P.bkg_thresh,P.region_number,4);
        autotrace_region(callback,parent,&P,bmenu,menu_name);
    }
    else if (menu_name == tr("trace_gd"))
    {
        BASIC_LS_PARA P;
        bool bmenu = true;
        regionBasicDialog dialog(callback, parent);
        if (!dialog.image)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;
        P.inimg_file = dialog.image->getFileName();
        P.bkg_thresh = dialog.bkgd_thresh;
        P.region_number = dialog.region_num;

        //autotrace_region_basic(callback,parent,P,bmenu,"gd");
        //autotrace_region(callback,parent,&P,bmenu,P.inimg_file,P.bkg_thresh,P.region_number,5);
        autotrace_region(callback,parent,&P,bmenu,menu_name);
    }
    else if (menu_name == tr("trace_mst"))
    {
        MST_LS_PARA P;
        bool bmenu = true;
        regionMSTDialog dialog(callback, parent);
        if (!dialog.image)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;
        P.inimg_file = dialog.image->getFileName();
        P.bkg_thresh = dialog.bkgd_thresh;
        P.region_number = dialog.region_num;
        P.window_size = dialog.window_sz;

        //autotrace_region_mst(callback,parent,P,bmenu);
        //autotrace_region(callback,parent,&P,bmenu,P.inimg_file,P.bkg_thresh,P.region_number,6);
        autotrace_region(callback,parent,&P,bmenu,menu_name);
    }
    else if (menu_name == tr("trace_neuronchaser"))
    {
        NC_LS_PARA P;
        bool bmenu = true;
        regionNCDialog dialog(callback, parent);
        if (!dialog.image)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;
        P.inimg_file = dialog.image->getFileName();
        P.bkgd_thresh = dialog.bkgd_thresh;
        P.region_num = dialog.region_num;
        P.channel = dialog.channel;
        P.scale = dialog.scale;
        P.corr_thresh = dialog.corr_thresh;
        P.nr_dirs = dialog.nr_dirs;
        P.angular_sigma = dialog.angular_sigma;
        P.nr_iters = dialog.nr_iters;
        P.nr_states = dialog.nr_states;
        P.z_dist = dialog.z_dist;
        P.save_midres = dialog.save_midres;

        //autotrace_region_nc(callback,parent,P,bmenu);
        //autotrace_region(callback,parent,P,bmenu,P.inimg_file,P.bkgd_thresh,P.region_num,7);
        autotrace_region(callback,parent,&P,bmenu,menu_name);
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2015-01-16"));
	}
}

bool neurontracing_region_neuron2::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("trace_app1"))
    {
        APP1_LS_PARA P;
        bool bmenu = false;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        P.inimg_file = infiles[0];
        int k=0;
         //try to use as much as the default value in the PARA_APP1 constructor as possible
        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 10; k++;
        P.b_256cube = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.region_number = (paras.size() >= k+1) ? atoi(paras[k]) : 10000; k++;

        autotrace_region_app1(callback,parent,P,bmenu);

    }
    else if (func_name == tr("trace_app2"))
	{
        APP2_LS_PARA P;
        bool bmenu = false;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        P.inimg_file = infiles[0];
        int k=0;
         //try to use as much as the default value in the PARA_APP2 constructor as possible
        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 10; k++;
        P.b_256cube = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.is_gsdt = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.is_break_accept = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.length_thresh = (paras.size() >= k+1) ? atof(paras[k]) : 5; k++;
        P.region_number = (paras.size() >= k+1) ? atoi(paras[k]) : 10000; k++;

        P.cnn_type = 2;
        P.SR_ratio = 3.0/9.0;
        P.b_RadiusFrom2D = 1;

        //autotrace_region_app2(callback,parent,P,bmenu);
        autotrace_region(callback,parent,&P,bmenu,func_name);

    }
    else if (func_name == tr("trace_advantra"))
    {
        ADVANTRA_LS_PARA P;
        bool bmenu = false;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        P.inimg_file = infiles[0];
        int k=0;
         //try to use as much as the default value as possible
        P.scale = (paras.size() >= k+1) ? atoi(paras[k]) : 10;  k++;
        P.background_ratio = (paras.size() >= k+1) ? atof(paras[k]) : .5; k++;
        P.correlation_thresh = (paras.size() >= k+1) ? atof(paras[k]) : .75; k++;
        P.nr_directions = (paras.size() >= k+1) ? atoi(paras[k]) : 20; k++;
        P.angular_sigma = (paras.size() >= k+1) ? atof(paras[k]) : 60.0; k++;
        P.nr_iters = (paras.size() >= k+1) ? atoi(paras[k]) : 5; k++;
        P.nr_states = (paras.size() >= k+1) ? atoi(paras[k]) : 5; k++;
        P.z_layer_dist_pix = (paras.size() >= k+1) ? atof(paras[k]) : 1.0; k++;
        P.bkgd_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 55; k++;
        P.region_num = (paras.size() >= k+1) ? atoi(paras[k]) : 10000; k++;

        //autotrace_region_advantra(callback,parent,P,bmenu);
        autotrace_region(callback,parent,&P,bmenu,func_name);

    }
    else if (func_name == tr("trace_ent"))
    {
        BASIC_LS_PARA P;
        bool bmenu = false;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        P.inimg_file = infiles[0];
        int k=0;
         //try to use as much as the default value as possible
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 55;  k++;
        P.region_number = (paras.size() >= k+1) ? atoi(paras[k]) : 10000; k++;

        //autotrace_region_basic(callback,parent,P,bmenu,"ent");
        autotrace_region(callback,parent,&P,bmenu,func_name);

    }
    else if (func_name == tr("trace_gd"))
    {
        BASIC_LS_PARA P;
        bool bmenu = false;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        P.inimg_file = infiles[0];
        int k=0;
         //try to use as much as the default value as possible
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 55;  k++;
        P.region_number = (paras.size() >= k+1) ? atoi(paras[k]) : 10000; k++;

        //autotrace_region_basic(callback,parent,P,bmenu,"gd");
        autotrace_region(callback,parent,&P,bmenu,func_name);

    }
    else if (func_name == tr("trace_mst"))
    {
        MST_LS_PARA P;
        bool bmenu = false;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        P.inimg_file = infiles[0];
        int k=0;
         //try to use as much as the default value as possible
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 55;  k++;
        P.region_number = (paras.size() >= k+1) ? atoi(paras[k]) : 10000; k++;
        P.window_size = (paras.size() >= k+1) ? atoi(paras[k]) : 10; k++;

        //autotrace_region_mst(callback,parent,P,bmenu);
        autotrace_region(callback,parent,&P,bmenu,func_name);

    }
    else if (func_name == tr("trace_neuronchaser"))
    {
        NC_LS_PARA P;
        bool bmenu = false;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        P.inimg_file = infiles[0];
        int k=0;
         //try to use as much as the default value as possible
        P.bkgd_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 55;  k++;
        P.region_num = (paras.size() >= k+1) ? atoi(paras[k]) : 10000; k++;
        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1; k++;
        P.scale = (paras.size() >= k+1) ? atoi(paras[k]) : 12; k++;
        P.corr_thresh = (paras.size() >= k+1) ? atof(paras[k]) : .6; k++;
        P.nr_dirs = (paras.size() >= k+1) ? atoi(paras[k]) : 20; k++;
        P.angular_sigma = (paras.size() >= k+1) ? atof(paras[k]) : 60.0; k++;
        P.nr_iters = (paras.size() >= k+1) ? atoi(paras[k]) : 30; k++;
        P.nr_states = (paras.size() >= k+1) ? atoi(paras[k]) : 5; k++;
        P.z_dist = (paras.size() >= k+1) ? atof(paras[k]) : 1.0; k++;
        P.save_midres = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;

        //autotrace_region_nc(callback,parent,P,bmenu);
        autotrace_region(callback,parent,&P,bmenu,func_name);

    }
	else if (func_name == tr("help"))
	{
        printf("\n**** Usage of Region based Neuron2 tracing ****\n");

        printf("vaa3d -x plugin_name -f trace_neuronchaser -i <inimg_file> -p <bkg_thresh> <region_number> <channel> <scale> <cor_thresh> <nr_dirs> <ang_sig> <nr_it> <nr_states> <z_dist> <save_mid> \n");
        printf("inimg_file       The input image\n");
        printf("bkg_thresh       Default 10\n");
        printf("region_number    Default 10000\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("scale            Scale (5, 20] pix.\n");
        printf("cor_thresh       Correlation threshold [0.5, 1.0).\n");
        printf("nr_dirs          nr. directions [5, 20].\n");
        printf("ang_sig          Angular sigma [1,90] degs.\n");
        printf("nr_it            nr. iterations [2, 50].\n");
        printf("nr_states        nr. states [1, 20].\n");
        printf("z_dist           z layer dist [1, 4] pix.\n");
        printf("save_mid         Save midresults 0-no, 1 -yes.\n");
        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

        printf("vaa3d -x plugin_name -f trace_mst -i <inimg_file> -p <bkg_thresh> <region_number> <window_size> \n");
        printf("inimg_file       The input image\n");
        printf("bkg_thresh       Default 10\n");
        printf("region_number    Default 10000\n");
        printf("window_size      Window size for seed detection. Default 10");
        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

        printf("vaa3d -x plugin_name -f trace_gd -i <inimg_file> -p <bkg_thresh> <region_number> \n");
        printf("inimg_file       The input image\n");
        printf("bkg_thresh       Default 10\n");
        printf("region_number    Default 10000\n");
        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

        printf("vaa3d -x plugin_name -f trace_ent -i <inimg_file> -p <bkg_thresh> <region_number> \n");
        printf("inimg_file       The input image\n");
        printf("bkg_thresh       Default 10\n");
        printf("region_number    Default 10000\n");
        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

        printf("vaa3d -x plugin_name -f trace_advantra -i <inimg_file> -p  <scale> <bkg_ratio> <corr_thresh> <nr_directions> <ang_sig> <nr_iters> <nr_states> <z_dist> <bkgd_thrsh> <region_n> \n");
        printf("inimg_file       The input image\n");
        printf("scale            Scale (5, 20] pix.\n");
        printf("bkg_ration       Background ratio (0, 1].\n");
        printf("corr_thresh      Correlation threshold [0.5, 1.0).\n");
        printf("nr_directions    nr. directions [5, 20].\n");
        printf("ang_sig          Angular sigma [20,90] degs.\n");
        printf("nr_iters         nr. iterations [2, 50].\n");
        printf("nr_states        nr. states [1, 20].\n");
        printf("z_dist           z layer dist [1, 4] pix.\n");
        printf("bkg_thrsh       Default 10\n");
        printf("region_number    Default 10000\n");
        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

        printf("vaa3d -x plugin_name -f trace_app2 -i <inimg_file> -p  <channel> <bkg_thresh> <b_256cube> <is_gsdt> <is_gap> <length_thresh> <region_number>\n");
        printf("inimg_file       Should be 8/16/32bit image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("bkg_thresh       Default 10\n");

        printf("b_256cube        If trace in a auto-downsampled volume (1 for yes, and 0 for no. Default 0.)\n");
        printf("is_gsdt          If use gray-scale distance transform (1 for yes and 0 for no. Default 0.)\n");
        printf("is_gap           If allow gap (1 for yes and 0 for no. Default 0.)\n");
        printf("length_thresh    Default 5\n");
        printf("region_number    Default 10000\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

        printf("vaa3d -x plugin_name -f trace_app1 -i <inimg_file> -p  <channel> <bkg_thresh> <b_256cube> <region_number>\n");
        printf("inimg_file       Should be 8/16/32bit image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("bkg_thresh       Default 10\n");
        printf("b_256cube        If trace in a auto-downsampled volume (1 for yes, and 0 for no. Default 0.)\n");
        printf("region_number    Default 10000\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");


	}
	else return false;

	return true;
}

void autotrace_region_app1(V3DPluginCallback2 &callback, QWidget *parent,APP1_LS_PARA &Para,bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,C;
    QString image_name = Para.inimg_file;

    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        Image4DSimple* p4DImage = callback.getImage(curwin);
        data1d = p4DImage->getRawData();

        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        C = p4DImage->getCDim();
    }
    else
    {
        V3DLONG *im_sz = 0;
        int datatype = 0;
        if (loadImage(const_cast<char *>(image_name.toStdString().c_str()), data1d, im_sz, datatype)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",image_name.toStdString().c_str());
            return;
        }
        N = im_sz[0];
        M = im_sz[1];
        P = im_sz[2];
        C = im_sz[3];

    }

    int th = Para.bkg_thresh;
    QString tmpfolder = QFileInfo(image_name).path()+("/") + QFileInfo(image_name).completeBaseName()+("_tmp");
    system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));
    if(tmpfolder.isEmpty())
    {

        printf("Can not create a tmp folder!\n");
        return;
    }

    V3DLONG pagesz = N*M*P;
    unsigned char *image_binary=0;
    try {image_binary = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for image_binary."); return;}
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if(data1d[i] > th)
            image_binary[i] = 255;
        else
            image_binary[i] = 0;
    }

    V3DLONG in_sz[4];
    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;
    in_sz[3] = 1;

    QString input_image_name = tmpfolder + "/binary.raw";
    simple_saveimage_wrapper(callback, input_image_name.toStdString().c_str(),  (unsigned char *)image_binary, in_sz, V3D_UINT8);

    if(image_binary) {delete []image_binary; image_binary = 0;}

    QString output_image_name = tmpfolder +"/region.raw";

    #if  defined(Q_OS_LINUX)
        QString cmd_region = QString("%1/vaa3d -x regiongrow -f rg -i %2 -o %3 -p 1 0 1 %4").arg(getAppPath().toStdString().c_str()).arg(input_image_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str()).arg(Para.region_number);
        system(qPrintable(cmd_region));
    #elif defined(Q_OS_MAC)
        QString cmd_region = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x regiongrow -f rg -i %2 -o %3 -p 1 0 1 %4").arg(getAppPath().toStdString().c_str()).arg(input_image_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str()).arg(Para.region_number);
        system(qPrintable(cmd_region));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

   unsigned char * image_region = 0;
   int datatype;
   V3DLONG in_zz[4];
   if(!simple_loadimage_wrapper(callback, output_image_name.toStdString().c_str(), image_region, in_zz, datatype))
   {
       v3d_msg("Fail to load image");
       return;
   }

   int groupNum = 0;
   for(V3DLONG i = 0; i < pagesz; i++)
   {
       if(image_region[i] > groupNum)
           groupNum = image_region[i];
   }

   int *groupArray = new int[groupNum];
   int *groupIndex = new int[groupNum];

   for(int i = 0; i < groupNum; i++)
   {
       groupArray[i] = 0;
       groupIndex[i] = i+1;
   }

   for(V3DLONG i = 0; i < pagesz; i++)
   {
       if(image_region[i] > 0)
           groupArray[image_region[i] - 1] += 1;
   }

   int tmp,tmp_index;
   for(V3DLONG i = 0; i < groupNum; i++)
   {
       if (i > 0)
       {
           V3DLONG j = i;
           while(j > 0 && groupArray[j-1]<groupArray[j])
           {
               tmp = groupArray[j];
               groupArray[j] = groupArray[j-1];
               groupArray[j-1] = tmp;

               tmp_index = groupIndex[j];
               groupIndex[j] = groupIndex[j-1];
               groupIndex[j-1] = tmp_index;

               j--;
           }
       }
   }


   int groupmax = groupNum;
   vector<MyMarker*> outswc_final;

   for(int dd = 0; dd < groupmax; dd++)
   {
       unsigned char *image_region_one = new unsigned char [pagesz];
       V3DLONG group_type = groupIndex[dd];


       for(V3DLONG i = 0; i < pagesz*datatype; i++)
       {

           if(image_region[i] == group_type)
               image_region_one[int(i/datatype)] = data1d[int(i/datatype)];
           else
               image_region_one[int(i/datatype)] = 0;
       }


       QString APP1_image_name = tmpfolder + "/group_one.raw";
       simple_saveimage_wrapper(callback, APP1_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
       if(image_region_one) {delete []image_region_one; image_region_one = 0;}

       QString APP1_swc =  APP1_image_name + QString("_group_%1.swc").arg(group_type);


        #if  defined(Q_OS_LINUX)
            QString cmd_APP1 = QString("%1/vaa3d -x Vaa3D_Neuron2 -f app1 -i %2 -o %3 -p NULL %4 %5 %6").arg(getAppPath().toStdString().c_str()).arg(APP1_image_name.toStdString().c_str()).arg(APP1_swc.toStdString().c_str())
                    .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube);
            system(qPrintable(cmd_APP1));
        #elif defined(Q_OS_MAC)
            QString cmd_APP1 = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x Vaa3D_Neuron2 -f app1 -i %2 -o %3 -p NULL %4 %5 %6").arg(getAppPath().toStdString().c_str()).arg(APP1_image_name.toStdString().c_str()).arg(APP1_swc.toStdString().c_str())
                    .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube);
            system(qPrintable(cmd_APP1));
        #else
                 v3d_msg("The OS is not Linux or Mac. Do nothing.");
                 return;
        #endif

       vector<MyMarker*> temp_out_swc = readSWC_file(APP1_swc.toStdString());

       for(V3DLONG d = 0; d <temp_out_swc.size(); d++)
       {
           outswc_final.push_back(temp_out_swc[d]);

       }

    }

    if(image_region) {delete []image_region; image_region = 0;}
    if(~bmenu)
        if(data1d) {delete []data1d; data1d = 0;}


    QString final_swc = image_name + "_region_APP1.swc";
    saveSWC_file(final_swc.toStdString(), outswc_final);

    system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

   return;
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

template <class T> void app2_tracing(const T* image_region,
                                     unsigned char* data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     void * para,
                                     bool bmenu)
{
    APP2_LS_PARA Para = *(APP2_LS_PARA *)para;
    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    int groupNum = 0;
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > groupNum)
            groupNum = image_region[i];
    }

    int *groupArray = new int[groupNum];
    int *groupIndex = new int[groupNum];

    for(int i = 0; i < groupNum; i++)
    {
        groupArray[i] = 0;
        groupIndex[i] = i+1;
    }

    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > 0)
            groupArray[(unsigned short int)image_region[i] - 1] += 1;
    }

    int tmp,tmp_index;
    for(V3DLONG i = 0; i < groupNum; i++)
    {
        if (i > 0)
        {
            V3DLONG j = i;
            while(j > 0 && groupArray[j-1]<groupArray[j])
            {
                tmp = groupArray[j];
                groupArray[j] = groupArray[j-1];
                groupArray[j-1] = tmp;

                tmp_index = groupIndex[j];
                groupIndex[j] = groupIndex[j-1];
                groupIndex[j-1] = tmp_index;

                j--;
            }
        }
    }

    int groupmax = 50;
    if(groupNum <= groupmax) groupmax = groupNum;

    vector<MyMarker*> outswc_final;

    for(int dd = 0; dd < groupmax; dd++)
    {
        unsigned char *image_region_one = new unsigned char [pagesz];
        V3DLONG group_type = groupIndex[dd];


        for(V3DLONG i = 0; i < pagesz; i++)
        {

            if(image_region[i] == group_type)
                image_region_one[i] = data1d[i];
            else
                image_region_one[i] = 0;
        }

        QString APP2_image_name = tmpfolder + "/group_one.raw";
        simple_saveimage_wrapper(callback, APP2_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
        if(image_region_one) {delete []image_region_one; image_region_one = 0;}

        QString APP2_swc =  APP2_image_name + QString("_group_%1.swc").arg(group_type);


    #if  defined(Q_OS_LINUX)
        QString cmd_APP2 = QString("%1/vaa3d -x Vaa3D_Neuron2 -f app2 -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str())
                .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
        system(qPrintable(cmd_APP2));
    #elif defined(Q_OS_MAC)
        QString cmd_APP2 = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x Vaa3D_Neuron2 -f app2 -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str())
                .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
        system(qPrintable(cmd_APP2));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

        vector<MyMarker*> temp_out_swc = readSWC_file(APP2_swc.toStdString());

        for(V3DLONG d = 0; d <temp_out_swc.size(); d++)
        {
            outswc_final.push_back(temp_out_swc[d]);

        }

    }

    QString final_swc = Para.inimg_file + "_region_APP2.swc";
    saveSWC_file(final_swc.toStdString(), outswc_final);

    system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

}

template <class T> void advantra_tracing(const T* image_region,
                                     unsigned char* data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     void * para,
                                     bool bmenu)
{

    ADVANTRA_LS_PARA Para = * (ADVANTRA_LS_PARA *) para;

    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    int groupNum = 0;
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > groupNum)
            groupNum = image_region[i];
    }

    int *groupArray = new int[groupNum];
    int *groupIndex = new int[groupNum];

    for(int i = 0; i < groupNum; i++)
    {
        groupArray[i] = 0;
        groupIndex[i] = i+1;
    }

    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > 0)
            groupArray[(unsigned short int)image_region[i] - 1] += 1;
    }

    int tmp,tmp_index;
    for(V3DLONG i = 0; i < groupNum; i++)
    {
        if (i > 0)
        {
            V3DLONG j = i;
            while(j > 0 && groupArray[j-1]<groupArray[j])
            {
                tmp = groupArray[j];
                groupArray[j] = groupArray[j-1];
                groupArray[j-1] = tmp;

                tmp_index = groupIndex[j];
                groupIndex[j] = groupIndex[j-1];
                groupIndex[j-1] = tmp_index;

                j--;
            }
        }
    }

    int groupmax = 50;
    if(groupNum <= groupmax) groupmax = groupNum;

    vector<MyMarker*> outswc_final;

    for(int dd = 0; dd < groupmax; dd++)
    {
        unsigned char *image_region_one = new unsigned char [pagesz];
        V3DLONG group_type = groupIndex[dd];


        for(V3DLONG i = 0; i < pagesz; i++)
        {

            if(image_region[i] == group_type)
                image_region_one[i] = data1d[i];
            else
                image_region_one[i] = 0;
        }

        QString ADVANTRA_image_name = tmpfolder + "/group_one.raw";
        simple_saveimage_wrapper(callback, ADVANTRA_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
        if(image_region_one) {delete []image_region_one; image_region_one = 0;}

        QString ADVANTRA_swc =  ADVANTRA_image_name + QString("_Advantra.swc").arg(group_type);


    #if  defined(Q_OS_LINUX)
        QString cmd_ADVANTRA = QString("%1/vaa3d -x Advantra -f advantra_func -i %2 -p %3 %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(ADVANTRA_image_name.toStdString().c_str())
                .arg(Para.scale).arg(Para.background_ratio).arg(Para.correlation_thresh).arg(Para.nr_directions).arg(Para.angular_sigma).arg(Para.nr_iters).arg(Para.nr_states).arg(Para.z_layer_dist_pix);
        system(qPrintable(cmd_ADVANTRA));
    #elif defined(Q_OS_MAC)
        QString cmd_ADVANTRA = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x Advantra -f advantra_func -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(ADVANTRA_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str())
                .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
        system(qPrintable(cmd_ADVANTRA));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

        vector<MyMarker*> temp_out_swc = readSWC_file(ADVANTRA_swc.toStdString());

        for(V3DLONG d = 0; d <temp_out_swc.size(); d++)
        {
            outswc_final.push_back(temp_out_swc[d]);

        }

    }

    QString final_swc = Para.inimg_file + "_region_ADVANTRA.swc";
    saveSWC_file(final_swc.toStdString(), outswc_final);

    system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

}

template <class T> void ent_tracing(const T* image_region,
                                     unsigned char* data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     void * para,
                                     bool bmenu)
{

    BASIC_LS_PARA Para = *(BASIC_LS_PARA *)para;
    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    int groupNum = 0;
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > groupNum)
            groupNum = image_region[i];
    }

    int *groupArray = new int[groupNum];
    int *groupIndex = new int[groupNum];

    for(int i = 0; i < groupNum; i++)
    {
        groupArray[i] = 0;
        groupIndex[i] = i+1;
    }

    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > 0)
            groupArray[(unsigned short int)image_region[i] - 1] += 1;
    }

    int tmp,tmp_index;
    for(V3DLONG i = 0; i < groupNum; i++)
    {
        if (i > 0)
        {
            V3DLONG j = i;
            while(j > 0 && groupArray[j-1]<groupArray[j])
            {
                tmp = groupArray[j];
                groupArray[j] = groupArray[j-1];
                groupArray[j-1] = tmp;

                tmp_index = groupIndex[j];
                groupIndex[j] = groupIndex[j-1];
                groupIndex[j-1] = tmp_index;

                j--;
            }
        }
    }

    int groupmax = 50;
    if(groupNum <= groupmax) groupmax = groupNum;

    vector<MyMarker*> outswc_final;

    for(int dd = 0; dd < groupmax; dd++)
    {
        unsigned char *image_region_one = new unsigned char [pagesz];
        V3DLONG group_type = groupIndex[dd];


        for(V3DLONG i = 0; i < pagesz; i++)
        {

            if(image_region[i] == group_type)
                image_region_one[i] = data1d[i];
            else
                image_region_one[i] = 0;
        }

        QString ENT_image_name = tmpfolder + "/group_one.raw";
        simple_saveimage_wrapper(callback, ENT_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
        if(image_region_one) {delete []image_region_one; image_region_one = 0;}

        QString ENT_swc =  ENT_image_name + "_ENT.swc";

    #if  defined(Q_OS_LINUX)
        QString cmd_ENT = QString("%1/vaa3d -x ENT -f tracing_func -i %2").arg(getAppPath().toStdString().c_str()).arg(ENT_image_name.toStdString().c_str());
        system(qPrintable(cmd_ENT));
    #elif defined(Q_OS_MAC)
        QString cmd_ENT = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x ENT -f tracing_func -i %2").arg(getAppPath().toStdString().c_str()).arg(ENT_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str());
        system(qPrintable(cmd_ENT));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

        vector<MyMarker*> temp_out_swc = readSWC_file(ENT_swc.toStdString());

        for(V3DLONG d = 0; d <temp_out_swc.size(); d++)
        {
            outswc_final.push_back(temp_out_swc[d]);

        }

    }

    QString final_swc = Para.inimg_file + "_region_ENT.swc";
    saveSWC_file(final_swc.toStdString(), outswc_final);

    system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

}

template <class T> void gd_tracing(const T* image_region,
                                     unsigned char* data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     void * para,
                                     bool bmenu)
{

    BASIC_LS_PARA Para = *(BASIC_LS_PARA *)para;
    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    int groupNum = 0;
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > groupNum)
            groupNum = image_region[i];
    }

    int *groupArray = new int[groupNum];
    int *groupIndex = new int[groupNum];

    for(int i = 0; i < groupNum; i++)
    {
        groupArray[i] = 0;
        groupIndex[i] = i+1;
    }

    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > 0)
            groupArray[(unsigned short int)image_region[i] - 1] += 1;
    }

    int tmp,tmp_index;
    for(V3DLONG i = 0; i < groupNum; i++)
    {
        if (i > 0)
        {
            V3DLONG j = i;
            while(j > 0 && groupArray[j-1]<groupArray[j])
            {
                tmp = groupArray[j];
                groupArray[j] = groupArray[j-1];
                groupArray[j-1] = tmp;

                tmp_index = groupIndex[j];
                groupIndex[j] = groupIndex[j-1];
                groupIndex[j-1] = tmp_index;

                j--;
            }
        }
    }

    int groupmax = 50;
    if(groupNum <= groupmax) groupmax = groupNum;

    vector<MyMarker*> outswc_final;

    for(int dd = 0; dd < groupmax; dd++)
    {
        unsigned char *image_region_one = new unsigned char [pagesz];
        V3DLONG group_type = groupIndex[dd];


        for(V3DLONG i = 0; i < pagesz; i++)
        {

            if(image_region[i] == group_type)
                image_region_one[i] = data1d[i];
            else
                image_region_one[i] = 0;
        }

        QString GD_image_name = tmpfolder + "/group_one.raw";
        simple_saveimage_wrapper(callback, GD_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
        if(image_region_one) {delete []image_region_one; image_region_one = 0;}

        QString GD_swc =  GD_image_name + "_nctuTW_GD.swc";

    #if  defined(Q_OS_LINUX)
        QString cmd_GD = QString("%1/vaa3d -x libtips -f tracing_func -i %2").arg(getAppPath().toStdString().c_str()).arg(GD_image_name.toStdString().c_str());
        system(qPrintable(cmd_GD));
    #elif defined(Q_OS_MAC)
        QString cmd_GD = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x libtips -f tracing_func -i %2").arg(getAppPath().toStdString().c_str()).arg(GD_image_name.toStdString().c_str())
        system(qPrintable(cmd_GD));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

        vector<MyMarker*> temp_out_swc = readSWC_file(GD_swc.toStdString());

        for(V3DLONG d = 0; d <temp_out_swc.size(); d++)
        {
            outswc_final.push_back(temp_out_swc[d]);

        }

    }

    QString final_swc = Para.inimg_file + "_region_nctuTW_GD.swc";
    saveSWC_file(final_swc.toStdString(), outswc_final);

    system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

}

template <class T> void mst_tracing(const T* image_region,
                                     unsigned char* data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     void *para,
                                     bool bmenu)
{

    MST_LS_PARA Para = *(MST_LS_PARA *)para;
    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    int groupNum = 0;
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > groupNum)
            groupNum = image_region[i];
    }

    int *groupArray = new int[groupNum];
    int *groupIndex = new int[groupNum];

    for(int i = 0; i < groupNum; i++)
    {
        groupArray[i] = 0;
        groupIndex[i] = i+1;
    }

    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > 0)
            groupArray[(unsigned short int)image_region[i] - 1] += 1;
    }

    int tmp,tmp_index;
    for(V3DLONG i = 0; i < groupNum; i++)
    {
        if (i > 0)
        {
            V3DLONG j = i;
            while(j > 0 && groupArray[j-1]<groupArray[j])
            {
                tmp = groupArray[j];
                groupArray[j] = groupArray[j-1];
                groupArray[j-1] = tmp;

                tmp_index = groupIndex[j];
                groupIndex[j] = groupIndex[j-1];
                groupIndex[j-1] = tmp_index;

                j--;
            }
        }
    }

    int groupmax = 50;
    if(groupNum <= groupmax) groupmax = groupNum;

    vector<MyMarker*> outswc_final;

    for(int dd = 0; dd < groupmax; dd++)
    {
        unsigned char *image_region_one = new unsigned char [pagesz];
        V3DLONG group_type = groupIndex[dd];


        for(V3DLONG i = 0; i < pagesz; i++)
        {

            if(image_region[i] == group_type)
                image_region_one[i] = data1d[i];
            else
                image_region_one[i] = 0;
        }

        QString MST_image_name = tmpfolder + "/group_one.raw";
        simple_saveimage_wrapper(callback, MST_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
        if(image_region_one) {delete []image_region_one; image_region_one = 0;}

        QString MST_swc =  MST_image_name + "_MST_Tracing.swc";

    #if  defined(Q_OS_LINUX)
        QString cmd_MST = QString("%1/vaa3d -x MST -f trace_mst -i %2 -p 1 %3").arg(getAppPath().toStdString().c_str()).arg(MST_image_name.toStdString().c_str()).arg(Para.window_size);
        system(qPrintable(cmd_MST));
    #elif defined(Q_OS_MAC)
        QString cmd_MST = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x MST -f trace_mst -i %2 -p 1 %3").arg(getAppPath().toStdString().c_str()).arg(GD_image_name.toStdString().c_str()).arg(Para.window_size);
        system(qPrintable(cmd_MST));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

        vector<MyMarker*> temp_out_swc = readSWC_file(MST_swc.toStdString());

        for(V3DLONG d = 0; d <temp_out_swc.size(); d++)
        {
            outswc_final.push_back(temp_out_swc[d]);

        }

    }

    QString final_swc = Para.inimg_file + "_region_MST_Tracing.swc";
    saveSWC_file(final_swc.toStdString(), outswc_final);

    system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

}

template <class T> void nc_tracing(const T* image_region,
                                     unsigned char* data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     void *para,
                                     bool bmenu)
{

    NC_LS_PARA Para = *(NC_LS_PARA *)para;
    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    int groupNum = 0;
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > groupNum)
            groupNum = image_region[i];
    }

    int *groupArray = new int[groupNum];
    int *groupIndex = new int[groupNum];

    for(int i = 0; i < groupNum; i++)
    {
        groupArray[i] = 0;
        groupIndex[i] = i+1;
    }

    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > 0)
            groupArray[(unsigned short int)image_region[i] - 1] += 1;
    }

    int tmp,tmp_index;
    for(V3DLONG i = 0; i < groupNum; i++)
    {
        if (i > 0)
        {
            V3DLONG j = i;
            while(j > 0 && groupArray[j-1]<groupArray[j])
            {
                tmp = groupArray[j];
                groupArray[j] = groupArray[j-1];
                groupArray[j-1] = tmp;

                tmp_index = groupIndex[j];
                groupIndex[j] = groupIndex[j-1];
                groupIndex[j-1] = tmp_index;

                j--;
            }
        }
    }

    int groupmax = 50;
    if(groupNum <= groupmax) groupmax = groupNum;

    vector<MyMarker*> outswc_final;

    for(int dd = 0; dd < groupmax; dd++)
    {
        unsigned char *image_region_one = new unsigned char [pagesz];
        V3DLONG group_type = groupIndex[dd];


        for(V3DLONG i = 0; i < pagesz; i++)
        {

            if(image_region[i] == group_type)
                image_region_one[i] = data1d[i];
            else
                image_region_one[i] = 0;
        }

        QString NC_image_name = tmpfolder + "/group_one.raw";
        simple_saveimage_wrapper(callback, NC_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
        if(image_region_one) {delete []image_region_one; image_region_one = 0;}

        QString NC_swc =  NC_image_name + "_NeuronChaser.swc";


    #if  defined(Q_OS_LINUX)
        QString cmd_NC = QString("%1/vaa3d -x NeuronChaser -f nc_func -i %2 -p %3 %4 %5 %6 %7 %8 %9 %10 %11").arg(getAppPath().toStdString().c_str()).arg(NC_image_name.toStdString().c_str())
                .arg(Para.channel).arg(Para.scale).arg(Para.corr_thresh).arg(Para.nr_dirs).arg(Para.angular_sigma).arg(Para.nr_iters).arg(Para.nr_states).arg(Para.z_dist).arg(Para.save_midres);
        system(qPrintable(cmd_NC));
    #elif defined(Q_OS_MAC)
        QString cmd_NC = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x NeuronChaser -f nc_func -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(NC_image_name.toStdString().c_str())
                .arg(Para.channel).arg(Para.scale).arg(Para.corr_thresh).arg(Para.nr_dirs).arg(Para.angular_sigma).arg(Para.nr_iters).arg(Para.nr_states).arg(Para.z_dist).arg(Para.save_midres);
        system(qPrintable(cmd_NC));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

        vector<MyMarker*> temp_out_swc = readSWC_file(NC_swc.toStdString());

        for(V3DLONG d = 0; d <temp_out_swc.size(); d++)
        {
            outswc_final.push_back(temp_out_swc[d]);

        }

    }

    QString final_swc = Para.inimg_file + "_region_NeuronChaser.swc";
    saveSWC_file(final_swc.toStdString(), outswc_final);

    system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

}

void autotrace_region(V3DPluginCallback2 &callback, QWidget *parent,void * Para,bool bmenu, QString image_name, const QString trace)
{
    int trace_num;
    int th;
    int region_num;

    if (trace == QString("trace_advantra")) {
        ADVANTRA_LS_PARA * para = (ADVANTRA_LS_PARA *) Para;
        th = para->bkgd_thresh;
        region_num = para->region_num;
        trace_num = 1;
    }
    else if (trace == QString("trace_app2")) {
        APP2_LS_PARA * para = (APP2_LS_PARA *) Para;
        th = para->bkg_thresh;
        region_num = para->region_number;
        trace_num = 3;
    }
    else if (trace == QString("trace_ent")) {
        BASIC_LS_PARA * para = (BASIC_LS_PARA *) Para;
        th = para->bkg_thresh;
        region_num = para->region_number;
        trace_num = 4;
    }
    else if (trace == QString("trace_gd")) {
        BASIC_LS_PARA * para = (BASIC_LS_PARA *) Para;
        th = para->bkg_thresh;
        region_num = para->region_number;
        trace_num = 5;
    }
    else if (trace == QString("trace_mst")) {
        MST_LS_PARA * para = (MST_LS_PARA *) Para;
        th = para->bkg_thresh;
        region_num = para->region_number;
        trace_num = 6;
    }
    else if (trace == QString("trace_neuronchaser")) {
        NC_LS_PARA * para = (NC_LS_PARA *) Para;
        th = para->bkgd_thresh;
        region_num = para->region_num;
        trace_num = 7;
    }
    else {
        v3d_msg("Invalid menu option");
    }
    unsigned char* data1d = 0;
    V3DLONG N,M,P,C;

    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        Image4DSimple* p4DImage = callback.getImage(curwin);
        data1d = p4DImage->getRawData();

        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        C = p4DImage->getCDim();
    }
    else
    {
        V3DLONG *im_sz = 0;
        int datatype = 0;
        if (loadImage(const_cast<char *>(image_name.toStdString().c_str()), data1d, im_sz, datatype)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",image_name.toStdString().c_str());
            return;
        }
        N = im_sz[0];
        M = im_sz[1];
        P = im_sz[2];
        C = im_sz[3];

    }

    QString tmp = "_tmp_";
    QString tmpfolder = QFileInfo(image_name).path()+("/") + QFileInfo(image_name).completeBaseName()+ tmp.append(trace);
    system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));
    if(tmpfolder.isEmpty())
    {

        printf("Can not create a tmp folder!\n");
        return;
    }

    V3DLONG pagesz = N*M*P;
    unsigned char *image_binary=0;
    try {image_binary = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for image_binary."); return;}
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if(data1d[i] > th)
            image_binary[i] = 255;
        else
            image_binary[i] = 0;
    }

    V3DLONG in_sz[4];
    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;
    in_sz[3] = 1;

    QString input_image_name = tmpfolder + "/binary.raw";
    simple_saveimage_wrapper(callback, input_image_name.toStdString().c_str(),  (unsigned char *)image_binary, in_sz, V3D_UINT8);

    if(image_binary) {delete []image_binary; image_binary = 0;}

    QString output_image_name = tmpfolder +"/region.raw";

    #if  defined(Q_OS_LINUX)
        QString cmd_region = QString("%1/vaa3d -x regiongrow -f rg -i %2 -o %3 -p 1 0 1 %4").arg(getAppPath().toStdString().c_str()).arg(input_image_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str()).arg(region_num);
        system(qPrintable(cmd_region));
    #elif defined(Q_OS_MAC)
        QString cmd_region = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x regiongrow -f rg -i %2 -o %3 -p 1 0 1 %4").arg(getAppPath().toStdString().c_str()).arg(input_image_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str()).arg(region_num);
        system(qPrintable(cmd_region));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

   unsigned char * image_region = 0;
   int datatype;
   V3DLONG in_zz[4];
   if(!simple_loadimage_wrapper(callback, output_image_name.toStdString().c_str(), image_region, in_zz, datatype))
   {
       v3d_msg("Fail to load image");
       return;
   }

   switch (trace_num)
   {
   case 1:
       switch (datatype)
       {
       case 1: advantra_tracing(image_region, data1d,in_zz, callback,tmpfolder,Para,bmenu); break;
       case 2: advantra_tracing((unsigned short int *)image_region,data1d,in_zz,callback,tmpfolder,Para,bmenu); break;
       default: v3d_msg("Invalid data type. Do nothing."); return;
       }
       break;
   case 3:
       switch (datatype)
       {
       case 1: app2_tracing(image_region, data1d,in_zz, callback,tmpfolder,Para,bmenu); break;
       case 2: app2_tracing((unsigned short int *)image_region,data1d,in_zz,callback,tmpfolder,Para,bmenu); break;
       default: v3d_msg("Invalid data type. Do nothing."); return;
       }
       break;
   case 4:
       switch (datatype)
       {
       case 1: ent_tracing(image_region, data1d,in_zz, callback,tmpfolder,Para,bmenu); break;
       case 2: ent_tracing((unsigned short int *)image_region,data1d,in_zz,callback,tmpfolder,Para,bmenu); break;
       default: v3d_msg("Invalid data type. Do nothing."); return;
       }
       break;
   case 5:
       switch (datatype)
       {
       case 1: gd_tracing(image_region, data1d,in_zz, callback,tmpfolder,Para,bmenu); break;
       case 2: gd_tracing((unsigned short int *)image_region,data1d,in_zz,callback,tmpfolder,Para,bmenu); break;
       default: v3d_msg("Invalid data type. Do nothing."); return;
       }
       break;
   case 6:
       switch (datatype)
       {
       case 1: mst_tracing(image_region, data1d,in_zz, callback,tmpfolder,Para,bmenu); break;
       case 2: mst_tracing((unsigned short int *)image_region,data1d,in_zz,callback,tmpfolder,Para,bmenu); break;
       default: v3d_msg("Invalid data type. Do nothing."); return;
       }
       break;
   case 7:
       switch (datatype)
       {
       case 1: nc_tracing(image_region, data1d,in_zz, callback,tmpfolder,Para,bmenu); break;
       case 2: nc_tracing((unsigned short int *)image_region,data1d,in_zz,callback,tmpfolder,Para,bmenu); break;
       default: v3d_msg("Invalid data type. Do nothing."); return;
       }
       break;
   default:
       v3d_msg("invalid trace type. Do nothing.");
       return;
   }


   if(image_region) {delete []image_region; image_region = 0;}
   if(~bmenu)
       if(data1d) {delete []data1d; data1d = 0;}

   return;
}
