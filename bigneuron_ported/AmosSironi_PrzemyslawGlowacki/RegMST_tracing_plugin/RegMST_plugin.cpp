/* RegMST_plugin.cpp
 * a plugin for applying the regression tubularity approach [1]
 * in combiantion with a MST algorithm [2] to reconstruct tubular structures.
 * 2015-6-23: by Amos Sironi and Przemysław Głowacki
 *
 * [1] A. Sironi, E. Türetken, V. Lepetit and P. Fua. Multiscale Centerline Detection, submitted to IEEE Transactions on Pattern Analysis and Machine Intelligence.
 * [2] H. Peng, Z. Ruan, D. Atasoy and S. Sternson Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model. Bioinformatics.
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "RegMST_plugin.h"


#include <math.h>

#include "../libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h"

#include "../libs/regression/sep_conv.h"
#include "../libs/regression/sampling.h"
#include "../libs/regression/regressor.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "../SQBTree_plugin/RegressionTubularityAC_plugin.h"
#include "mst_functions/neurontracing_mst_plugin.h"

Q_EXPORT_PLUGIN2(RegMST, RegMST);

using namespace std;

//parameters used by tubularity plugin
struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    V3DLONG n_ac_iters;
    QList<QString> regressor_paths;
    V3DLONG Ws;
};


void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 

QStringList RegMST::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList RegMST::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void RegMST::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        v3d_msg("To be implemented. Please call this plugin from command line.\n "
                "\n"
                "***Usage of RegMST tracing***\n"
                "vaa3d -x RegMST -f tracing_func -i <inimg_file> -p <channel> <n_AC> <reg_path_1.cfg> ... <reg_path_n_AC.cfg> <window size>\n"
                "inimg_file                                The input image\n"
                "channel                                   Data channel for tracing. Start from 1 (default 1)\n"
                "n_AC                                      Number of autoncontext iterations to compute tubularity. 1 = one regressor i.e. no auto-context; 0 = apply MST on original image (default 0)\n"
                "reg_path_1.cfg ... reg_path_n_AC.cfg      Path to regressors (generated using RegressionTubularityAC plugin). Number of files must be equal to n_AC\n"
                "window size                               Window size for seed detection in MST. (default 10)\n"
                "outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n");

        // input_PARA PARA;
       // reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
        v3d_msg(tr("plugin for applying the regression tubularity approach [1]\n "
                   "in combiantion with the MST algorithm to reconstruct tubular structures.\n\n"
                   " [1]  A. Sironi, E. Turetken, V. Lepetit and P. Fua. Multiscale Centerline Detection, submitted to IEEE Transactions on Pattern Analysis and Machine Intelligence.\n"
			"Developed by Amos Sironi and Przemyslaw Glowacki, 2015-6-23"));
	}
}

bool RegMST::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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
        PARA.n_ac_iters = (paras.size() >= k+1) ? atoi(paras[k]) : -1;  k++;
        for(unsigned int i_ac =0; i_ac < PARA.n_ac_iters; i_ac++,k++){
            PARA.regressor_paths.append(paras[k]);
        }
        PARA.Ws = (paras.size() >= k+1) ? atoi(paras[k]) : 10;  k++;

        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of RegMST tracing **** \n");
        printf("vaa3d -x RegMST -f tracing_func -i <inimg_file> -p <channel> <n_AC> <reg_path_1.cfg> ... <reg_path_n_AC.cfg> <window size>\n");
        printf("inimg_file                                The input image\n");
        printf("channel                                   Data channel for tracing. Start from 1 (default 1).\n");
        printf("n_AC                                      Number of autoncontext iterations to compute tubularity. 1 = one regressor i.e. no auto-context; 0 = apply MST on original image (default 0)\n");
        printf("reg_path_1.cfg ... reg_path_n_AC.cfg      Path to regressors (generated using RegressionTubularityAC plugin). Number of files must be equal to n_AC.\n");
        printf("window size                               Window size for seed detection in MST. (default 10)\n");

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

    //main neuron reconstruction code
    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE


 V3DPluginArgList  input;
 V3DPluginArgItem name_input_v3d;
 char *buf_input = new char[PARA.inimg_file.toStdString().size()+1];
 strcpy(buf_input, PARA.inimg_file.toStdString().c_str());
 vector<char*> *buf_input_vec= new vector<char*>[1];
buf_input_vec->push_back(buf_input);
 name_input_v3d.p =buf_input_vec;
 input.append(name_input_v3d);


V3DPluginArgList  output;
V3DPluginArgItem name_output_v3d;

QString out_tubularity_filename = PARA.inimg_file + "_tubularity.v3draw";
char *buf_out = new char[out_tubularity_filename.toStdString().size()+1];
strcpy(buf_out, out_tubularity_filename.toStdString().c_str());
vector<char*> *buf_out_vec= new vector<char*>[1];
buf_out_vec->push_back(buf_out);
name_output_v3d.p =buf_out_vec;
output.append(name_output_v3d);


vector<char*> *buf_para_vec= new vector<char*>[PARA.n_ac_iters];

for(unsigned int i_ac = 0; i_ac < PARA.n_ac_iters; i_ac++){

    char *buf_para = new char[PARA.regressor_paths.at(i_ac).toStdString().size()+1];
    strcpy(buf_para, PARA.regressor_paths.at(i_ac).toStdString().c_str());
    buf_para_vec->push_back(buf_para);

}

if( (PARA.n_ac_iters)>0){
    V3DPluginArgItem name_para_v3d;
     name_para_v3d.p =buf_para_vec;
    input.append(name_para_v3d);
}



MST_PARA mst_PARA;
mst_PARA.Ws = PARA.Ws;
mst_PARA.channel = PARA.channel;

if((PARA.n_ac_iters>0)){//compute regression tubularity

    std::cout << "calling REG tubularity plugin  " <<std::endl << std::flush;

    QString full_plugin_name_tubularity = "RegressionTubularityAC";
    QString func_name_test_tubularity = "test";
    callback.callPluginFunc(full_plugin_name_tubularity,func_name_test_tubularity, input,output);

    mst_PARA.inimg_file = out_tubularity_filename;

}else{//use original image for tracing
    mst_PARA.inimg_file = PARA.inimg_file;

    std::cout << "no Regressor passed as input. n_ac_iters =   " <<PARA.n_ac_iters<<std::endl<< std::flush;
    std::cout << "Applying MST to original image." <<std::endl<< std::flush;;

}

//now call tracing function

std::cout << "calling MST tracing plugin  " <<std::endl<< std::flush;;
    autotrace_mst(callback,parent,mst_PARA,bmenu);

    return;
}
